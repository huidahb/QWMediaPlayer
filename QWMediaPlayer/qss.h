#pragma once

// ============================================================================
// qss.h —— 全局样式表（QSS, Qt Style Sheets，Qt 样式表）集中管理
// 把散落在各 .cpp 中的样式字符串统一收纳，避免重复、便于换肤与维护。
// 组织方式：顶层命名空间 Qss，内部按界面模块划分子命名空间；
//           固定样式用 const char*，需要拼接路径的用 inline 函数返回 std::string。
// 设计约定：深色基调，主背景 rgb(45,45,45)/rgb(54,54,54)，选中蓝 #3D6DB5。
// ============================================================================

#include <string>

namespace Qss {

// ============================================================================
// Common —— 通用样式与跨模块复用的图标按钮生成函数
// ============================================================================
namespace Common {

// 纯黑背景（视频区与画面容器共用）
const char* const blackBackground = "background-color:rgb(0,0,0);";

// 播放控制条图标按钮：透明背景，hover 仅切换为 name_hover.svg，不加底色。
// 供 CPlayButtonGroupWidget、CBottomCtrlBar 复用，消除原先两处重复函数。
inline std::string playCtrlIconButton(const char* name)
{
    const std::string base =
        std::string(":/playCtrlBar/resources/playctrlBar/") + name;
    return "QPushButton{background:transparent;border:none;image:url("
           + base + ".svg);}"
           "QPushButton:hover{image:url(" + base + "_hover.svg);}";
}

} // namespace Common

// ============================================================================
// TitleBar —— 标题栏
// ============================================================================
namespace TitleBar {

// 标题栏底色
const char* const background = "background-color:rgb(54,54,54);";

// 左侧“QW影音”文字按钮：透明底、白色文字，悬停浮现灰色底
const char* const logoButton =
    "QLabel{"
    "color:rgb(255,255,255);"
    "background:transparent;"
    "border:none;"
    "text-align:left;"
    "padding-left:10px;"
    "font-family:'Microsoft YaHei';"
    "font-size:14px;"
    "}";

// 标题栏下拉菜单：深色风格
const char* const menu =
    "QMenu{"
    "background-color:rgb(54,54,54);"
    "border:1px solid rgb(80,80,80);"
    "color:rgb(255,255,255);"
    "padding:4px;"
    "font-family:'Microsoft YaHei';"
    "font-size:13px;"
    "}"
    "QMenu::item{padding:6px 28px 6px 20px;border-radius:4px;}"
    "QMenu::item:selected{background-color:rgb(99,99,99);}"
    "QMenu::separator{height:1px;background:rgb(80,80,80);margin:4px 8px;}";

// 标题栏图标按钮：常态显示 normalIcon，hover 铺灰底并显示 hoverIcon
inline std::string iconButton(const char* normalIcon, const char* hoverIcon)
{
    std::string qss;
    qss += "QPushButton{background-color:transparent;background-image:url(";
    qss += normalIcon;
    qss += ");background-repeat:no-repeat;background-position:center;border:none;}"
           "QPushButton:hover{background-color:rgb(99,99,99);background-image:url(";
    qss += hoverIcon;
    qss += ");background-repeat:no-repeat;background-position:center;border:none;}";
    return qss;
}

// 可选中图标按钮：在普通图标按钮基础上增加 checked(选中)态，
// 选中后显示 checkedIcon(选中蓝图标)，选中悬停时额外铺灰底
inline std::string checkableIconButton(const char* normalIcon,
                                       const char* hoverIcon,
                                       const char* checkedIcon)
{
    std::string qss;
    qss += "QPushButton{background-color:transparent;background-image:url(";
    qss += normalIcon;
    qss += ");background-repeat:no-repeat;background-position:center;border:none;}"
           "QPushButton:hover{background-color:rgb(99,99,99);background-image:url(";
    qss += hoverIcon;
    qss += ");background-repeat:no-repeat;background-position:center;border:none;}"
           "QPushButton:checked{background-color:transparent;background-image:url(";
    qss += checkedIcon;
    qss += ");background-repeat:no-repeat;background-position:center;border:none;}"
           "QPushButton:checked:hover{background-color:rgb(99,99,99);background-image:url(";
    qss += checkedIcon;
    qss += ");background-repeat:no-repeat;background-position:center;border:none;}";
    return qss;
}

} // namespace TitleBar

// ============================================================================
// Video —— 视频区：打开文件胶囊、时间进度条、悬浮层右侧列表按钮
// ============================================================================
namespace Video {

// 时间进度条：已播放(sub-page)橙色，未播放(add-page)灰色，白色圆形滑块
const char* const timeSlider =
    "QSlider{background:transparent;}"
    "QSlider::groove:horizontal{background:transparent;height:6px;}"
    "QSlider::add-page:horizontal{background-color:#3A3A3A;"
    "height:5px;border-radius:2px;}"
    "QSlider::sub-page:horizontal{background-color:#FF7826;"
    "height:5px;border-radius:2px;}"
    "QSlider::handle:horizontal{width:14px;height:14px;margin:-4px 0;"
    "border-radius:7px;background:rgb(255,255,255);}";

// “打开文件”胶囊左侧主按钮：左半圆角
const char* const openFileButton =
    "QPushButton{background-color:rgb(55,55,55);color:rgb(255,255,255);"
    "border:none;border-top-left-radius:20px;border-bottom-left-radius:20px;"
    "padding-left:16px;text-align:left;font-family:'Microsoft YaHei';font-size:14px;}"
    "QPushButton:hover{background-color:rgb(75,75,75);}";

// 胶囊右侧下拉箭头按钮：右半圆角，左侧竖线分隔
const char* const arrowButton =
    "QPushButton{background-color:rgb(55,55,55);border:none;"
    "border-top-right-radius:20px;border-bottom-right-radius:20px;"
    "background-image:url(:/videoWidget/resources/videoWidget/vline.svg);"
    "background-repeat:no-repeat;background-position:left center;"
    "image:url(:/titlebar/resources/titleBar/down_arrow.svg);"
    "padding:12px 10px;}"
    "QPushButton:hover{background-color:rgb(75,75,75);"
    "border-top-right-radius:20px;border-bottom-right-radius:20px;"
    "background-image:url(:/videoWidget/resources/videoWidget/vline.svg);"
    "background-repeat:no-repeat;background-position:left center;"
    "image:url(:/titlebar/resources/titleBar/down_arrow.svg);"
    "padding:12px 10px;}";

// 打开文件胶囊的下拉菜单
const char* const openMenu =
    "QMenu{background-color:rgb(55,55,55);border:1px solid rgb(45,45,45);"
    "border-radius:8px;}"
    "QMenu::item{color:rgb(255,255,255);background-color:rgb(55,55,55);"
    "padding:8px 32px;border-bottom:1px solid rgb(85,85,85);"
    "font-family:'Microsoft YaHei';font-size:14px;}"
    "QMenu::item:selected{background-color:rgb(75,75,75);}";

// 悬浮层右侧“播放列表”开关（左箭头图标）
const char* const rightListButton =
    "QPushButton{background-color:rgb(54,54,54);border:none;"
    "image:url(:/videoWidget/resources/videoWidget/left_arrow.svg);}"
    "QPushButton:hover{background-color:rgb(99,99,99);"
    "image:url(:/videoWidget/resources/videoWidget/left_arrow_hover.svg);}";

} // namespace Video

// ============================================================================
// Playlist —— 右侧悬浮播放列表：关闭按钮、列表、顶部“+”
// ============================================================================
namespace Playlist {

// 左侧收起按钮（右箭头图标）
const char* const closeButton =
    "QPushButton{background-color:rgb(54,54,54);border:none;"
    "image:url(:/videoWidget/resources/videoWidget/right_arrow.svg);}"
    "QPushButton:hover{background-color:rgb(99,99,99);"
    "image:url(:/videoWidget/resources/videoWidget/right_arrow_hover.svg);}";

// 列表本体及 item 的常态/选中/悬停配色
const char* const list =
    "QListWidget{background-color:rgb(45,45,45);border:none;"
    "outline:none;font-size:13px;}"
    "QListWidget::item{color:rgb(200,200,200);padding:8px 12px;"
    "border-bottom:1px solid rgb(35,35,35);}"
    "QListWidget::item:selected{background-color:#3D6DB5;color:white;}"
    "QListWidget::item:hover{background-color:rgb(60,60,60);}";

// 顶部“+ 添加”按钮
const char* const addButton =
    "QPushButton{background-color:rgb(45,45,45);color:rgb(200,200,200);"
    "border:none;border-bottom:1px solid rgb(35,35,35);"
    "text-align:left;padding-left:12px;font-size:13px;}"
    "QPushButton:hover{background-color:rgb(60,60,60);color:white;}";

} // namespace Playlist

// ============================================================================
// BottomBar —— 底部控制栏：底色、时间标签
// ============================================================================
namespace BottomBar {

// 底部栏底色
const char* const background = "background-color:rgb(67,67,67);";

// 时间标签文字样式
const char* const timeLabel =
    "QLabel{font-family:'Microsoft YaHei';font-size:16px;"
    "color:rgb(255,255,255);}";

} // namespace BottomBar

// ============================================================================
// Volume —— 音量按钮、音量弹窗及其滑块
// ============================================================================
namespace Volume {

// 音量按钮：透明背景，hover 切换图标
const char* const button =
    "QPushButton{background:transparent;border:none;"
    "image:url(:/playCtrlBar/resources/playctrlBar/audio_open.svg);}"
    "QPushButton:hover{"
    "image:url(:/playCtrlBar/resources/playctrlBar/audio_open_hover.svg);}";

// 音量弹窗背景
const char* const dialog =
    "QDialog{background-color:rgba(54,54,54,0.95);border-radius:4px;}";

// 弹窗内垂直滑块：add-page（手柄下方）蓝色，从底部向上填充
const char* const slider =
    "QSlider{background:transparent;}"
    "QSlider::groove:vertical{background:rgb(120,120,120);"
    "width:4px;border-radius:2px;}"
    "QSlider::add-page:vertical{background:rgb(0,160,233);"
    "width:4px;border-radius:2px;}"
    "QSlider::handle:vertical{width:12px;height:12px;margin:0 -4px;"
    "border-radius:6px;background:rgb(255,255,255);}";

} // namespace Volume

} // namespace Qss
