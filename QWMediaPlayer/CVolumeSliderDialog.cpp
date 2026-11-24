#include "CVolumeSliderDialog.h"
#include "qss.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QSlider>
#include <QtCore/QEvent>

#include <qt_windows.h>

CVolumeSliderDialog::CVolumeSliderDialog(QWidget* parent)
    : QDialog(parent)
{
    setFixedSize(40, 120);
    setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);
    setStyleSheet(QString::fromUtf8(Qss::Volume::dialog));

    m_pSlider = new QSlider(Qt::Vertical, this);
    m_pSlider->setRange(0, 100);
    m_pSlider->setValue(50);
    m_pSlider->setStyleSheet(QString::fromUtf8(Qss::Volume::slider));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 10, 8, 10);
    layout->addWidget(m_pSlider);

    connect(m_pSlider, &QSlider::valueChanged,
            this, &CVolumeSliderDialog::sig_SliderValueChanged);
}

CVolumeSliderDialog::~CVolumeSliderDialog() = default;

void CVolumeSliderDialog::setSliderValue(int value)
{
    m_pSlider->setValue(value);
}

bool CVolumeSliderDialog::event(QEvent* event)
{
    // 窗口句柄变化时移除系统类的阴影样式，防止弹出框出现多余阴影
    if (event->type() == QEvent::WinIdChange)
    {
        HWND hwnd = reinterpret_cast<HWND>(winId());
        LONG style = ::GetClassLong(hwnd, GCL_STYLE);
        ::SetClassLong(hwnd, GCL_STYLE, style & ~CS_DROPSHADOW);
    }
    return QDialog::event(event);
}
