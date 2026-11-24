#include "CVideoWidgetTopWidget.h"
#include "COpenFileButton.h"
#include "qss.h"

#include <QtWidgets/QPushButton>
#include <QtGui/QEnterEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QCursor>

CVideoWidgetTopWidget::CVideoWidgetTopWidget(QWidget* parent)
    : QWidget(parent)
{
    // 成为原生窗口（Native Window，独立 HWND），才能压在 libvlc 渲染窗口之上
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_TranslucentBackground); // 自身透明，仅显示子控件

    // 居中的“打开文件”胶囊按钮
    m_pOpenFileButton = new COpenFileButton(this);

    // 右侧播放列表开关（竖向窄按钮，左箭头图标）
    m_pRightListBtn = new QPushButton(this);
    m_pRightListBtn->setFixedSize(36, 80);
    m_pRightListBtn->setCursor(Qt::PointingHandCursor);
    m_pRightListBtn->setStyleSheet(
        QString::fromUtf8(Qss::Video::rightListButton));

    // 初始按钮隐藏（鼠标尚未进入）；悬浮层窗口本身保持可见
    hideButtons();

    // 子控件信号统一向外转发，本层不做业务处理
    connect(m_pOpenFileButton, &COpenFileButton::sig_openfile,
            this, &CVideoWidgetTopWidget::sig_OpenFile);
    connect(m_pOpenFileButton, &COpenFileButton::sig_openFloder,
            this, &CVideoWidgetTopWidget::sig_OpenFloder);
    connect(m_pRightListBtn, &QPushButton::clicked,
            this, &CVideoWidgetTopWidget::sig_OpenPlaylist);
}

CVideoWidgetTopWidget::~CVideoWidgetTopWidget() = default;

void CVideoWidgetTopWidget::setPlayStatus(bool playing)
{
    // 记录播放状态；按钮是否显示由 enter/leave 控制，播放中不显示打开按钮
    m_isPlaying = playing;
}

void CVideoWidgetTopWidget::setMediaAvailable(bool available)
{
    m_hasMedia = available;
    if (!m_hasMedia)
    {
        // 媒体被清空：立即隐藏播放列表按钮，即使鼠标此刻正悬停在控件上
        m_pRightListBtn->hide();
        return;
    }

    // 变为有媒体：若鼠标此刻正停在悬浮层区域内，立即刷新按钮，无需重新进入
    if (rect().contains(mapFromGlobal(QCursor::pos())))
        showButtons();
}

// 鼠标进入悬浮层：右侧播放列表按钮始终显示；打开按钮仅在未播放时显示
void CVideoWidgetTopWidget::enterEvent(QEnterEvent* event)
{
    showButtons();
    emit sig_hoverChanged(true);
    QWidget::enterEvent(event);
}

// 鼠标离开悬浮层：隐藏全部按钮。
// 若只是移动到内部子控件（按钮），全局坐标仍在窗口内，不隐藏
void CVideoWidgetTopWidget::leaveEvent(QEvent* event)
{
    if (!rect().contains(mapFromGlobal(QCursor::pos())))
    {
        hideButtons();
        emit sig_hoverChanged(false);
    }
    QWidget::leaveEvent(event);
}

void CVideoWidgetTopWidget::showButtons()
{
    // 无媒体时不显示播放列表按钮；打开按钮仅在未播放时显示
    m_pRightListBtn->setVisible(m_hasMedia);
    m_pOpenFileButton->setVisible(!m_isPlaying);
}

void CVideoWidgetTopWidget::hideButtons()
{
    m_pRightListBtn->hide();
    m_pOpenFileButton->hide();
}

void CVideoWidgetTopWidget::resizeEvent(QResizeEvent* event)
{
    // 打开按钮：水平垂直居中
    m_pOpenFileButton->move(width() / 2 - m_pOpenFileButton->width() / 2,
                            height() / 2 - m_pOpenFileButton->height() / 2);

    // 右侧按钮：贴右、垂直居中
    m_pRightListBtn->move(width() - m_pRightListBtn->width(),
                          height() / 2 - m_pRightListBtn->height() / 2);

    QWidget::resizeEvent(event);
}

void CVideoWidgetTopWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit sig_doubleClicked();

        event->accept();
    }
}
