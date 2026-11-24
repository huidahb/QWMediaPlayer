#include "CFrameLessWidgetBase.h"

#include <QtWidgets/QApplication>
#include <QtGui/QMouseEvent>

#include <qt_windows.h>   // Qt 封装的 Windows 头，带入 HWND/MSG
#include <windowsx.h>    // GET_X_LPARAM/GET_Y_LPARAM 宏，安全取带符号坐标
#include <dwmapi.h>     // DWM(Desktop Window Manager，桌面窗口管理器)API，系统阴影


// user32：用户界面库(窗口/消息/光标)；dwmapi：DWM 导入库
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "dwmapi.lib")

CFrameLessWidgetBase::CFrameLessWidgetBase(QWidget* parent)
    : QWidget(parent)
{
    // FramelessWindowHint：去掉系统标题栏和边框
    // WindowMinMaxButtonsHint：保留任务栏右键的最小化/最大化菜单
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    setAttribute(Qt::WA_Hover); // WA(Widget Attribute，控件属性)-Hover，保证悬停事件投递

    // winId() 立即创建原生窗口；HWND(Handle to Window，窗口句柄)是窗口的“身份证号”
    HWND hwnd = reinterpret_cast<HWND>(winId());

    // GetWindowLongPtr：读取窗口长整型属性；GWL_STYLE 指定读“样式”
    const LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
    // SetWindowLongPtr：写回样式；WS(Window Style，窗口样式)用按位或叠加
    SetWindowLongPtr(hwnd, GWL_STYLE,
                     style | WS_THICKFRAME   // Thick Frame 可调大小框，DWM 阴影的关键依赖
                           | WS_CAPTION      // Caption 标题栏样式位
                           | WS_SYSMENU      // System Menu 系统菜单
                           | WS_MINIMIZEBOX  // Minimize Box 最小化按钮
                           | WS_MAXIMIZEBOX);// Maximize Box 最大化按钮
    // 无边框会移除上述样式，而 DWM 阴影依赖它们，故手动加回；视觉边框交给 WM_NCCALCSIZE 去除

    // 抑制 DWM 默认边框(浅色模式下为白边)
    // COLORREF(Color Reference，颜色引用)；34 = DWMWA_BORDER_COLOR 边框颜色属性
    const COLORREF noBorder = 0xFFFFFFFE; // 0xFFFFFFFE = DWMWA_COLOR_NONE，不绘制边框
    DwmSetWindowAttribute(hwnd, 34, &noBorder, sizeof(noBorder)); // 阴影独立合成，仍保留

    // 应用级过滤器：鼠标事件落在子控件上，统一在此检测边缘并触发系统缩放
    qApp->installEventFilter(this);
}

CFrameLessWidgetBase::~CFrameLessWidgetBase()
{
    qApp->removeEventFilter(this);

    // 兜底：销毁时移除可能残留的覆盖光标，保持 set/restore 计数平衡
    while (QApplication::overrideCursor() != nullptr)
        QApplication::restoreOverrideCursor();
}

// 统一拦截本窗口内所有控件的鼠标按下/移动：边缘按下交给系统缩放，悬停时换光标
bool CFrameLessWidgetBase::eventFilter(QObject* watched, QEvent* event)
{
    QWidget* source = qobject_cast<QWidget*>(watched);
    if (source == nullptr || source->window() != this)
        return QWidget::eventFilter(watched, event);

    if (event->type() != QEvent::MouseButtonPress &&
        event->type() != QEvent::MouseMove)
        return QWidget::eventFilter(watched, event);

    QMouseEvent* mouse = static_cast<QMouseEvent*>(event);
    const QPoint localPos = mapFromGlobal(mouse->globalPosition().toPoint());
    const Qt::Edges edges = edgesAt(localPos);

    if (event->type() == QEvent::MouseButtonPress)
    {
        if (mouse->button() == Qt::LeftButton && edges != Qt::Edges())
        {
            if (QWindow* handle = windowHandle())
                handle->startSystemResize(edges); // 系统接管原生缩放
            return true;                          // 拦截按下事件
        }
    }
    else if (mouse->buttons() == Qt::NoButton)
    {
        if (edges != Qt::Edges())
        {
            // 命中边缘：应用层覆盖缩放光标，不修改控件自身光标
            const QCursor desired = cursorForEdges(edges);
            const QCursor* overridden = QApplication::overrideCursor();
            if (overridden == nullptr)
                QApplication::setOverrideCursor(desired);    // 首次覆盖
            else if (overridden->shape() != desired.shape())
                QApplication::changeOverrideCursor(desired); // 换边：替换形状，计数不变
        }
        else if (QApplication::overrideCursor() != nullptr)
        {
            // 非边缘：移除覆盖，恢复控件自身光标（如小手）
            QApplication::restoreOverrideCursor();
        }
    }

    return QWidget::eventFilter(watched, event);
}


// 依据鼠标相对窗口的位置返回命中的边缘；最大化或不在边缘时返回空
Qt::Edges CFrameLessWidgetBase::edgesAt(const QPoint& localPos) const
{
    if (isMaximized())
        return Qt::Edges();

    Qt::Edges edges;
    if (localPos.x() < m_borderWidth)
        edges |= Qt::LeftEdge;
    if (localPos.x() > width() - m_borderWidth)
        edges |= Qt::RightEdge;
    if (localPos.y() < m_borderWidth)
        edges |= Qt::TopEdge;
    if (localPos.y() > height() - m_borderWidth)
        edges |= Qt::BottomEdge;
    return edges;
}

// 把命中边缘映射为对应光标：四角斜向、四边单向、其余普通箭头
QCursor CFrameLessWidgetBase::cursorForEdges(Qt::Edges edges) const
{
    const bool left   = edges & Qt::LeftEdge;
    const bool right  = edges & Qt::RightEdge;
    const bool top    = edges & Qt::TopEdge;
    const bool bottom = edges & Qt::BottomEdge;

    if ((left && top) || (right && bottom))
        return QCursor(Qt::SizeFDiagCursor);
    if ((right && top) || (left && bottom))
        return QCursor(Qt::SizeBDiagCursor);
    if (left || right)
        return QCursor(Qt::SizeHorCursor);
    if (top || bottom)
        return QCursor(Qt::SizeVerCursor);
    return QCursor(Qt::ArrowCursor);
}

bool CFrameLessWidgetBase::nativeEvent(const QByteArray& eventType,
                                       void* message, qintptr* result)
{
    Q_UNUSED(eventType);

    // MSG(Message，消息结构)：含窗口、消息编号、wParam/lParam 参数
    MSG* msg = static_cast<MSG*>(message);

    switch (msg->message)
    {
    // WM(Window Message，窗口消息)-NC(Non-Client，非客户区)-CALCSIZE(Calculate Size)
    // 非客户区尺寸计算：wParam(Word 参数)为 TRUE 时返回 0，
    // 整个窗口都作为客户区，移除系统标题栏和边框
    case WM_NCCALCSIZE:
        if (msg->wParam == TRUE)
        {
            *result = 0;
            return true;
        }
        return false;

    // WM-ERASEBKGND(Erase Background，擦除背景)：返回 1 表示背景自理，
    // 阻止系统用白色刷子擦除新区域，避免缩放闪白边
    case WM_ERASEBKGND:
        *result = 1;
        return true;

    // WM-ACTIVATE 窗口被激活：DwmExtendFrameIntoClientArea 把 DWM 帧扩展进客户区，
    // 配合加回的 WS_THICKFRAME 激活系统阴影
    case WM_ACTIVATE:
    {
        MARGINS margins; // 边距：左/右/上/下扩展宽度
        margins.cxLeftWidth    = 1;
        margins.cxRightWidth   = 1;
        margins.cyTopHeight    = 0;
        margins.cyBottomHeight = 1;
        const HRESULT hr = DwmExtendFrameIntoClientArea(msg->hwnd, &margins); // HRESULT 结果码
        *result = hr;
        return true;
    }

    default:
        return false;
    }
}
