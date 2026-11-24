#pragma once

// ============================================================================
// CPlayButtonGroupWidget —— 播放控制按钮组
// 组成：停止、上一个、播放/暂停、下一个、音量（从左到右）
// 纯界面行为：播放按钮点击后在“播放/暂停”图标间切换；停止按钮复位图标
// 解耦：仅通过信号向外通知，不接触播放内核；提供 setPlaying 供外部同步状态
// ============================================================================

#include <QtWidgets/QWidget>

class QPushButton;
class CVolumeButton;

class CPlayButtonGroupWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CPlayButtonGroupWidget(QWidget* parent = nullptr);
    ~CPlayButtonGroupWidget() override;

    void setPlaying(bool playing); // 同步播放状态并切换播放按钮图标

signals:
    void sig_play();       // 点击播放/暂停
    void sig_stop();       // 点击停止
    void sig_next();       // 点击下一个
    void sig_pre();        // 点击上一个
    void sig_volume(int);  // 音量变化（0~100）

private:
    void updatePlayButtonIcon(); // 根据 m_playing 切换图标

    QPushButton* m_pStopButton  = nullptr;
    QPushButton* m_pPreButton   = nullptr;
    QPushButton* m_pPlayButton  = nullptr;
    QPushButton* m_pNextButton  = nullptr;
    CVolumeButton* m_pAudioButton = nullptr;

    bool m_playing = false;
};
