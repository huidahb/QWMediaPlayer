#pragma once

// ============================================================================
// CFrameLessWidgetBase —— 无边框窗口基类（FrameLess = 无窗口边框）
// 职责：
//   1. 去掉系统原生标题栏与边框（Qt::FramelessWindowHint）
//   2. 手动加回 Win32 的 WS_THICKFRAME 等样式，配合 WM_NCCALCSIZE 去帧，
//      从而在保持无边框外观的同时保留 DWM 系统阴影
//   3. 边缘缩放：应用级事件过滤器统一检测鼠标，按下边缘时调用 Qt 官方
//      QWindow::startSystemResize，由系统接管原生缩放（含 Aero Snap、光标）
// 子类只需继承本类即可获得“无边框 + 阴影 + 可缩放”能力，与业务界面解耦
// ============================================================================

#include <QtGui/QCursor>
#include <QtGui/QWindow>
#include <QtWidgets/QWidget>

class CFrameLessWidgetBase : public QWidget
{
    Q_OBJECT

public:
    explicit CFrameLessWidgetBase(QWidget* parent = nullptr);
    ~CFrameLessWidgetBase() override;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    bool nativeEvent(const QByteArray& eventType, void* message,
                     qintptr* result) override;

private:
    Qt::Edges edgesAt(const QPoint& localPos) const; // 鼠标命中的边缘
    QCursor cursorForEdges(Qt::Edges edges) const;   // 边缘对应的光标

    int m_borderWidth = 8; // 边缘可缩放区域宽度（逻辑像素，自动适配 DPI）
};
