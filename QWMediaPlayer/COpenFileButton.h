#pragma once

// ============================================================================
// COpenFileButton —— “打开文件”胶囊按钮
// 结构：左侧“打开文件”主按钮 + 右侧带下拉菜单的小箭头按钮（打开文件/打开文件夹）
// 职责：弹出文件/文件夹选择对话框，并通过信号把选择结果向外发送
// 注：本类内部用 QSettings 记住上次打开目录，纯界面骨架下不做真实播放
// ============================================================================

#include <QtWidgets/QWidget>

class QPushButton;

class COpenFileButton : public QWidget
{
    Q_OBJECT

public:
    explicit COpenFileButton(QWidget* parent = nullptr);
    ~COpenFileButton() override;

signals:
    void sig_openfile(const QStringList& fileList);
    void sig_openFloder(const QString& folder);

private slots:
    void on_openfile();    // 弹出文件选择对话框
    void on_openFloder();  // 弹出文件夹选择对话框

private:
    QPushButton* m_pOpenFileButton = nullptr;
    QPushButton* m_pArrowButton    = nullptr;
};
