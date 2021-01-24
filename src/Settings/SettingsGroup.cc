/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "SettingsGroup.h"
#include "QGCCorePlugin.h"
#include "QGCApplication.h"

static const char* kJsonFile = ":/json/%1.SettingsGroup.json";

SettingsGroup::SettingsGroup(const QString& name, const QString& settingsGroup, QObject* parent)
    : QObject       (parent)
    , _visible      (qgcApp()->toolbox()->corePlugin()->overrideSettingsGroupVisibility(name))
    , _name         (name)
    , _settingsGroup(settingsGroup)
{
    //读取设置参数,通过参数 获取所有设置表
    _nameToMetaDataMap = FactMetaData::createMapFromJsonFile(QString(kJsonFile).arg(name), this);
}

///setting 的Fact成员
SettingsFact* SettingsGroup::_createSettingsFact(const QString& factName)//传入Facte类名
{
    FactMetaData* m = _nameToMetaDataMap[factName];
    if(!m) {
        qCritical() << "Fact name " << factName << "not found in" << QString(kJsonFile).arg(_name);//没查询到
        exit(-1);
    }
    return new SettingsFact(_settingsGroup, m, this);
}

