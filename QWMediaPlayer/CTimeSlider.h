#pragma once

// ============================================================================
// CTimeSlider —— 时间进度条（水平 QSlider）
// 增强点：默认 QSlider 只有拖动滑块才会跳转，点击槽身不会；本类通过
//         eventFilter 拦截鼠标左键点击，按点击比例直接跳到对应位置。
// 信号：sig_SliderMove 向外通知进度变化，本类不关心播放内核如何响应
// ============================================================================

#include <QtWidgets/QSlider>

class CTimeSlider : public QSlider
{
    Q_OBJECT

public:
    explicit CTimeSlider(QWidget* parent = nullptr);
    ~CTimeSlider() override;

    void setTimeSliderRange(qint64 value);
    void setSliderValue(qint64 value);

signals:
    void sig_SliderMove(int value);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onSliderMoved(int position);

private:
    void seekOnMouseClick(QMouseEvent* event); // 按点击位置计算并跳转
};
