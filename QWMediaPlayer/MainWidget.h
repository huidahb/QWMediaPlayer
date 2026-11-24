#pragma once

// ============================================================================
// MainWidget —— 主窗口（协调者角色）
// 职责：
//   1. 用布局组装 标题栏 / 视频区（含右侧播放列表）/ 底部控制栏
//   2. 创建并持有播放内核 CVlcPlayer，连接界面与内核的双向信号
//   3. 处理窗口级动作：最小化、最大化、关闭、置顶、迷你模式、全屏
// 解耦：本类只做“接线”，界面模块不感知 libvlc，内核不感知具体控件
// ============================================================================

#include "CFrameLessWidgetBase.h"

class CTitleBar;
class VideoWidget;
class CRightPlayListWidget;
class CBottomCtrlBar;
class CVlcPlayer;

class MainWidget : public CFrameLessWidgetBase
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget* parent = nullptr);
    ~MainWidget() override;

protected:
    void changeEvent(QEvent* event) override;      // 同步最大化状态到标题栏
    void keyPressEvent(QKeyEvent* event) override; // Esc 退出全屏

private:
    void initUI();
    void connectSignals();                          // 连接界面与内核信号
    QStringList collectVideoFiles(const QString& folder) const; // 收集文件夹视频
    QStringList removeDuplicateFiles(const QStringList& incoming) const; // 过滤已存在的重复文件
    void addUniqueFiles(const QStringList& incoming); // 去重后添加，并对重复项给出信息提示

    void toggleMaximize();
    void toggleFullScreen();
    void togglePlaylist();
    void toggleMinimode();

    CTitleBar* m_pTitleBar          = nullptr;
    VideoWidget* m_pVideoWidget     = nullptr;
    CRightPlayListWidget* m_pPlaylist = nullptr;
    CBottomCtrlBar* m_pBottomBar    = nullptr;
    CVlcPlayer* m_pPlayer           = nullptr;

    bool m_minimode = false;     // 是否处于迷你模式
    QRect m_normalGeometry;       // 进入迷你模式前的窗口位置尺寸
};
