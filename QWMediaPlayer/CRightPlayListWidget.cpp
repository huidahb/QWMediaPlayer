#include "CRightPlayListWidget.h"

#include "AppConfig.h"
#include "qss.h"

#include <QtWidgets/QListWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMenu>
#include <QtWidgets/QFileDialog>
#include <QtCore/QFileInfo>
#include <QtCore/QEvent>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>
#include <QtGui/QMouseEvent>

CRightPlayListWidget::CRightPlayListWidget(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground);
    setFixedWidth(260);

    QHBoxLayout* rootLay = new QHBoxLayout(this);
    rootLay->setContentsMargins(0, 0, 0, 0);
    rootLay->setSpacing(0);

    m_pCloseBtn = new QPushButton(this);
    m_pCloseBtn->setFixedSize(36, 80);
    m_pCloseBtn->setCursor(Qt::PointingHandCursor);     
    m_pCloseBtn->setStyleSheet(
        QString::fromUtf8(Qss::Playlist::closeButton));
    connect(m_pCloseBtn, &QPushButton::clicked,
            this, &CRightPlayListWidget::sig_closeRequested);

    m_pList = new QListWidget(this);
    m_pList->setFrameShape(QFrame::NoFrame);
    m_pList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_pList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pList->setStyleSheet(
        QString::fromUtf8(Qss::Playlist::list));

    // 顶部"+"工具条按钮：点击下拉菜单选择添加文件/添加文件夹
    m_pAddBtn = new QPushButton(QString::fromUtf8("＋  添加"), this);
    m_pAddBtn->setFixedHeight(34);
    m_pAddBtn->setCursor(Qt::PointingHandCursor);
    m_pAddBtn->setStyleSheet(
        QString::fromUtf8(Qss::Playlist::addButton));

    // 右侧容器：纵向排列“顶部工具条 + 列表”
    QWidget* rightContainer = new QWidget(this);
    QVBoxLayout* rightLay = new QVBoxLayout(rightContainer);
    rightLay->setContentsMargins(0, 0, 0, 0);
    rightLay->setSpacing(0);
    rightLay->addWidget(m_pAddBtn);
    rightLay->addWidget(m_pList);

    rootLay->addWidget(m_pCloseBtn);
    rootLay->addWidget(rightContainer);

    // 点击“+”弹出下拉菜单，菜单定位在按钮左下角
    connect(m_pAddBtn, &QPushButton::clicked, this, [this]() {
        QMenu menu(this);
        QAction* actFile   = menu.addAction(QString::fromUtf8("添加文件"));
        QAction* actFolder = menu.addAction(QString::fromUtf8("添加文件夹"));
        connect(actFile, &QAction::triggered,
                this, &CRightPlayListWidget::onAddFiles);
        connect(actFolder, &QAction::triggered,
                this, &CRightPlayListWidget::onAddFolder);
        const QPoint bottomLeft =
            m_pAddBtn->mapToGlobal(QPoint(0, m_pAddBtn->height()));
        menu.exec(bottomLeft);
    });

    // 鼠标左键的单击/双击统一在 eventFilter 中处理：
    // 单击拦截不选中（高亮始终停在播放项），双击 MouseButtonDblClick 才切播
    // 右键菜单策略：NoContextMenu + eventFilter 拦截右键按下，
    // 避免 Qt 默认行为把高亮移到右键点的项上
    m_pList->setContextMenuPolicy(Qt::NoContextMenu);
    m_pList->viewport()->installEventFilter(this);
}

CRightPlayListWidget::~CRightPlayListWidget() = default;

void CRightPlayListWidget::setFiles(const QStringList& files)
{
    m_pList->blockSignals(true);
    m_pList->clear();
    for (const QString& path : files)
        m_pList->addItem(createPathItem(path));
    m_pList->blockSignals(false);
}

void CRightPlayListWidget::appendFiles(const QStringList& files)
{
    // 只追加不 clear，也不调用 setCurrentRow，当前高亮保持不变
    m_pList->blockSignals(true);
    for (const QString& path : files)
        m_pList->addItem(createPathItem(path));
    m_pList->blockSignals(false);
}

QListWidgetItem* CRightPlayListWidget::createPathItem(const QString& path) const
{
    const QFileInfo info(path);
    QListWidgetItem* item = new QListWidgetItem(info.fileName());
    // UserRole 存完整绝对路径，作为去重判定依据；显示文本仍是文件名
    item->setData(Qt::UserRole, info.absoluteFilePath());
    return item;
}

QStringList CRightPlayListWidget::existingAbsolutePaths() const
{
    QStringList paths;
    const int count = m_pList->count();
    paths.reserve(count);
    for (int i = 0; i < count; ++i)
        paths.append(m_pList->item(i)->data(Qt::UserRole).toString());
    return paths;
}

void CRightPlayListWidget::setCurrentIndex(int index)
{
    if (index < 0 || index >= m_pList->count())
        return;
    m_pList->blockSignals(true);
    m_pList->setCurrentRow(index);
    m_pList->blockSignals(false);
}

bool CRightPlayListWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched != m_pList->viewport())
        return QWidget::eventFilter(watched, event);

    // 鼠标事件统一拦截，自己处理，不让 QListWidget 默认逻辑移动高亮
    switch (event->type())
    {
    case QEvent::MouseButtonPress:
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::RightButton)
        {
            // 右键按下：完全拦截，自己弹菜单，不让 QListWidget 选中该项
            QPoint pos = me->pos();
            QListWidgetItem* item = m_pList->itemAt(pos);
            if (item)
            {
                QMenu menu(this);
                QAction* actRemove = menu.addAction(QString::fromUtf8("移除"));
                connect(actRemove, &QAction::triggered, this, [this, item]() {
                    const int row = m_pList->row(item);
                    emit sig_removeRequested(row);
                    m_pList->blockSignals(true);
                    delete m_pList->takeItem(row);
                    m_pList->blockSignals(false);
                });
                menu.exec(m_pList->viewport()->mapToGlobal(pos));
            }
            return true;
        }
        // 左键单击：只拦截不选中，高亮保持在当前播放项
        if (me->button() == Qt::LeftButton)
            return true;
        break;
    }
    case QEvent::MouseButtonDblClick:
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton)
        {
            // 双击：切到对应视频（切播后内核会回报 currentIndex 驱动高亮）
            QListWidgetItem* item = m_pList->itemAt(me->pos());
            if (item)
                emit sig_itemSelected(m_pList->row(item));
        }
        return true;
    }
    case QEvent::MouseButtonRelease:
        // 释放也拦截，避免默认 release 选择逻辑改变选中
        return true;
    default:
        break;
    }
    return QWidget::eventFilter(watched, event);
}

void CRightPlayListWidget::onAddFiles()
{
    // 复用与"打开文件"相同的注册表位置，保持上次目录记忆一致
    QSettings settings(
        QString::fromLatin1(AppConfig::settingsPath),
        QSettings::NativeFormat);
    QString lastPath = settings.value("openfile_path").toString();
    if (lastPath.isEmpty())
        lastPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    const QStringList fileList = QFileDialog::getOpenFileNames(
        this, QString::fromUtf8("选择要添加的文件"), lastPath,
        QString::fromUtf8("视频文件 (*.flv *.rmvb *.avi *.mp4 *.mkv);;"
                          "所有文件 (*.*)"));
    if (fileList.isEmpty())
        return;

    settings.setValue("openfile_path",
                      QFileInfo(fileList.first()).absolutePath());

    emit sig_addFiles(fileList);
}

void CRightPlayListWidget::onAddFolder()
{
    const QString folder = QFileDialog::getExistingDirectory(
        this, QString::fromUtf8("选择要添加的文件夹"), QStringLiteral("/"));
    if (folder.isEmpty())
        return;

    emit sig_addFolder(folder);
}
