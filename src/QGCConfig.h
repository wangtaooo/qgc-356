#ifndef QGC_CONFIGURATION_H
#define QGC_CONFIGURATION_H

#include <QString>

/** @brief Polling interval in ms */
#define SERIAL_POLL_INTERVAL 4

#define WITH_TEXT_TO_SPEECH 1

// If you need to make an incompatible changes to stored settings, bump this version number
// up by 1. This will caused store settings to be cleared on next boot.
//如果你需要对存储的设置进行不兼容的更改，将这个版本号增加1。这将导致存储设置在下一次引导时被清除。
#define QGC_SETTINGS_VERSION 8

#endif // QGC_CONFIGURATION_H
