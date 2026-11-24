#pragma once

#include <QtWidgets/QWidget>

class QListWidget;
class QListWidgetItem;
class QPushButton;

class CRightPlayListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CRightPlayListWidget(QWidget* parent = nullptr);
    ~CRightPlayListWidget() override;

    void setFiles(const QStringList& files);
    void appendFiles(const QStringList& files); // 追加到列表末尾（不清空、不动高亮）
    void setCurrentIndex(int index);

    // 返回列表中所有视频的完整绝对路径（用于去重判断）
    QStringList existingAbsolutePaths() const;

signals:
    void sig_itemSelected(int index);
    void sig_closeRequested();
    void sig_removeRequested(int index); // 右键菜单"移除"
    void sig_addFiles(const QStringList& fileList); // 顶部"+"：添加文件
    void sig_addFolder(const QString& folder);      // 顶部"+"：添加文件夹

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onAddFiles();   // 弹出文件选择对话框
    void onAddFolder();  // 弹出文件夹选择对话框

private:
    // 创建显示文件名、UserRole 存完整绝对路径的列表项
    QListWidgetItem* createPathItem(const QString& path) const;

    QListWidget* m_pList;
    QPushButton* m_pCloseBtn;
    QPushButton* m_pAddBtn = nullptr; // 顶部"+"按钮
};