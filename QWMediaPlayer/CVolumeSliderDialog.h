#pragma once

// ============================================================================
// CVolumeSliderDialog —— 音量滑块弹出框
// 无边框、ToolTip 风格（失去焦点自动关闭），内含一个垂直 QSlider
// ============================================================================

#include <QtWidgets/QDialog>

class QSlider;

class CVolumeSliderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CVolumeSliderDialog(QWidget* parent = nullptr);
    ~CVolumeSliderDialog() override;

    void setSliderValue(int value);

signals:
    void sig_SliderValueChanged(int value);

protected:
    bool event(QEvent* event) override; // 去除系统阴影，避免与自定义样式冲突

private:
    QSlider* m_pSlider = nullptr;
};
