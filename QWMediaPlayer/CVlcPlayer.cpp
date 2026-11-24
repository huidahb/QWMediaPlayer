#include "CVlcPlayer.h"

// windows.h 内含 basetsd.h，提供 SSIZE_T；同时提供窗口枚举所需 API
#ifdef _WIN32
#include <windows.h>
// Windows 没有 POSIX 的 ssize_t（带符号 size_t），libvlc 回调需要它
typedef SSIZE_T ssize_t;
#endif

#include "vlc/vlc.h"

#include <QtCore/QDir>
#include <QtCore/QTimer>


// ============================================================================
// 静态事件回调：libvlc 是 C 库，只认识函数指针，不认识 Qt 信号槽。
// 注册事件时把本对象指针作为 userData 传入，事件发生时再取回，
// 由对应的 handle 方法把 C 事件转换为 Qt 信号
// ============================================================================
void CVlcPlayer::onVlcEvent(const libvlc_event_t* event, void* userData)
{
    CVlcPlayer* self = static_cast<CVlcPlayer*>(userData);
    if (!self || !event)
        return;

    switch (event->type)
    {
    // 进度/时间/时长变化统一交给进度处理
    case libvlc_MediaPlayerPositionChanged:
    case libvlc_MediaPlayerTimeChanged:
    case libvlc_MediaPlayerLengthChanged:
        self->handlePositionChanged();
        break;
    case libvlc_MediaPlayerAudioVolume:
        self->handleVolumeChanged();
        break;
    // 播放/暂停/停止/播完统一交给状态处理
    case libvlc_MediaPlayerPlaying:
    case libvlc_MediaPlayerPaused:
    case libvlc_MediaPlayerStopped:
    case libvlc_MediaPlayerEndReached:
        self->handleStateChanged();
        break;
    case libvlc_MediaPlayerMediaChanged:
        self->handleMediaChanged();
        break;
    default:
        break;
    }
}

CVlcPlayer::CVlcPlayer(QObject* parent)
    : QObject(parent)
{
    // 创建 VLC 实例（new = 新建；0, nullptr 表示无额外命令行参数）
    m_pInstance = libvlc_new(0, nullptr);
    if (!m_pInstance)
        return;

    // 依次创建核心播放器、媒体列表、列表播放器
    m_pPlayer     = libvlc_media_player_new(m_pInstance);
    m_pMediaList  = libvlc_media_list_new(m_pInstance);
    m_pListPlayer = libvlc_media_list_player_new(m_pInstance);

    // 取得核心播放器的事件管理器并挂接需要监听的事件
    libvlc_event_manager_t* manager =
        libvlc_media_player_event_manager(m_pPlayer);
    if (manager)
    {
        libvlc_event_attach(manager, libvlc_MediaPlayerPositionChanged,
                            &onVlcEvent, this);
        libvlc_event_attach(manager, libvlc_MediaPlayerTimeChanged,
                            &onVlcEvent, this);
        libvlc_event_attach(manager, libvlc_MediaPlayerLengthChanged,
                            &onVlcEvent, this);
        libvlc_event_attach(manager, libvlc_MediaPlayerAudioVolume,
                            &onVlcEvent, this);
        libvlc_event_attach(manager, libvlc_MediaPlayerPlaying,
                            &onVlcEvent, this);
        libvlc_event_attach(manager, libvlc_MediaPlayerPaused,
                            &onVlcEvent, this);
        libvlc_event_attach(manager, libvlc_MediaPlayerStopped,
                            &onVlcEvent, this);
        libvlc_event_attach(manager, libvlc_MediaPlayerEndReached,
                            &onVlcEvent, this);
        libvlc_event_attach(manager, libvlc_MediaPlayerMediaChanged,
                            &onVlcEvent, this);
    }

    // 在主线程（本构造函数所在线程）创建并配置轮询定时器。
    // 不能放到 libvlc 事件回调里创建，否则定时器会被归入 libvlc 后台线程，
    // 触发 "Timers can only be used with threads started with QThread"，等于没启动
    m_pSubWindowTimer = new QTimer(this);
    connect(m_pSubWindowTimer, &QTimer::timeout,
            this, &CVlcPlayer::disableVideoSubWindow);

    // libvlc 事件在后台线程发出本信号，接收槽在主线程执行（AutoConnection
    // 跨线程自动变为 QueuedConnection），从而安全地启停定时器
    connect(this, &CVlcPlayer::requestFixSubWindow,
            this, &CVlcPlayer::onRequestFixSubWindow);
}

CVlcPlayer::~CVlcPlayer()
{
    // 先解绑全部事件，再按创建逆序释放对象
    if (m_pPlayer)
    {
        libvlc_event_manager_t* manager =
            libvlc_media_player_event_manager(m_pPlayer);
        if (manager)
        {
            libvlc_event_detach(manager, libvlc_MediaPlayerPositionChanged,
                                &onVlcEvent, this);
            libvlc_event_detach(manager, libvlc_MediaPlayerTimeChanged,
                                &onVlcEvent, this);
            libvlc_event_detach(manager, libvlc_MediaPlayerLengthChanged,
                                &onVlcEvent, this);
            libvlc_event_detach(manager, libvlc_MediaPlayerAudioVolume,
                                &onVlcEvent, this);
            libvlc_event_detach(manager, libvlc_MediaPlayerPlaying,
                                &onVlcEvent, this);
            libvlc_event_detach(manager, libvlc_MediaPlayerPaused,
                                &onVlcEvent, this);
            libvlc_event_detach(manager, libvlc_MediaPlayerStopped,
                                &onVlcEvent, this);
            libvlc_event_detach(manager, libvlc_MediaPlayerEndReached,
                                &onVlcEvent, this);
            libvlc_event_detach(manager, libvlc_MediaPlayerMediaChanged,
                                &onVlcEvent, this);
        }
    }

    if (m_pListPlayer) libvlc_media_list_player_release(m_pListPlayer);
    if (m_pMediaList)  libvlc_media_list_release(m_pMediaList);
    if (m_pPlayer)     libvlc_media_player_release(m_pPlayer);
    if (m_pInstance)   libvlc_release(m_pInstance);
}

void CVlcPlayer::setVideoOutput(WId windowId)
{
    // 保存 Qt 窗口句柄，打开文件时再交给 libvlc
    m_pVideoHwnd = reinterpret_cast<void*>(windowId);
}

void CVlcPlayer::openFiles(const QStringList& files)
{
    if (!m_pInstance || files.isEmpty())
        return;

    // 清空旧媒体列表（lock/unlock 防止遍历期间列表被改动）
    libvlc_media_list_lock(m_pMediaList);
    while (libvlc_media_list_count(m_pMediaList) > 0)
        libvlc_media_list_remove_index(m_pMediaList, 0);
    libvlc_media_list_unlock(m_pMediaList);

    appendMediaToMediaList(files);
    bindListAndPlay();
}

void CVlcPlayer::addFiles(const QStringList& files)
{
    if (!m_pInstance || files.isEmpty())
        return;

    // 记录追加前是否为空：空列表说明尚未关联播放，需补做关联并自动播放
    const int oldCount = libvlc_media_list_count(m_pMediaList);

    appendMediaToMediaList(files);

    // 非空列表已在 openFiles 时关联，libvlc 自动识别新增项，无需重新关联或播放
    if (oldCount == 0)
        bindListAndPlay();
}

void CVlcPlayer::appendMediaToMediaList(const QStringList& files)
{
    libvlc_media_list_lock(m_pMediaList);
    for (const QString& path : files)
    {
        // toNativeSeparators：统一为 Windows 反斜杠路径；toUtf8 转 C 字符串
        const QByteArray native =
            QDir::toNativeSeparators(path).toUtf8();
        libvlc_media_t* media =
            libvlc_media_new_path(m_pInstance, native.constData());
        if (!media)
            continue;
        libvlc_media_list_add_media(m_pMediaList, media);
        libvlc_media_release(media); // 列表内部会 retain，这里可安全释放
    }
    libvlc_media_list_unlock(m_pMediaList);
}

void CVlcPlayer::bindListAndPlay()
{
    // 列表播放器关联媒体列表与核心播放器
    libvlc_media_list_player_set_media_list(m_pListPlayer, m_pMediaList);
    libvlc_media_list_player_set_media_player(m_pListPlayer, m_pPlayer);

    // 指定视频输出窗口，画面将渲染到该句柄
    if (m_pVideoHwnd)
        libvlc_media_player_set_hwnd(m_pPlayer, m_pVideoHwnd);

    // default：顺序播放，列表内仍自动连播，但最后一个播完即停止（主流默认行为）
    libvlc_media_list_player_set_playback_mode(
        m_pListPlayer, libvlc_playback_mode_default);

    libvlc_media_list_player_play(m_pListPlayer);
}

void CVlcPlayer::togglePlay()
{
    // 根据核心播放器当前状态决定暂停还是继续
    const libvlc_state_t state = libvlc_media_player_get_state(m_pPlayer);
    if (state == libvlc_Playing)
        libvlc_media_list_player_pause(m_pListPlayer);
    else
        libvlc_media_list_player_play(m_pListPlayer);
}

void CVlcPlayer::stop()
{
    libvlc_media_list_player_stop(m_pListPlayer);
}

void CVlcPlayer::next()
{
    libvlc_media_list_player_next(m_pListPlayer);
}

void CVlcPlayer::previous()
{
    libvlc_media_list_player_previous(m_pListPlayer);
}

void CVlcPlayer::playAtIndex(int index)
{
    if (!m_pListPlayer || !m_pMediaList)
        return;
    const int count = libvlc_media_list_count(m_pMediaList);
    if (index < 0 || index >= count)
        return;
    libvlc_media_list_player_play_item_at_index(m_pListPlayer, index);
}


void CVlcPlayer::removeAtIndex(int index)
{
    if (!m_pMediaList)
        return;
    const int count = libvlc_media_list_count(m_pMediaList);
    if (index < 0 || index >= count)
        return;
    // 仅当播放器处于播放/暂停、且删除的正是当前项时，才需要主动接管播放
    // （libvlc 引用计数会让被移除的当前视频继续播，故必须显式处理）
    const libvlc_state_t state = libvlc_media_player_get_state(m_pPlayer);
    const bool playerActive =
        (state == libvlc_Playing || state == libvlc_Paused);
    const bool removingCurrent = playerActive && (index == m_iCurrentIndex);

    libvlc_media_list_lock(m_pMediaList);
    libvlc_media_list_remove_index(m_pMediaList, index);
    // count 按 API 契约需持锁调用；删除后立即上报最新数量，驱动进度条显隐
    const int newCount = libvlc_media_list_count(m_pMediaList);
    libvlc_media_list_unlock(m_pMediaList);

    if (removingCurrent)
    {
        if (newCount == 0)
        {
            libvlc_media_list_player_stop(m_pListPlayer); // 列表清空：停止播放
            m_iCurrentIndex = -1;
        }
        else
        {
            // 下一项顶到原索引；若删的是末尾项（原索引已不存在）则回退一个
            const int target = (index < newCount) ? index : (newCount - 1);
            libvlc_media_list_player_play_item_at_index(m_pListPlayer, target);
        }
    }

    emit mediaCountChanged(newCount);
}
void CVlcPlayer::setPosition(float position)
{
    // 钳制到 0.0~1.0，再交给核心播放器跳转
    if (position < 0.0f) position = 0.0f;
    if (position > 1.0f) position = 1.0f;
    libvlc_media_player_set_position(m_pPlayer, position);
}

void CVlcPlayer::setVolume(int volume)
{
    // 钳制到 0~100，再设置音频音量
    if (volume < 0)   volume = 0;
    if (volume > 100) volume = 100;
    libvlc_audio_set_volume(m_pPlayer, volume);
}

void CVlcPlayer::handlePositionChanged()
{
    // get_position：获取当前位置（0.0~1.0；无有效媒体时可能为 -1）
    float position = libvlc_media_player_get_position(m_pPlayer);
    if (position < 0.0f) position = 0.0f;
    if (position > 1.0f) position = 1.0f;
    emit positionChanged(position);

    // get_time：当前时间；get_length：总时长（单位均为毫秒）
    const qint64 current = libvlc_media_player_get_time(m_pPlayer);
    const qint64 length  = libvlc_media_player_get_length(m_pPlayer);
    if (length > 0)
        m_totalMs = length;
    emit timeChanged(current < 0 ? 0 : current, m_totalMs);
}

void CVlcPlayer::handleVolumeChanged()
{
    int volume = libvlc_audio_get_volume(m_pPlayer);
    if (volume < 0) volume = 0;
    emit volumeChanged(volume);
}

void CVlcPlayer::handleStateChanged()
{
    // get_state：获取播放状态；仅 libvlc_Playing 视为播放中。
    // 注意：本函数运行在 libvlc 后台线程，不能直接碰 QTimer，
    // 只发信号，把定时器启停委托给主线程槽 onRequestFixSubWindow
    const libvlc_state_t state = libvlc_media_player_get_state(m_pPlayer);
    const bool playing = (state == libvlc_Playing);
    emit playingChanged(playing);
    emit requestFixSubWindow(playing);

}

// 主线程槽：根据播放状态启停轮询定时器
void CVlcPlayer::handleMediaChanged()
{
    libvlc_media_t* current = libvlc_media_player_get_media(m_pPlayer);
    if (!current || !m_pMediaList)
        return;

    // count/item_at_index 按 API 契约要求持有列表锁，
    // 防止遍历期间主线程增删列表造成并发竞态
    libvlc_media_list_lock(m_pMediaList);
    const int count = libvlc_media_list_count(m_pMediaList);
    for (int i = 0; i < count; ++i)
    {
        libvlc_media_t* item =
            libvlc_media_list_item_at_index(m_pMediaList, i);
        if (item == current)
        {
            m_iCurrentIndex = i;
            emit currentIndexChanged(i);
        }
        // item_at_index 已对返回 media 调用 retain，必须配对 release
        libvlc_media_release(item);
    }
    libvlc_media_list_unlock(m_pMediaList);
}

void CVlcPlayer::onRequestFixSubWindow(bool playing)
{
    if (playing)
    {
        // 播放期间持续运行（不提前停止）：VLC 切换视频若重建渲染窗口，
        // 新窗口最多 200ms 就会被重新禁用并沉底，保证每个视频双击都有效
        disableVideoSubWindow();           // 立即先处理一次
        m_pSubWindowTimer->start(200);
    }
    else
    {
        // 暂停/停止时不再轮询；此时旧窗口保持禁用，双击仍能命中悬浮层
        m_pSubWindowTimer->stop();
    }
}

void CVlcPlayer::disableVideoSubWindow()
{
    if (!m_pVideoHwnd)
        return;

    HWND parent = static_cast<HWND>(m_pVideoHwnd);

    // 枚举全部后代窗口；父窗口句柄通过 LPARAM 传入，lambda 保持无捕获以兼容 C 回调
    EnumChildWindows(parent,
        [](HWND child, LPARAM lp) -> BOOL
        {
            wchar_t className[256] = { 0 };
            GetClassName(child, className, 256);

            // Qt 窗口类名以 "Qt" 开头，libvlc 渲染子窗口不是 Qt 类，据此区分
            if (wcsstr(className, L"Qt") == nullptr)
            {
                EnableWindow(child, FALSE); // 禁用输入，鼠标事件回到 Qt 悬浮层

                // VLC 渲染窗口默认在 Z 序顶部会盖住悬浮层，沉到兄弟窗口最底部，
                // 保证悬浮层始终在画面之上。只改 Z 序，不动位置尺寸、不抢焦点
                SetWindowPos(child, HWND_BOTTOM, 0, 0, 0, 0,
                             SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

                // 注意：不手动 resize VLC 窗口。VLC 自己会响应父窗口 WM_SIZE
                // 正确 letterbox，手动 SetWindowPos 反而会导致画面黑边异常
            }
            return TRUE;
        },
        reinterpret_cast<LPARAM>(parent));
}
