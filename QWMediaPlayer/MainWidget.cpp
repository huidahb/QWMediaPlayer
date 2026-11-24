#include "MainWidget.h"

#include "CTitleBar.h"
#include "VideoWidget.h"
#include "CRightPlayListWidget.h"
#include "CBottomCtrlBar.h"
#include "CVlcPlayer.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QtGui/QGuiApplication>
#include <QtGui/QKeyEvent>
#include <QtGui/QScreen>
#include <QtCore/QEvent>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QSet>

namespace {
// 进度条统一用 0~1000 对应 libvlc 的 0.0~1.0，不依赖具体时长
const int kSliderMax = 1000;
}

MainWidget::MainWidget(QWidget* parent)
    : CFrameLessWidgetBase(parent)
{
    const QSize initialSize(1000, 620); // 适中的初始窗口大小
    resize(initialSize);

    // 取主屏幕可用区域（排除任务栏），将窗口居中放置
    const QRect available =
        QGuiApplication::primaryScreen()->availableGeometry();
    move(available.center() - QPoint(initialSize.width() / 2,
                                    initialSize.height() / 2));

    initUI();
    connectSignals();

}

MainWidget::~MainWidget() = default;

void MainWidget::initUI()
{
    m_pTitleBar    = new CTitleBar(this);
    m_pVideoWidget = new VideoWidget(this);
    m_pPlaylist    = new CRightPlayListWidget(this);
    m_pPlaylist->hide();
    m_pBottomBar   = new CBottomCtrlBar(this);

    // 创建播放内核，并把视频画面容器的窗口句柄绑定为输出目标
    m_pPlayer = new CVlcPlayer(this);
    m_pPlayer->setVideoOutput(m_pVideoWidget->videoViewId());

    // 整体纵向布局：标题栏 / 中部(视频+播放列表) / 底部栏
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_pTitleBar);

    QHBoxLayout* centerLayout = new QHBoxLayout;
    centerLayout->addWidget(m_pVideoWidget);
    centerLayout->addWidget(m_pPlaylist);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(centerLayout);

    mainLayout->addWidget(m_pBottomBar);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
}

void MainWidget::connectSignals()
{
    // ---- 标题栏：窗口级动作 ----
    connect(m_pTitleBar, &CTitleBar::sig_minimize,
            this, &QWidget::showMinimized);
    connect(m_pTitleBar, &CTitleBar::sig_toggleMaximize,
            this, &MainWidget::toggleMaximize);
    connect(m_pTitleBar, &CTitleBar::sig_close,
            this, &QWidget::close);
    connect(m_pTitleBar, &CTitleBar::sig_minimode,
            this, &MainWidget::toggleMinimode);
    connect(m_pTitleBar, &CTitleBar::sig_toggleTopMost,
            this, [this](bool on) {
                Qt::WindowFlags flags = windowFlags();
                if (on)
                    flags |= Qt::WindowStaysOnTopHint;
                else
                    flags &= ~Qt::WindowStaysOnTopHint;
                setWindowFlags(flags); // 重建窗口后需重新 show
                show();
            });

    // ---- 视频区 → 内核：打开文件/文件夹 ----
    connect(m_pVideoWidget, &VideoWidget::sig_openFile,
            this, [this](const QStringList& files) {
                m_pPlayer->openFiles(files);
                m_pPlaylist->setFiles(files);
                m_pPlaylist->show();
                m_pPlaylist->setCurrentIndex(0);
            });
    connect(m_pVideoWidget, &VideoWidget::sig_openFolder,
            this, [this](const QString& folder) {
                const QStringList files = collectVideoFiles(folder);
                m_pPlayer->openFiles(files);
                m_pPlaylist->setFiles(files);
                m_pPlaylist->show();
                m_pPlaylist->setCurrentIndex(0);
            });

    // 拖动进度条：滑块值换算为 0.0~1.0 位置后跳转
    connect(m_pVideoWidget, &VideoWidget::sig_sliderMoved,
            this, [this](int value) {
                m_pPlayer->setPosition(static_cast<float>(value) / kSliderMax);
            });

    connect(m_pVideoWidget, &VideoWidget::sig_doubleClicked,
            this, &MainWidget::toggleFullScreen);
    connect(m_pVideoWidget, &VideoWidget::sig_openPlaylist,
            this, &MainWidget::togglePlaylist);

    // 播放内核上报当前播放项 → 列表高亮；点列表项 → 切到对应视频
    connect(m_pPlayer, &CVlcPlayer::currentIndexChanged,
            m_pPlaylist, &CRightPlayListWidget::setCurrentIndex);
    connect(m_pPlaylist, &CRightPlayListWidget::sig_itemSelected,
            m_pPlayer, &CVlcPlayer::playAtIndex);
    connect(m_pPlaylist, &CRightPlayListWidget::sig_closeRequested,
            m_pPlaylist, &CRightPlayListWidget::hide);

    // 右键移除列表项 → 从内核播放列表删除
    connect(m_pPlaylist, &CRightPlayListWidget::sig_removeRequested,
            this, [this](int idx) {
                m_pPlayer->removeAtIndex(idx);
                // 删除后按 VLC 实际播放索引恢复高亮
                m_pPlaylist->setCurrentIndex(m_pPlayer->currentIndex());
            });

    // 顶部“+”添加文件：去重后添加，重复项由 addUniqueFiles 统一提示
    connect(m_pPlaylist, &CRightPlayListWidget::sig_addFiles,
            this, [this](const QStringList& files) {
                addUniqueFiles(files);
            });
    // 添加文件夹：先收集目录下视频，再走相同的去重添加流程
    connect(m_pPlaylist, &CRightPlayListWidget::sig_addFolder,
            this, [this](const QString& folder) {
                addUniqueFiles(collectVideoFiles(folder));
            });

    // ---- 底部栏 → 内核：播放控制 ----
    connect(m_pBottomBar, &CBottomCtrlBar::sig_play,
            this, [this]() { m_pPlayer->togglePlay(); });
    connect(m_pBottomBar, &CBottomCtrlBar::sig_stop,
            this, [this]() { m_pPlayer->stop(); });
    connect(m_pBottomBar, &CBottomCtrlBar::sig_pre,
            this, [this]() { m_pPlayer->previous(); });
    connect(m_pBottomBar, &CBottomCtrlBar::sig_next,
            this, [this]() { m_pPlayer->next(); });
    connect(m_pBottomBar, &CBottomCtrlBar::sig_volume,
            this, [this](int volume) { m_pPlayer->setVolume(volume); });
    connect(m_pBottomBar, &CBottomCtrlBar::sig_fullScreen,
            this, &MainWidget::toggleFullScreen);

    // ---- 内核 → 界面：进度回传，更新滑块 ----
    connect(m_pPlayer, &CVlcPlayer::positionChanged,
            this, [this](float position) {
                m_pVideoWidget->setSliderValue(
                    static_cast<int>(position * kSliderMax));
            });

    // 时间回传，更新底部时间标签
    connect(m_pPlayer, &CVlcPlayer::timeChanged,
            this, [this](qint64 currentMs, qint64 totalMs) {
                m_pBottomBar->setCurPlayTime(currentMs);
                m_pBottomBar->setTotalTime(totalMs);
            });

    // 播放状态回传，同步按钮图标、悬浮层与时间标签
    connect(m_pPlayer, &CVlcPlayer::playingChanged,
            this, [this](bool playing) {
                m_pVideoWidget->setPlayStatus(playing);
                m_pBottomBar->setPlaying(playing);
                m_pBottomBar->showTimeLabel(playing);
                if (playing)
                {
                    m_pVideoWidget->setSliderRange(kSliderMax);
                    if (m_pPlaylist->isVisible())
                        m_pPlaylist->hide();
                }
            });

    // 媒体列表数量变化：count=0（列表清空）时禁用并隐藏进度条，非空时恢复可用
    connect(m_pPlayer, &CVlcPlayer::mediaCountChanged, this,
            [this](int count) {
                m_pVideoWidget->setMediaAvailable(count > 0);
            });
}

QStringList MainWidget::collectVideoFiles(const QString& folder) const
{
    QDir dir(folder);
    // 只收集常见视频格式，按文件名排序
    const QStringList nameFilters = {
        "*.flv", "*.rmvb", "*.avi", "*.mp4", "*.mkv", "*.ts", "*.m3u8"
    };
    const QFileInfoList infos = dir.entryInfoList(
        nameFilters, QDir::Files | QDir::Readable, QDir::Name);

    QStringList files;
    for (const QFileInfo& info : infos)
        files.append(info.absoluteFilePath());
    return files;
}

QStringList MainWidget::removeDuplicateFiles(const QStringList& incoming) const
{
    // 规范化：绝对路径 + cleanPath 清理多余分隔符/./..，再转小写（Windows 大小写不敏感）
    auto normalize = [](const QString& path) {
        return QDir::cleanPath(QFileInfo(path).absoluteFilePath()).toLower();
    };

    // 先装入列表中已存在的路径作为去重基准
    QSet<QString> seen;
    for (const QString& path : m_pPlaylist->existingAbsolutePaths())
        seen.insert(normalize(path));

    // 逐个过滤：已存在或本批次重复的跳过，否则保留并登记
    QStringList unique;
    for (const QString& path : incoming)
    {
        const QString key = normalize(path);
        if (seen.contains(key))
            continue;
        seen.insert(key);
        unique.append(path);
    }
    return unique;
}

void MainWidget::addUniqueFiles(const QStringList& incoming)
{
    const QStringList unique = removeDuplicateFiles(incoming);
    const int skipped = incoming.size() - unique.size(); // 被跳过的重复项数量

    // 先完成实际添加，保证提示弹出时新文件确实已在列表中
    if (!unique.isEmpty())
    {
        m_pPlaylist->appendFiles(unique);
        m_pPlayer->addFiles(unique);
    }

    // 仅当存在重复项时弹信息提示，全部为新文件则不打扰
    if (skipped <= 0)
        return;

    const QString title = QString::fromUtf8("提示");
    QString text;
    if (unique.isEmpty())
        text = QString::fromUtf8("您选择的 %1 个文件均已在播放列表中，无需重复添加。")
                   .arg(skipped);
    else
        text = QString::fromUtf8("检测到 %1 个文件已在播放列表中，已自动跳过，"
                                 "其余 %2 个文件已添加。")
                   .arg(skipped)
                   .arg(unique.size());
    QMessageBox::information(this, title, text);
}

void MainWidget::changeEvent(QEvent* event)
{
    // 窗口最大化状态变化时，同步标题栏图标
    if (event->type() == QEvent::WindowStateChange)
        m_pTitleBar->setMaximized(isMaximized());
    CFrameLessWidgetBase::changeEvent(event);
}

void MainWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape && isFullScreen())
        showNormal();
    CFrameLessWidgetBase::keyPressEvent(event);
}

void MainWidget::toggleMaximize()
{
    if (isMaximized())
        showNormal();
    else
        showMaximized();
}

void MainWidget::toggleFullScreen()
{
    if (isFullScreen())
        showNormal();
    else
        showFullScreen();
}

void MainWidget::togglePlaylist()
{
    m_pPlaylist->setVisible(!m_pPlaylist->isVisible());
}

void MainWidget::toggleMinimode()
{
    const QSize miniSize(360, 240);

    if (!m_minimode)
    {
        m_normalGeometry = geometry();
        m_minimode = true;

        // 缩小后在屏幕可用区域居中
        QScreen* screen = QApplication::primaryScreen();
        const QPoint center = screen->availableGeometry().center();
        setGeometry(center.x() - miniSize.width() / 2,
                    center.y() - miniSize.height() / 2,
                    miniSize.width(), miniSize.height());
    }
    else
    {
        m_minimode = false;
        setGeometry(m_normalGeometry);
    }
}
