/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include "SettingsGroup.h"

class PlanViewSettings : public SettingsGroup
{
    Q_OBJECT
public:
    PlanViewSettings(QObject* parent = nullptr);
    DEFINE_SETTING_NAME_GROUP()
    // This is currently only used to set custom build visibility of the Plan view settings ui.
    // The settings themselves related to PlanView are in still in AppSettings due to historical reasons.

    //这是目前仅用于设置自定义构建可见性的平面图视图设置ui。
    //由于历史原因，与PlanView相关的设置本身仍然在AppSettings中。
};
