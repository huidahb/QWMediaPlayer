#pragma once

// ============================================================================
// VideoWidget —— 视频区容器（纯界面骨架，不含解码逻辑）
// 内部结构（参考 media player 的“视频 + 独立进度条”模式）：
//   QVBoxLayout
//   ├─ m_pVideoView  视频画面容器（黑色，libvlc 渲染到它的窗口句柄）
//   │   └─ CVideoWidgetTopWidget 悬浮层（打开文件/播放列表按钮，鼠标进入显示）
//   └─ m_pSlider     独立水平进度条（位于画面下方，不悬浮，不会被视频盖住）
// 职责：组装上述控件，并把打开文件、进度拖动、双击全屏等信号向外转发
// 解耦：本类只负责呈现与转发，不实现任何播放/解码能力
// ============================================================================

#include <QtWidgets/QWidget>
#include <QtGui/qwindowdefs.h> // WId（Window Identifier，窗口句柄类型）定义于此

class QEvent;
class QObject;
class QTimer;
class QGraphicsOpacityEffect;
class QPropertyAnimation;
class CVideoWidgetTopWidget;
class CTimeSlider;

class VideoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VideoWidget(QWidget* parent = nullptr);
    ~VideoWidget() override;

    // 视频画面窗口句柄（供 libvlc set_hwnd 作为渲染目标）
    WId videoViewId() const;

    void setMediaAvailable(bool available); // 媒体是否可用：false 时强制隐藏并复位进度条
    void setPlayStatus(bool playing);   // 更新播放状态（切换悬浮层内容）
    void setSliderRange(int max);       // 设置进度条范围（0~max）
    void setSliderValue(int value);     // 设置进度条当前值

signals:
    void sig_openFile(const QStringList& fileList); // 选择了文件
    void sig_openFolder(const QString& folder);     // 选择了文件夹
    void sig_sliderMoved(int value);               // 拖动进度条
    void sig_doubleClicked();                       // 双击画面（全屏切换）
    void sig_openPlaylist();                        // 打开播放列表

protected:
    // 用事件过滤器监听视频画面容器的进入/离开/尺寸/双击事件
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QWidget*               m_pVideoView  = nullptr; // 视频画面容器
    CVideoWidgetTopWidget* m_pTopWidget  = nullptr; // 画面上的悬浮层
    CTimeSlider*           m_pSlider     = nullptr; // 独立水平进度条
    QTimer*                m_pHideTimer   = nullptr; // 延迟隐藏进度条
    QGraphicsOpacityEffect* m_pSliderEffect = nullptr; // 进度条透明度效果
    QPropertyAnimation*    m_pShowAnim    = nullptr; // 淡入动画
    QPropertyAnimation*    m_pHideAnim   = nullptr; // 淡出动画
    bool                   m_hasMedia = false; // 是否已打开视频：false 时进度条不响应 hover
};
