#include "CTimeSlider.h"

#include "qss.h"

#include <QtCore/QEvent>
#include <QtGui/QMouseEvent>

CTimeSlider::CTimeSlider(QWidget* parent)
    : QSlider(Qt::Horizontal, parent)
{
    // 高度不再写死：由 VideoWidget 通过 maximumHeight 做展开/收起动画
    // （展开 15px，收起 0），实现 hover 时淡入、移走时淡出
    setMinimumHeight(0);
    installEventFilter(this); // 拦截自身鼠标事件，实现点击跳转

    // 样式集中在 qss.h：已播放橙色、未播放灰色、白色圆形滑块
    setStyleSheet(QString::fromUtf8(Qss::Video::timeSlider));

    connect(this, &QSlider::sliderMoved,
            this, &CTimeSlider::onSliderMoved);
}

CTimeSlider::~CTimeSlider() = default;

bool CTimeSlider::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == this && event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton)
            seekOnMouseClick(mouseEvent);
    }
    return QSlider::eventFilter(watched, event);
}

void CTimeSlider::seekOnMouseClick(QMouseEvent* event)
{
    const int minHint = minimumSizeHint().width();      // 滑块本身占用的最小宽度
    const int wndWidth = width();       // 控件当前总宽度
    if (wndWidth <= minHint)
        return;

    // 按点击横坐标占槽身的比例，换算出对应的逻辑位置
    // Qt6 中 QMouseEvent::x() 已弃用，改用 position() 返回的 QPointF
    const qreal clickX = event->position().x();
    const int target = minimum() +
        static_cast<int>((maximum() - minimum()) *
                         ((clickX - minHint / 2.0) / (wndWidth - minHint)));
    setSliderPosition(target);
    // 点击不会触发 sliderMoved（那是拖动专用信号），手动通知播放内核跳转
    emit sig_SliderMove(target);
}

void CTimeSlider::onSliderMoved(int position)
{
    emit sig_SliderMove(position);
}

void CTimeSlider::setTimeSliderRange(qint64 value)
{
    setRange(0, static_cast<int>(value));
}

void CTimeSlider::setSliderValue(qint64 value)
{
    setValue(static_cast<int>(value));
}
