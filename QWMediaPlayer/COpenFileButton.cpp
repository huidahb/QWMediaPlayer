#include "COpenFileButton.h"

#include "AppConfig.h"
#include "qss.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMenu>
#include <QtGui/QAction>
#include <QtWidgets/QFileDialog>
#include <QtCore/QStandardPaths>
#include <QtCore/QSettings>
#include <QtCore/QFileInfo>

namespace {
const int kLeftWidth    = 145;
const int kRightWidth   = 36;  // 收窄下拉按钮，使箭头更精致
const int kButtonHeight = 40;
const int kTotalWidth   = kLeftWidth + kRightWidth;
}

COpenFileButton::COpenFileButton(QWidget* parent)
    : QWidget(parent)
{
    // 左侧主按钮：图标 + 文字，左半圆角
    m_pOpenFileButton = new QPushButton(QStringLiteral("打开文件"), this);
    m_pOpenFileButton->setFixedSize(kLeftWidth, kButtonHeight);
    m_pOpenFileButton->setIcon(QIcon(":/videoWidget/resources/videoWidget/openfile.svg"));
    m_pOpenFileButton->setCursor(Qt::PointingHandCursor);
    m_pOpenFileButton->setStyleSheet(
        QString::fromUtf8(Qss::Video::openFileButton));

    // 右侧小箭头按钮：右半圆角，左侧竖线分隔；用 padding 缩小前景箭头
    m_pArrowButton = new QPushButton(this);
    m_pArrowButton->setFixedSize(kRightWidth, kButtonHeight);
    m_pArrowButton->setCursor(Qt::PointingHandCursor);
    m_pArrowButton->setStyleSheet(
        QString::fromUtf8(Qss::Video::arrowButton));

    // 下拉菜单：样式集中在 qss.h，宽度固定为整个胶囊宽度，便于整体居中对齐
    QMenu* menu = new QMenu(this);
    menu->setFixedWidth(kTotalWidth);
    menu->setStyleSheet(QString::fromUtf8(Qss::Video::openMenu));
    QAction* fileAction   = menu->addAction(QStringLiteral("打开文件"));
    QAction* folderAction = menu->addAction(QStringLiteral("打开文件夹"));

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(m_pOpenFileButton);
    layout->addWidget(m_pArrowButton);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    setFixedSize(kTotalWidth, kButtonHeight);

    connect(m_pOpenFileButton, &QPushButton::clicked,
            this, &COpenFileButton::on_openfile);

    // 手动弹出菜单：左上角与胶囊左上角对齐，菜单宽度等于胶囊，实现整体居中
    connect(m_pArrowButton, &QPushButton::clicked, this, [this, menu]() {
        const QPoint topLeft = mapToGlobal(QPoint(0, height() + 2));
        menu->exec(topLeft);
    });

    connect(fileAction, &QAction::triggered,
            this, &COpenFileButton::on_openfile);
    connect(folderAction, &QAction::triggered,
            this, &COpenFileButton::on_openFloder);
}

COpenFileButton::~COpenFileButton() = default;

void COpenFileButton::on_openfile()
{
    //打开 "持久化配置" 对象，使用本地注册表 
    QSettings settings(QString::fromLatin1(AppConfig::settingsPath),
                       QSettings::NativeFormat);           
    QString lastPath = settings.value("openfile_path").toString();

    if (lastPath.isEmpty())
        lastPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    QStringList fileList = QFileDialog::getOpenFileNames(
        this, QStringLiteral("选择要播放的文件"), lastPath,
        QStringLiteral("视频文件 (*.flv *.rmvb *.avi *.mp4 *.mkv);;"
                       "所有文件 (*.*)"));
    if (fileList.isEmpty())
        return;

    // 记住本次所在目录
    settings.setValue("openfile_path", QFileInfo(fileList.first()).absolutePath());

    emit sig_openfile(fileList);
}

void COpenFileButton::on_openFloder()
{
    const QString folder = QFileDialog::getExistingDirectory(
        this, QStringLiteral("选择文件夹"), QStringLiteral("/"));

    if (folder.isEmpty()) return;

    emit sig_openFloder(folder);
}
