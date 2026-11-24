#pragma once

// ============================================================================
// CVlcPlayer —— libvlc 播放内核封装（QObject）
// 职责：
//   1. 把 libvlc 的 C 接口（实例/播放器/媒体列表/事件回调）封装成 Qt 对象
//   2. 对外只暴露简单方法与 Qt 信号，界面层无需包含任何 libvlc 头文件
//   3. 在静态回调中把 C 事件转换为 Qt 信号，实现内核与界面解耦
// 设计：头文件仅前置声明 libvlc 不透明类型，C 头文件只在 .cpp 中包含
// ============================================================================

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtGui/qwindowdefs.h> // WId（Window Identifier，窗口句柄类型）定义于此

// libvlc 不透明句柄前置声明（与 vlc.h 中的 typedef 一致，可重复声明）
typedef struct libvlc_instance_t libvlc_instance_t;
typedef struct libvlc_media_player_t libvlc_media_player_t;
typedef struct libvlc_media_list_t libvlc_media_list_t;
typedef struct libvlc_media_list_player_t libvlc_media_list_player_t;
typedef struct libvlc_event_t libvlc_event_t;

class QTimer;

class CVlcPlayer : public QObject
{
    Q_OBJECT

public:
    explicit CVlcPlayer(QObject* parent = nullptr);
    ~CVlcPlayer() override;

    // 绑定视频输出窗口（WId = Window Identifier，Qt 窗口句柄；Windows 即 HWND）
    void setVideoOutput(WId windowId);

    // 加载文件列表并开始播放
    void openFiles(const QStringList& files);

    // 追加文件到现有列表末尾（不清空、不打断当前播放）；
    // 若当前列表为空，则补做关联并自动播放第一个
    void addFiles(const QStringList& files);

    void togglePlay();  // 播放/暂停切换
    void stop();        // 停止
    void next();        // 下一个
    void previous();    // 上一个
    void playAtIndex(int index); // 跳到列表中第 index 个
    void removeAtIndex(int index); // 从播放列表移除第 index 个
    int currentIndex() const { return m_iCurrentIndex; } // 当前播放索引

    // 跳转进度（position：位置，取值 0.0~1.0）
    void setPosition(float position);

    // 设置音量（volume：取值 0~100）
    void setVolume(int volume);

signals:
    void positionChanged(float position);       // 播放进度（0.0~1.0）
    void timeChanged(qint64 currentMs, qint64 totalMs); // 时间（毫秒）
    void volumeChanged(int volume);             // 音量
    void playingChanged(bool playing);          // 播放状态（true 播放中）
    void currentIndexChanged(int index);     // 当前播放项索引变化
    void mediaCountChanged(int count);       // 媒体列表数量变化（count=0 即列表已清空）

    // 内部信号：libvlc 回调在后台线程，QTimer 必须在本对象所属的主线程创建/启停，
    // 故跨线程发出此信号，由 QueuedConnection 投递到主线程槽处理
    void requestFixSubWindow(bool playing);

private:
    // libvlc 只能接受普通函数指针，故用静态回调；userData 携带本对象
    static void onVlcEvent(const libvlc_event_t* event, void* userData);

    void handlePositionChanged();   // 处理进度/时间事件
    void handleVolumeChanged();     // 处理音量事件
    void handleStateChanged();      // 处理播放状态事件（libvlc 后台线程）
    void handleMediaChanged();       // 处理切换视频事件，上报索引

    // 把文件逐个包装成 media 并追加到 m_pMediaList（openFiles/addFiles 共用）
    void appendMediaToMediaList(const QStringList& files);
    // 关联列表播放器与媒体列表、设置输出与循环模式并开始播放
    void bindListAndPlay();

    // 主线程槽：根据播放状态启停轮询定时器（这里才是安全操作 QTimer 的地方）
    void onRequestFixSubWindow(bool playing);

    // 禁用 libvlc 渲染子窗口的输入，让鼠标事件回到 Qt 窗口
    void disableVideoSubWindow();

    libvlc_instance_t* m_pInstance = nullptr;
    libvlc_media_player_t* m_pPlayer = nullptr;
    libvlc_media_list_t* m_pMediaList = nullptr;
    libvlc_media_list_player_t* m_pListPlayer = nullptr;

    void* m_pVideoHwnd = nullptr;  // 视频输出窗口句柄
    qint64 m_totalMs = 0;           // 当前媒体总时长（毫秒）
    int m_iCurrentIndex = -1;         // 当前播放索引

    // 播放期间持续运行：VLC 切换视频可能重建渲染窗口，需反复重新禁用/沉底，
    // 不能一次性找到就停止，否则新窗口会重新吞掉鼠标事件
    QTimer* m_pSubWindowTimer = nullptr;
};
