/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include <QElapsedTimer>
#include <cmath>

#include "MultiVehicleManager.h"
#include "FirmwarePlugin.h"
#include "MAVLinkProtocol.h"
#include "FollowMe.h"
#include "Vehicle.h"
#include "PositionManager.h"
#include "SettingsManager.h"
#include "AppSettings.h"

FollowMe::FollowMe(QGCApplication* app, QGCToolbox* toolbox)
    : QGCTool(app, toolbox), estimatation_capabilities(0)
{
    memset(&_motionReport, 0, sizeof(motionReport_s));
    runTime.start();//启动定时器
    _gcsMotionReportTimer.setSingleShot(false);//计时器发射运动报告-初始时 关闭
}

void FollowMe::setToolbox(QGCToolbox* toolbox)
{
    QGCTool::setToolbox(toolbox);
    connect(&_gcsMotionReportTimer, &QTimer::timeout, this, &FollowMe::_sendGCSMotionReport);
    connect(toolbox->settingsManager()->appSettings()->followTarget(), &Fact::rawValueChanged, this, &FollowMe::_settingsChanged);
    _currentMode = _toolbox->settingsManager()->appSettings()->followTarget()->rawValue().toUInt();
    if(_currentMode == MODE_ALWAYS) {
        _enable();
    }
}

void FollowMe::followMeHandleManager(const QString&)
{
    //-- If we are to change based on current flight mode
    //如果我们要改变当前的飞行模式
    if(_currentMode == MODE_FOLLOWME) {
        QmlObjectListModel & vehicles = *_toolbox->multiVehicleManager()->vehicles();
        for (int i=0; i< vehicles.count(); i++) {
            Vehicle* vehicle = qobject_cast<Vehicle*>(vehicles[i]);
            if (vehicle->px4Firmware() && vehicle->flightMode().compare(FirmwarePlugin::px4FollowMeFlightMode, Qt::CaseInsensitive) == 0) {
                _enable();
                return;
            }
        }
        _disable();
    }
}

void FollowMe::_settingsChanged()
{
    uint32_t mode = _toolbox->settingsManager()->appSettings()->followTarget()->rawValue().toUInt();
    if(_currentMode != mode) {//修改模式的话，执行
        _currentMode = mode;
        switch (mode) {
        case MODE_NEVER:
            if(_gcsMotionReportTimer.isActive()) {
                _disable();
            }
            break;
        case MODE_ALWAYS:
            if(!_gcsMotionReportTimer.isActive()) {
                _enable();
            }
            break;
        case MODE_FOLLOWME:
            if(!_gcsMotionReportTimer.isActive()) {
                followMeHandleManager(QString());
            }
            break;
        default:
            break;
        }
    }
}

void FollowMe::_enable()
{
    connect(_toolbox->qgcPositionManager(),
            &QGCPositionManager::positionInfoUpdated,
            this,
            &FollowMe::_setGPSLocation);//当位置信息更新时，触发FollowMe::_setGPSLocation
    _gcsMotionReportTimer.setInterval(_toolbox->qgcPositionManager()->updateInterval());
    _gcsMotionReportTimer.start();//开启定时器
}

void FollowMe::_disable()
{
    disconnect(_toolbox->qgcPositionManager(),
               &QGCPositionManager::positionInfoUpdated,
               this,
               &FollowMe::_setGPSLocation);//关闭
    _gcsMotionReportTimer.stop();//关闭定时器
}

void FollowMe::_setGPSLocation(QGeoPositionInfo geoPositionInfo)
{
    if (!geoPositionInfo.isValid()) {
        //-- Invalidate cached coordinates
        //缓存失效坐标
        memset(&_motionReport, 0, sizeof(motionReport_s));
    } else {
        // get the current location coordinates
        //获取当前位置坐标

        QGeoCoordinate geoCoordinate = geoPositionInfo.coordinate();//使动作协调

        _motionReport.lat_int = geoCoordinate.latitude()  * 1e7;
        _motionReport.lon_int = geoCoordinate.longitude() * 1e7;
        _motionReport.alt     = geoCoordinate.altitude();

        estimatation_capabilities |= (1 << POS);//估算能力

        // get the current eph and epv
        //获取当前的eph和epv

        if(geoPositionInfo.hasAttribute(QGeoPositionInfo::HorizontalAccuracy) == true) {//水平精确度
            _motionReport.pos_std_dev[0] = _motionReport.pos_std_dev[1] = geoPositionInfo.attribute(QGeoPositionInfo::HorizontalAccuracy);
        }

        if(geoPositionInfo.hasAttribute(QGeoPositionInfo::VerticalAccuracy) == true) {//垂直精确度
            _motionReport.pos_std_dev[2] = geoPositionInfo.attribute(QGeoPositionInfo::VerticalAccuracy);
        }                

        // calculate z velocity if it's available
        //计算z轴速度，如果有的话

        if(geoPositionInfo.hasAttribute(QGeoPositionInfo::VerticalSpeed)) {
            _motionReport.vz = geoPositionInfo.attribute(QGeoPositionInfo::VerticalSpeed);
        }

        // calculate x,y velocity if it's available
        //计算x,y轴的速度，如果有的话

        if((geoPositionInfo.hasAttribute(QGeoPositionInfo::Direction)   == true) &&
           (geoPositionInfo.hasAttribute(QGeoPositionInfo::GroundSpeed) == true)) {

            estimatation_capabilities |= (1 << VEL);////估算能力

            qreal direction = _degreesToRadian(geoPositionInfo.attribute(QGeoPositionInfo::Direction));//方向需要角度转换
            qreal velocity  = geoPositionInfo.attribute(QGeoPositionInfo::GroundSpeed);

            _motionReport.vx = cos(direction)*velocity;
            _motionReport.vy = sin(direction)*velocity;

        } else {
            _motionReport.vx = 0.0f;//没有速度设置为0
            _motionReport.vy = 0.0f;
        }
    }
}

void FollowMe::_sendGCSMotionReport()//超时时，发送数据
{

    //-- Do we have any real data?
    //我们有真实的数据吗?
    if(_motionReport.lat_int == 0 && _motionReport.lon_int == 0 && _motionReport.alt == 0) {
        return;
    }

    QmlObjectListModel & vehicles    = *_toolbox->multiVehicleManager()->vehicles();
    MAVLinkProtocol* mavlinkProtocol = _toolbox->mavlinkProtocol();
    mavlink_follow_target_t follow_target;//创建mavlink数据包
    memset(&follow_target, 0, sizeof(follow_target));

    follow_target.timestamp = runTime.nsecsElapsed() * 1e-6;
    follow_target.est_capabilities = estimatation_capabilities;//填充mavlink并发送
    follow_target.position_cov[0] = _motionReport.pos_std_dev[0];
    follow_target.position_cov[2] = _motionReport.pos_std_dev[2];
    follow_target.alt = _motionReport.alt;
    follow_target.lat = _motionReport.lat_int;
    follow_target.lon = _motionReport.lon_int;
    follow_target.vel[0] = _motionReport.vx;
    follow_target.vel[1] = _motionReport.vy;

    for (int i=0; i< vehicles.count(); i++) {
        Vehicle* vehicle = qobject_cast<Vehicle*>(vehicles[i]);//获取要控制的飞机
        if(_currentMode || vehicle->flightMode().compare(FirmwarePlugin::px4FollowMeFlightMode, Qt::CaseInsensitive) == 0) {
            mavlink_message_t message;
            mavlink_msg_follow_target_encode_chan(mavlinkProtocol->getSystemId(),
                                                  mavlinkProtocol->getComponentId(),
                                                  vehicle->priorityLink()->mavlinkChannel(),
                                                  &message,
                                                  &follow_target);
            vehicle->sendMessageOnLink(vehicle->priorityLink(), message);//发送
        }
    }
}

double FollowMe::_degreesToRadian(double deg)
{
    return deg * M_PI / 180.0;
}
