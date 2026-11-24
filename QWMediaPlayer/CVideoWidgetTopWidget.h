#pragma once

// ============================================================================
// CVideoWidgetTopWidget —— 视频画面上的悬浮控制层（常驻，不动态隐藏）
// 职责：
//   1. 始终铺满视频画面容器并保持可见；自身背景透明，不影响观看视频
//   2. 鼠标进入时显示“打开文件/播放列表”按钮，离开时隐藏按钮（窗口本身
//      不隐藏），从而保证双击始终命中同一个窗口，避免窗口层级竞态
//   3. 向外转发打开文件、播放列表、双击全屏等信号
// 说明：进度条已独立到 VideoWidget 底部（不再悬浮），本类不管理滑块
// ============================================================================

#include <QtWidgets/QWidget>

class COpenFileButton;
class QPushButton;
class QEnterEvent;

class CVideoWidgetTopWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CVideoWidgetTopWidget(QWidget* parent = nullptr);
    ~CVideoWidgetTopWidget() override;

    void setPlayStatus(bool playing);       // 播放状态切换（决定打开按钮是否可用）
    void setMediaAvailable(bool available); // 媒体是否可用（false 时隐藏播放列表按钮）

signals:
    void sig_OpenFile(const QStringList& fileList);
    void sig_OpenFloder(const QString& folder);
    void sig_OpenPlaylist();
    void sig_doubleClicked();   // 双击悬浮层（全屏切换）
    void sig_hoverChanged(bool entered); // 鼠标进入/离开悬浮层（驱动进度条显隐）

protected:
    void enterEvent(QEnterEvent* event) override;   // 鼠标进入：显示按钮
    void leaveEvent(QEvent* event) override;        // 鼠标离开：隐藏按钮
    void resizeEvent(QResizeEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    void showButtons();   // 按当前播放状态显示对应按钮
    void hideButtons();

    COpenFileButton* m_pOpenFileButton = nullptr;
    QPushButton*     m_pRightListBtn   = nullptr;
    bool             m_isPlaying = false;
    bool             m_hasMedia = false;   // 是否存在已加载媒体
};
