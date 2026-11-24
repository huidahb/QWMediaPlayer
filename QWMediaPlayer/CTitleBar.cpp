#include "CTitleBar.h"
#include "qss.h"


#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtGui/QMouseEvent>

#include <qt_windows.h>

#pragma comment(lib, "user32.lib")

namespace {
// 标题栏图标统一 URL 前缀
const char* const kTitleIconPath = ":/titlebar/resources/titleBar/";

// 拼出标题栏某个图标的完整 qrc URL
std::string iconUrl(const std::string& fileName)
{
    return std::string(kTitleIconPath) + fileName;
}
} // namespace

CTitleBar::CTitleBar(QWidget* parent)
    : QWidget(parent)
{
    initUI();
}

CTitleBar::~CTitleBar() = default;

void CTitleBar::initUI()
{
    setAttribute(Qt::WA_StyledBackground); // 允许样式表绘制本控件背景
    setFixedHeight(32 + 5 * 2);            // 内容高 32，上下各留 5
    setStyleSheet(QString::fromUtf8(Qss::TitleBar::background));

    // ---- 左侧 logo 文字label----
    m_pLogoBtn = new QLabel(QStringLiteral("QW影音"), this);
    m_pLogoBtn->setFixedSize(138, 32);
    m_pLogoBtn->setStyleSheet(QString::fromUtf8(Qss::TitleBar::logoButton));


    // ---- 右侧功能按钮，样式由 qss.h 统一生成 ----
    m_pMinimodeBtn = new QPushButton(this);
    m_pMinimodeBtn->setFixedSize(32, 32);
    m_pMinimodeBtn->setCheckable(true); // 迷你模式为可选中状态，选中显示蓝色图标
    m_pMinimodeBtn->setStyleSheet(QString::fromStdString(
        Qss::TitleBar::checkableIconButton(iconUrl("minimode.svg").c_str(),
                                iconUrl("minimode_hover.svg").c_str(),
                                iconUrl("minimode_active.svg").c_str())));

    m_pSettopBtn = new QPushButton(this);
    m_pSettopBtn->setFixedSize(32, 32);
    m_pSettopBtn->setCheckable(true); // 置顶为可选中状态，选中显示蓝色图标
    m_pSettopBtn->setStyleSheet(QString::fromStdString(
        Qss::TitleBar::checkableIconButton(iconUrl("settop.svg").c_str(),
                                iconUrl("settop_hover.svg").c_str(),
                                iconUrl("settop_active.svg").c_str())));

    m_pMinBtn = new QPushButton(this);
    m_pMinBtn->setFixedSize(32, 32);
    m_pMinBtn->setStyleSheet(QString::fromStdString(
        Qss::TitleBar::iconButton(iconUrl("min.svg").c_str(),
                                iconUrl("min_hover.svg").c_str())));

    m_pMaxBtn = new QPushButton(this);
    m_pMaxBtn->setFixedSize(32, 32);
    m_pMaxBtn->setStyleSheet(QString::fromStdString(
        Qss::TitleBar::iconButton(iconUrl("max.svg").c_str(),
                                iconUrl("max_hover.svg").c_str())));

    m_pCloseBtn = new QPushButton(this);
    m_pCloseBtn->setFixedSize(32, 32);
    m_pCloseBtn->setStyleSheet(QString::fromStdString(
        Qss::TitleBar::iconButton(iconUrl("close.svg").c_str(),
                                iconUrl("close_hover.svg").c_str())));

    // ---- 水平布局：logo 居左，其余按钮居右，按钮间用固定间距分隔 ----
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(m_pLogoBtn);
    layout->addStretch();
    layout->addWidget(m_pMinimodeBtn);
    layout->addSpacing(20);
    layout->addWidget(m_pSettopBtn);
    layout->addSpacing(20);
    layout->addWidget(m_pMinBtn);
    layout->addSpacing(18);
    layout->addWidget(m_pMaxBtn);
    layout->addSpacing(18);
    layout->addWidget(m_pCloseBtn);
    layout->setContentsMargins(5, 5, 5, 5);

    // ---- 按钮点击仅向外发信号，不直接操作窗口 ----
    connect(m_pMinBtn, &QPushButton::clicked, this, &CTitleBar::sig_minimize);
    connect(m_pMaxBtn, &QPushButton::clicked, this, &CTitleBar::sig_toggleMaximize);
    connect(m_pCloseBtn, &QPushButton::clicked, this, &CTitleBar::sig_close);
    connect(m_pMinimodeBtn, &QPushButton::clicked, this, &CTitleBar::sig_minimode);
    connect(m_pSettopBtn, &QPushButton::clicked, this, [this]() {
        emit sig_toggleTopMost(m_pSettopBtn->isChecked());
    });
}

void CTitleBar::setMaximized(bool maximized)
{
    if (!m_pMaxBtn)
        return;

    // 最大化时显示“还原”图标，否则显示“最大化”图标
    const std::string normal = maximized ? iconUrl("normal.svg") : iconUrl("max.svg");
    const std::string hover  = maximized ? iconUrl("normal_hover.svg")
                                         : iconUrl("max_hover.svg");
    m_pMaxBtn->setStyleSheet(QString::fromStdString(
        Qss::TitleBar::iconButton(normal.c_str(), hover.c_str())));
}

void CTitleBar::mousePressEvent(QMouseEvent* event)
{
    // 左键按住标题栏：释放鼠标捕获并向窗口发送“移动”系统命令，实现拖动
    if (event->button() == Qt::LeftButton && ReleaseCapture())
    {
        QWidget* window = this->window();
        if (window->isWindow())
        {
            SendMessage(HWND(window->winId()), WM_SYSCOMMAND,
                        SC_MOVE + HTCAPTION, 0);
        }
    }
    QWidget::mousePressEvent(event);
}

void CTitleBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        emit sig_toggleMaximize();
}
