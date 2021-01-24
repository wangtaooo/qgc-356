/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "VideoSettings.h"
#include "QGCApplication.h"
#include "VideoManager.h"

#include <QQmlEngine>
#include <QtQml>
#include <QVariantList>

#ifndef QGC_DISABLE_UVC
#include <QCameraInfo>
#endif
//下面是集中视频源的类型
const char* VideoSettings::videoSourceNoVideo   = "No Video Available";//初始化前面的变量
const char* VideoSettings::videoDisabled        = "Video Stream Disabled";
const char* VideoSettings::videoSourceRTSP      = "RTSP Video Stream";
const char* VideoSettings::videoSourceUDP       = "UDP Video Stream";
const char* VideoSettings::videoSourceTCP       = "TCP-MPEG2 Video Stream";
const char* VideoSettings::videoSourceMPEGTS    = "MPEG-TS (h.264) Video Stream";
const char* VideoSettings::videoSourceTomTest   = "Tom Test 20210123";

DECLARE_SETTINGGROUP(Video, "Video")
{
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    qmlRegisterUncreatableType<VideoSettings>("QGroundControl.SettingsManager", 1, 0, "VideoSettings", "Reference only");

    // Setup enum values for videoSource settings into meta data
    //Setup enum将videoSource设置的值转换为元数据
    QStringList videoSourceList;
#ifdef QGC_GST_STREAMING
    videoSourceList.append(videoSourceRTSP);
#ifndef NO_UDP_VIDEO
    videoSourceList.append(videoSourceUDP);
#endif
    videoSourceList.append(videoSourceTCP);
    videoSourceList.append(videoSourceMPEGTS);
    videoSourceList.append(videoSourceTomTest);
#endif
#ifndef QGC_DISABLE_UVC
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    for (const QCameraInfo &cameraInfo: cameras) {
        videoSourceList.append(cameraInfo.description());
    }
#endif
    if (videoSourceList.count() == 0) {
        _noVideo = true;
        videoSourceList.append(videoSourceNoVideo);//设置为没有视频源可用
    } else {
        videoSourceList.insert(0, videoDisabled);//在最前面插入disable
    }
    QVariantList videoSourceVarList;
    for (const QString& videoSource: videoSourceList) {
        videoSourceVarList.append(QVariant::fromValue(videoSource));
    }
    _nameToMetaDataMap[videoSourceName]->setEnumInfo(videoSourceList, videoSourceVarList);
    // Set default value for videoSource
    //为videoSource设置默认值
    _setDefaults();
}

void VideoSettings::_setDefaults()
{
    if (_noVideo) {
        _nameToMetaDataMap[videoSourceName]->setRawDefaultValue(videoSourceNoVideo);//设置为没有
    } else {
        _nameToMetaDataMap[videoSourceName]->setRawDefaultValue(videoDisabled);//设置为失能
    }
}

DECLARE_SETTINGSFACT(VideoSettings, aspectRatio)
DECLARE_SETTINGSFACT(VideoSettings, videoFit)
DECLARE_SETTINGSFACT(VideoSettings, gridLines)
DECLARE_SETTINGSFACT(VideoSettings, showRecControl)
DECLARE_SETTINGSFACT(VideoSettings, recordingFormat)
DECLARE_SETTINGSFACT(VideoSettings, maxVideoSize)
DECLARE_SETTINGSFACT(VideoSettings, enableStorageLimit)
DECLARE_SETTINGSFACT(VideoSettings, rtspTimeout)
DECLARE_SETTINGSFACT(VideoSettings, streamEnabled)
DECLARE_SETTINGSFACT(VideoSettings, disableWhenDisarmed)

//VideoSettings类,里面声明个变量对应的函数
DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, videoSource)
{
    if (!_videoSourceFact) {//如果没有这个Fact变量的话，创建它
        _videoSourceFact = _createSettingsFact(videoSourceName);
        //-- Check for sources no longer available 检查不再可用的来源
        if(!_videoSourceFact->enumStrings().contains(_videoSourceFact->rawValue().toString())) {
            if (_noVideo) {
                _videoSourceFact->setRawValue(videoSourceNoVideo);
            } else {
                _videoSourceFact->setRawValue(videoDisabled);
            }
        }
        connect(_videoSourceFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _videoSourceFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, udpPort)
{
    if (!_udpPortFact) {
        _udpPortFact = _createSettingsFact(udpPortName);
        connect(_udpPortFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _udpPortFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, udpPort1)//udpPort1
{
     if (!_udpPort1Fact) {
         _udpPort1Fact = _createSettingsFact(udpPort1Name);
         connect(_udpPort1Fact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
     }
     return _udpPort1Fact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, udpPort2)//udpPort2
{
     if (!_udpPort2Fact) {
         _udpPort2Fact = _createSettingsFact(udpPort2Name);
         connect(_udpPort2Fact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
     }
     return _udpPort2Fact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, udpPort3)//udpPort3
{
     if (!_udpPort3Fact) {
         _udpPort3Fact = _createSettingsFact(udpPort3Name);
         connect(_udpPort3Fact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
     }
     return _udpPort3Fact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtspUrl)
{
    if (!_rtspUrlFact) {
        _rtspUrlFact = _createSettingsFact(rtspUrlName);
        connect(_rtspUrlFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtspUrlFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, tcpUrl)
{
    if (!_tcpUrlFact) {
        _tcpUrlFact = _createSettingsFact(tcpUrlName);
        connect(_tcpUrlFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _tcpUrlFact;
}

bool VideoSettings::streamConfigured(void)//流配置
{
#if !defined(QGC_GST_STREAMING)
    return false;
#endif
    //-- First, check if it's disabled
    //首先，检查它是否被禁用
    QString vSource = videoSource()->rawValue().toString();
    if(vSource == videoSourceNoVideo || vSource == videoDisabled) {
        return false;
    }
    //-- Check if it's autoconfigured
    if(qgcApp()->toolbox()->videoManager()->autoStreamConfigured()) {
        qCDebug(VideoManagerLog) << "Stream auto configured";
        return true;
    }
    //-- If UDP, check if port is set
    if(vSource == videoSourceUDP) {
        qCDebug(VideoManagerLog) << "Testing configuration for UDP Stream:" << udpPort()->rawValue().toInt();
        qCDebug(VideoManagerLog) << "Testing configuration for UDP Stream:" << udpPort1()->rawValue().toInt();
        qCDebug(VideoManagerLog) << "Testing configuration for UDP Stream:" << udpPort2()->rawValue().toInt();
        qCDebug(VideoManagerLog) << "Testing configuration for UDP Stream:" << udpPort3()->rawValue().toInt();
        return udpPort()->rawValue().toInt() != 0;
        return udpPort1()->rawValue().toInt() != 0;
        return udpPort2()->rawValue().toInt() != 0;
        return udpPort3()->rawValue().toInt() != 0;
    }
    //-- If RTSP, check for URL
    if(vSource == videoSourceRTSP) {
        qCDebug(VideoManagerLog) << "Testing configuration for RTSP Stream:" << rtspUrl()->rawValue().toString();
        return !rtspUrl()->rawValue().toString().isEmpty();
    }
    //-- If TCP, check for URL
    if(vSource == videoSourceTCP) {
        qCDebug(VideoManagerLog) << "Testing configuration for TCP Stream:" << tcpUrl()->rawValue().toString();
        return !tcpUrl()->rawValue().toString().isEmpty();
    }
    //-- If MPEG-TS, check if port is set
    if(vSource == videoSourceMPEGTS) {
        qCDebug(VideoManagerLog) << "Testing configuration for MPEG-TS Stream:" << udpPort()->rawValue().toInt();
        return udpPort()->rawValue().toInt() != 0;
    }
    //-- If Tom Test, check if port is set
    if(vSource == videoSourceTomTest) {
        qCDebug(VideoManagerLog) << "Testing configuration for Tom Test:";
        return 0;
    }
    return false;
}

void VideoSettings::_configChanged(QVariant)
{
    emit streamConfiguredChanged();//触发流相关配置改变
}
