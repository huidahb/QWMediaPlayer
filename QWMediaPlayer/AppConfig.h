#pragma once

// ============================================================================
// AppConfig.h —— 应用级配置常量集中管理
// 与样式文件 qss.h 分离：这里只放与应用配置相关的常量，便于统一维护
// ============================================================================

namespace AppConfig {

// QSettings（Qt 配置读写类）在注册表中的精确位置，
// 用于记住上次打开/添加文件的目录（键名 openfile_path）
const char* const settingsPath =
    "HKEY_CURRENT_USER\\Software\\QWMediaPlayer";

} // namespace AppConfig
