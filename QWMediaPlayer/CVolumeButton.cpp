#include "CVolumeButton.h"
#include "CVolumeSliderDialog.h"
#include "qss.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QEnterEvent>
#include <QtGui/QCursor>
#include <QtCore/QTimerEvent>

CVolumeButton::CVolumeButton(QWidget* parent)
    : QPushButton(parent)
{
    setStyleSheet(QString::fromUtf8(Qss::Volume::button));
}

CVolumeButton::~CVolumeButton()
{
    if (m_timerId)
        killTimer(m_timerId);
}

void CVolumeButton::enterEvent(QEnterEvent* event)
{
    if (!m_pSliderDialog)
    {
        m_pSliderDialog = new CVolumeSliderDialog(this);
        connect(m_pSliderDialog, &CVolumeSliderDialog::sig_SliderValueChanged,
                this, &CVolumeButton::onVolumeChanged);
    }

    // 把弹窗定位到按钮正上方并水平居中
    const QPoint topLeft = mapToGlobal(QPoint(0, 0));
    const int x = topLeft.x() + (width() - m_pSliderDialog->width()) / 2;
    const int y = topLeft.y() - m_pSliderDialog->height() - 5;
    m_pSliderDialog->move(x, y);
    m_pSliderDialog->show();

    if (!m_timerId)
        m_timerId = startTimer(250);

    QPushButton::enterEvent(event);
}

void CVolumeButton::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        // 非静音点击→静音归零；静音点击→恢复到静音前音量
        const int value = m_isMute ? m_lastVolume : 0;
        if (m_pSliderDialog)
            m_pSliderDialog->setSliderValue(value); // 触发信号链统一外发
        else
            onVolumeChanged(value);                 // 弹窗尚未创建时直接更新状态
    }
    QPushButton::mousePressEvent(event);
}

void CVolumeButton::onVolumeChanged(int value)
{
    // 非零值同步记忆并视为非静音；零值视为静音
    if (value > 0) {
        m_lastVolume = value;
        m_isMute = false;
    } else {
        m_isMute = true;
    }
    emit sig_VolumeValue(value);
}

void CVolumeButton::timerEvent(QTimerEvent* event)
{
    if (!m_pSliderDialog || !m_pSliderDialog->isVisible())
        return;

    // 合并“按钮 + 弹窗”的整体矩形，判断鼠标是否仍在其中
    const QPoint btnTopLeft = mapToGlobal(QPoint(0, 0));
    const QRect buttonRect(btnTopLeft, size());
    const QRect dialogRect(m_pSliderDialog->pos(), m_pSliderDialog->size());
    const QRect hoverArea = buttonRect.united(dialogRect);

    if (!hoverArea.contains(QCursor::pos()))
        m_pSliderDialog->hide();

    QPushButton::timerEvent(event);
}
