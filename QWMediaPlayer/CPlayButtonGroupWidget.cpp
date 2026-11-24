#include "CPlayButtonGroupWidget.h"
#include "CVolumeButton.h"
#include "qss.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>

CPlayButtonGroupWidget::CPlayButtonGroupWidget(QWidget* parent)
    : QWidget(parent)
{
    m_pStopButton = new QPushButton(this);
    m_pStopButton->setFixedSize(32, 32);
    m_pStopButton->setStyleSheet(
        QString::fromStdString(Qss::Common::playCtrlIconButton("stop")));

    m_pPreButton = new QPushButton(this);
    m_pPreButton->setFixedSize(32, 32);
    m_pPreButton->setStyleSheet(
        QString::fromStdString(Qss::Common::playCtrlIconButton("pre")));

    m_pPlayButton = new QPushButton(this);
    m_pPlayButton->setFixedSize(48, 48);
    m_pPlayButton->setStyleSheet(
        QString::fromStdString(Qss::Common::playCtrlIconButton("play")));

    m_pNextButton = new QPushButton(this);
    m_pNextButton->setFixedSize(32, 32);
    m_pNextButton->setStyleSheet(
        QString::fromStdString(Qss::Common::playCtrlIconButton("next")));

    m_pAudioButton = new CVolumeButton(this);
    m_pAudioButton->setFixedSize(32, 32);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(m_pStopButton);
    layout->addWidget(m_pPreButton);
    layout->addWidget(m_pPlayButton);
    layout->addWidget(m_pNextButton);
    layout->addWidget(m_pAudioButton);
    layout->setSpacing(8);
    layout->setContentsMargins(0, 0, 0, 0);

    // 4 个 32px 按钮 + 1 个 48px 按钮 + 4 个 8px 间距
    setFixedSize(32 * 4 + 48 + 8 * 4, 48);

    connect(m_pPlayButton, &QPushButton::clicked, this, [this]() {
        m_playing = !m_playing;
        updatePlayButtonIcon();
        emit sig_play();
    });
    connect(m_pStopButton, &QPushButton::clicked, this, [this]() {
        m_playing = false;
        updatePlayButtonIcon();
        emit sig_stop();
    });
    connect(m_pPreButton, &QPushButton::clicked, this, &CPlayButtonGroupWidget::sig_pre);
    connect(m_pNextButton, &QPushButton::clicked, this, &CPlayButtonGroupWidget::sig_next);

    // 转发音量按钮的音量值
    connect(m_pAudioButton, &CVolumeButton::sig_VolumeValue,
            this, &CPlayButtonGroupWidget::sig_volume);
}

CPlayButtonGroupWidget::~CPlayButtonGroupWidget() = default;

void CPlayButtonGroupWidget::setPlaying(bool playing)
{
    m_playing = playing;
    updatePlayButtonIcon();
}

void CPlayButtonGroupWidget::updatePlayButtonIcon()
{
    // 播放中显示暂停图标，否则显示播放图标
    m_pPlayButton->setStyleSheet(
        QString::fromStdString(
            Qss::Common::playCtrlIconButton(m_playing ? "pause" : "play")));
}
