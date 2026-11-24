#pragma once

// ============================================================================
// CVolumeButton —— 音量控制按钮
// 行为：
//   1. 鼠标悬停时在按钮上方弹出音量滑块 CVolumeSliderDialog
//   2. 左键点击切换静音（静音时滑块归零）
//   3. 用定时器检测鼠标是否离开“按钮+弹窗”区域，离开则自动收起弹窗
// 信号：sig_VolumeValue 向外通知音量（0-100）
// ============================================================================

#include <QtWidgets/QPushButton>

class QEnterEvent;
class CVolumeSliderDialog;

class CVolumeButton : public QPushButton
{
    Q_OBJECT

public:
    explicit CVolumeButton(QWidget* parent);
    ~CVolumeButton() override;

signals:
    void sig_VolumeValue(int value);

protected:
    void enterEvent(QEnterEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void timerEvent(QTimerEvent* event) override;

private slots:
    // 滑块值变化的统一收口：同步静音状态与“静音前音量”，再向外转发
    void onVolumeChanged(int value);

private:
    CVolumeSliderDialog* m_pSliderDialog = nullptr;
    bool m_isMute = false;
    int  m_timerId = 0;
    int  m_lastVolume = 50; // 静音前音量，初值与滑块初始值 50 一致
};
