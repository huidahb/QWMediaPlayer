#pragma once

// ============================================================================
// CBottomCtrlBar —— 底部控制栏
// 组成：左侧时间标签（默认隐藏）、居中播放按钮组、右侧工具箱/播放列表/全屏
// 职责：统一管理底部各控件，并把按钮组与全屏的信号向上转发
// ============================================================================

#include <QtWidgets/QWidget>

class QLabel;
class QPushButton;
class CPlayButtonGroupWidget;

class CBottomCtrlBar : public QWidget
{
    Q_OBJECT

public:
    explicit CBottomCtrlBar(QWidget* parent = nullptr);
    ~CBottomCtrlBar() override;

    void showTimeLabel(bool show);
    void setCurPlayTime(qint64 curMs);
    void setTotalTime(qint64 totalMs);
    void setPlaying(bool playing); // 转发给播放按钮组，同步播放/暂停图标

signals:
    void sig_play();
    void sig_stop();
    void sig_pre();
    void sig_next();
    void sig_volume(int);
    void sig_fullScreen();

private:
    QString formatTimeMs(qint64 timeMs) const;

    QLabel* m_pCurPlayTimeLabel = nullptr;
    QLabel* m_pTotalTimeLabel   = nullptr;
    CPlayButtonGroupWidget* m_pPlayButtonGroup = nullptr;
    QPushButton* m_pToolBoxButton   = nullptr;
    QPushButton* m_pEffectButton    = nullptr;
    QPushButton* m_pFullscreenButton = nullptr;
};
