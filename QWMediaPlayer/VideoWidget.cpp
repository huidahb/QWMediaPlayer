#include "VideoWidget.h"
#include "CVideoWidgetTopWidget.h"
#include "CTimeSlider.h"
#include "qss.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGraphicsOpacityEffect>
#include <QtCore/QEvent>
#include <QtCore/QTimer>
#include <QtCore/QPropertyAnimation>

VideoWidget::VideoWidget(QWidget* parent)
    : QWidget(parent)
{
    // 整个视频区黑色背景（含独立进度条所在区域）
    setStyleSheet(QString::fromUtf8(Qss::Common::blackBackground));

    // 垂直布局：视频画面占满剩余空间，进度条贴底，二者分离不重叠
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ---- 视频画面容器：libvlc 的渲染目标 ----
    m_pVideoView = new QWidget(this);
    m_pVideoView->setAttribute(Qt::WA_NativeWindow); // 独立原生窗口，供 set_hwnd
    m_pVideoView->setStyleSheet(QString::fromUtf8(Qss::Common::blackBackground));
    m_pVideoView->installEventFilter(this); // 本类只负责把它的尺寸同步给悬浮层

    // 悬浮层父对象是画面容器，常驻显示并铺满（自身透明，不遮挡视频）。
    // 按钮的显隐由悬浮层按鼠标进入/离开自行管理
    m_pTopWidget = new CVideoWidgetTopWidget(m_pVideoView);
    m_pTopWidget->show();

    // ---- 独立水平进度条：位于画面下方，默认隐藏，鼠标进入视频区才淡入 ----
    m_pSlider = new CTimeSlider(this);
    m_pSlider->hide();
    m_pSlider->installEventFilter(this); // 监听进度条自身的 Enter/Leave

    // 透明度效果：QGraphicsOpacityEffect 负责整体淡入淡出
    m_pSliderEffect = new QGraphicsOpacityEffect(m_pSlider);
    m_pSliderEffect->setOpacity(0.0);
    m_pSlider->setGraphicsEffect(m_pSliderEffect);

    // 淡入动画：透明度 0 -> 1，200ms
    m_pShowAnim = new QPropertyAnimation(m_pSliderEffect, "opacity", this);
    m_pShowAnim->setDuration(200);
    m_pShowAnim->setStartValue(0.0);
    m_pShowAnim->setEndValue(1.0);

    // 淡出动画：透明度 1 -> 0，200ms；结束后才真正 hide()，让布局回收空间
    m_pHideAnim = new QPropertyAnimation(m_pSliderEffect, "opacity", this);
    m_pHideAnim->setDuration(200);
    m_pHideAnim->setStartValue(1.0);
    m_pHideAnim->setEndValue(0.0);
    connect(m_pHideAnim, &QPropertyAnimation::finished, m_pSlider, &QWidget::hide);

    mainLayout->addWidget(m_pVideoView, 1); // 画面拉伸占满
    mainLayout->addWidget(m_pSlider, 0);    // 进度条保持自身高度

    // 延迟隐藏定时器：鼠标离开视频区后，给 150ms 让用户把鼠标移到进度条上；
    // 到时启动淡出动画（而不是立刻 hide）
    m_pHideTimer = new QTimer(this);
    m_pHideTimer->setSingleShot(true);
    connect(m_pHideTimer, &QTimer::timeout, this, [this]() {
        m_pShowAnim->stop();
        m_pHideAnim->start();
    });

    // 鼠标进入视频区 → 停止淡出、淡入显示；离开 → 启动延迟隐藏
    connect(m_pTopWidget, &CVideoWidgetTopWidget::sig_hoverChanged, this,
            [this](bool entered) {
                // 未打开任何视频：进度条始终保持隐藏，hover 不做任何处理
                if (!m_hasMedia)
                    return;
                if (entered) {
                    m_pHideTimer->stop();
                    m_pHideAnim->stop();   // 若正在淡出，从当前透明度继续淡入
                    m_pSlider->show();
                    m_pShowAnim->start();
                } else {
                    m_pHideTimer->start(150);
                }
            });

    // ---- 转发悬浮层信号：本类只做传递，不处理具体业务 ----
    connect(m_pTopWidget, &CVideoWidgetTopWidget::sig_OpenFile,
            this, &VideoWidget::sig_openFile);
    connect(m_pTopWidget, &CVideoWidgetTopWidget::sig_OpenFloder,
            this, &VideoWidget::sig_openFolder);
    connect(m_pTopWidget, &CVideoWidgetTopWidget::sig_OpenPlaylist,
            this, &VideoWidget::sig_openPlaylist);
    connect(m_pTopWidget, &CVideoWidgetTopWidget::sig_doubleClicked,
            this, &VideoWidget::sig_doubleClicked);

    // 独立进度条拖动 → 向外转发
    connect(m_pSlider, &CTimeSlider::sig_SliderMove,
            this, &VideoWidget::sig_sliderMoved);
}

VideoWidget::~VideoWidget() = default;


WId VideoWidget::videoViewId() const
{
    // winId() 会确保画面容器已创建原生句柄
    return m_pVideoView->winId();
}

void VideoWidget::setMediaAvailable(bool available)
{
    m_hasMedia = available;
    m_pTopWidget->setMediaAvailable(available); // 同步媒体可用状态给悬浮层
    if (m_hasMedia)
        return;

    // 媒体列表已清空：停止全部显隐流程，把进度条复位并强制隐藏，
    // 防止无媒体时 hover 仍唤出“幽灵进度条”
    m_pHideTimer->stop();
    m_pShowAnim->stop();
    m_pHideAnim->stop();
    m_pSliderEffect->setOpacity(0.0);
    m_pSlider->setValue(0); // 程序化赋值不触发 sliderMoved，不会误发跳转信号
    m_pSlider->hide();
}

void VideoWidget::setPlayStatus(bool playing)
{
    m_pTopWidget->setPlayStatus(playing);
}

void VideoWidget::setSliderRange(int max)
{
    // 能设置进度范围即代表已打开视频，同步状态给悬浮层，进度条也允许响应 hover
    m_hasMedia = true;
    m_pTopWidget->setMediaAvailable(true);
    m_pSlider->setRange(0, max);
}

void VideoWidget::setSliderValue(int value)
{
    m_pSlider->setValue(value);
}

// 事件过滤器：
//  1. 画面容器尺寸变化时让悬浮层铺满
//  2. 进度条被鼠标进入 → 停止隐藏/淡出，保持显示；离开 → 准备隐藏
bool VideoWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_pVideoView && event->type() == QEvent::Resize)
    {
        m_pTopWidget->setGeometry(m_pVideoView->rect());
    }
    else if (watched == m_pSlider)
    {
        if (event->type() == QEvent::Enter)
        {
            m_pHideTimer->stop();
            m_pHideAnim->stop();
        }
        else if (event->type() == QEvent::Leave)
        {
            m_pHideTimer->start(150);
        }
    }

    return QWidget::eventFilter(watched, event);
}
