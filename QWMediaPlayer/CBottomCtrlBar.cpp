#include "CBottomCtrlBar.h"
#include "CPlayButtonGroupWidget.h"
#include "qss.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

CBottomCtrlBar::CBottomCtrlBar(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground);
    setMouseTracking(true);
    setFixedHeight(80);
    setStyleSheet(QString::fromUtf8(Qss::BottomBar::background));

    const QString labelQss =
        QString::fromUtf8(Qss::BottomBar::timeLabel);

    m_pCurPlayTimeLabel = new QLabel(QStringLiteral("00:00:00"), this);
    m_pCurPlayTimeLabel->setStyleSheet(labelQss);

    m_pTotalTimeLabel = new QLabel(QStringLiteral("00:00:00"), this);
    m_pTotalTimeLabel->setStyleSheet(labelQss);

    m_pPlayButtonGroup = new CPlayButtonGroupWidget(this);

    m_pToolBoxButton = new QPushButton(this);
    m_pToolBoxButton->setFixedSize(32, 32);
    m_pToolBoxButton->setStyleSheet(
        QString::fromStdString(Qss::Common::playCtrlIconButton("toolbox")));

    m_pEffectButton = new QPushButton(this);
    m_pEffectButton->setFixedSize(32, 32);
    m_pEffectButton->setStyleSheet(
        QString::fromStdString(Qss::Common::playCtrlIconButton("effect")));

    m_pFullscreenButton = new QPushButton(this);
    m_pFullscreenButton->setFixedSize(32, 32);
    m_pFullscreenButton->setStyleSheet(
        QString::fromStdString(Qss::Common::playCtrlIconButton("fullscreen")));

    // 右侧容器：收纳工具箱/效果/全屏三个按钮
    QWidget* rightContainer = new QWidget(this);
    QHBoxLayout* rightLayout = new QHBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->addStretch(); // 前置弹簧，把三个按钮推到容器（窗口）最右边
    rightLayout->addWidget(m_pToolBoxButton);
    rightLayout->addSpacing(18);
    rightLayout->addWidget(m_pEffectButton);
    rightLayout->addSpacing(18);
    rightLayout->addWidget(m_pFullscreenButton);

    // 左侧容器：收纳时间标签（默认隐藏）
    QWidget* leftContainer = new QWidget(this);
    QHBoxLayout* leftLayout = new QHBoxLayout(leftContainer);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->addWidget(m_pCurPlayTimeLabel);
    leftLayout->addWidget(m_pTotalTimeLabel);
    leftLayout->addStretch();

    // 三列网格：左右列等宽（stretch 均为 1），中间列放按钮组并居中，
    // 因此按钮组中心始终等于窗口水平中心，与“打开文件”按钮对齐
    QGridLayout* grid = new QGridLayout(this);
    grid->addWidget(leftContainer, 0, 0);
    grid->addWidget(m_pPlayButtonGroup, 0, 1, Qt::AlignCenter);
    grid->addWidget(rightContainer, 0, 2);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 0);
    grid->setColumnStretch(2, 1);
    grid->setContentsMargins(18, 0, 18, 0);

    showTimeLabel(false); // 初始不显示时间，与参考图片一致

    // 转发按钮组信号
    connect(m_pPlayButtonGroup, &CPlayButtonGroupWidget::sig_play,
            this, &CBottomCtrlBar::sig_play);
    connect(m_pPlayButtonGroup, &CPlayButtonGroupWidget::sig_stop,
            this, &CBottomCtrlBar::sig_stop);
    connect(m_pPlayButtonGroup, &CPlayButtonGroupWidget::sig_pre,
            this, &CBottomCtrlBar::sig_pre);
    connect(m_pPlayButtonGroup, &CPlayButtonGroupWidget::sig_next,
            this, &CBottomCtrlBar::sig_next);
    connect(m_pPlayButtonGroup, &CPlayButtonGroupWidget::sig_volume,
            this, &CBottomCtrlBar::sig_volume);
    connect(m_pFullscreenButton, &QPushButton::clicked,
            this, &CBottomCtrlBar::sig_fullScreen);
}

CBottomCtrlBar::~CBottomCtrlBar() = default;

void CBottomCtrlBar::setPlaying(bool playing)
{
    m_pPlayButtonGroup->setPlaying(playing);
}

void CBottomCtrlBar::showTimeLabel(bool show)
{
    m_pCurPlayTimeLabel->setVisible(show);
    m_pTotalTimeLabel->setVisible(show);
}

QString CBottomCtrlBar::formatTimeMs(qint64 timeMs) const
{
    const qint64 totalSeconds = timeMs / 1000;
    const int hours = totalSeconds / 3600;
    const int minutes = (totalSeconds % 3600) / 60;
    const int seconds = totalSeconds % 60;
    return QString::asprintf("%02d:%02d:%02d", hours, minutes, seconds);
}

void CBottomCtrlBar::setCurPlayTime(qint64 curMs)
{
    m_pCurPlayTimeLabel->setText(formatTimeMs(curMs));
}

void CBottomCtrlBar::setTotalTime(qint64 totalMs)
{
    m_pTotalTimeLabel->setText(QStringLiteral("/") + formatTimeMs(totalMs));
}
