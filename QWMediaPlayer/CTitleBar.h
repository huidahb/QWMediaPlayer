#pragma once

// ============================================================================
// CTitleBar —— 自定义标题栏（Title Bar）
// 职责：呈现 logo 菜单、迷你模式、窗口置顶、最小化、最大化/还原、关闭按钮，
//       并负责鼠标按住拖动窗口、双击最大化/还原。
// 解耦设计：本类不直接操作窗口，一律通过 signals 向外通知，
//           由 MainWidget 连接信号后决定具体窗口行为。
// ============================================================================

#include <QtWidgets/QWidget>
#include <QLabel>

class QPushButton;

class CTitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit CTitleBar(QWidget* parent = nullptr);
    ~CTitleBar() override;

    // 由主窗口在最大化状态变化时回调，用于切换“最大化/还原”图标
    void setMaximized(bool maximized);

signals:
    void sig_minimize();            // 点击最小化
    void sig_toggleMaximize();      // 点击或双击，要求切换最大化/还原
    void sig_close();               // 点击关闭
    void sig_toggleTopMost(bool on);// 点击窗口置顶，on=true 表示置顶
    void sig_minimode();            // 点击迷你模式

protected:
    void mousePressEvent(QMouseEvent* event) override;   // 按住拖动
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    void initUI();

    QLabel* m_pLogoBtn     = nullptr; // “QW影音”文字 + 下拉菜单
    QPushButton* m_pMinimodeBtn = nullptr; // 迷你模式
    QPushButton* m_pSettopBtn   = nullptr; // 窗口置顶（可选中）
    QPushButton* m_pMinBtn      = nullptr; // 最小化
    QPushButton* m_pMaxBtn      = nullptr; // 最大化/还原
    QPushButton* m_pCloseBtn    = nullptr; // 关闭
};
