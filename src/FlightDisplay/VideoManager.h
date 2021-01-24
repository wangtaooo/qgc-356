/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


#ifndef VideoManager_H
#define VideoManager_H

#include <QObject>
#include <QTimer>
#include <QTime>
#include <QUrl>

#include "QGCMAVLink.h"
#include "QGCLoggingCategory.h"
#include "VideoReceiver.h"
#include "QGCToolbox.h"

Q_DECLARE_LOGGING_CATEGORY(VideoManagerLog)

class VideoSettings;
class Vehicle;
class Joystick;

class VideoManager : public QGCTool
{
    Q_OBJECT

public:
    VideoManager    (QGCApplication* app, QGCToolbox* toolbox);
    ~VideoManager   ();

    Q_PROPERTY(bool             hasVideo                READ    hasVideo                                    NOTIFY hasVideoChanged)
    Q_PROPERTY(bool             isGStreamer             READ    isGStreamer                                 NOTIFY isGStreamerChanged)
    Q_PROPERTY(bool             isTaisync               READ    isTaisync       WRITE   setIsTaisync        NOTIFY isTaisyncChanged)
    Q_PROPERTY(QString          videoSourceID           READ    videoSourceID                               NOTIFY videoSourceIDChanged)
    Q_PROPERTY(bool             uvcEnabled              READ    uvcEnabled                                  CONSTANT)
    Q_PROPERTY(bool             fullScreen              READ    fullScreen      WRITE   setfullScreen       NOTIFY fullScreenChanged)
    Q_PROPERTY(VideoReceiver*   videoReceiver           READ    videoReceiver                               CONSTANT)
    Q_PROPERTY(double           aspectRatio             READ    aspectRatio                                 NOTIFY aspectRatioChanged)
    //声明一个视频流是否自动配置的函数      //接口函数                  //对应内部函数:一个读、一个写
    Q_PROPERTY(bool             autoStreamConfigured    READ    autoStreamConfigured                        NOTIFY autoStreamConfiguredChanged)

//下面是 几个可能在qml中调用的接口函数的声明
    bool        hasVideo            ();//检查是否有视频流的函数
    bool        isGStreamer         ();//检查是否为gst
    bool        isAutoStream        ();//自动视频流
    bool        isTaisync           () { return _isTaisync; }
    bool        fullScreen          () { return _fullScreen; }
    //函数接口为直接返回变量型
    QString     videoSourceID       () { return _videoSourceID; }
    double      aspectRatio         ();//未实现
    bool        autoStreamConfigured();

    VideoReceiver*  videoReceiver   () { return _videoReceiver; }

#if defined(QGC_DISABLE_UVC)
    bool        uvcEnabled          () { return false; }
#else
    bool        uvcEnabled          ();
#endif
    //设置是否为全屏
    void        setfullScreen       (bool f) { _fullScreen = f; emit fullScreenChanged(); }//设置全局变量，并触发信号
    void        setIsTaisync        (bool t) { _isTaisync = t;  emit isTaisyncChanged(); }

    // Override from QGCTool
    //初始化从这里开始重载设置函数
    void        setToolbox          (QGCToolbox *toolbox);

    Q_INVOKABLE void startVideo     () { _videoReceiver->start(); }
    Q_INVOKABLE void stopVideo      () { _videoReceiver->stop();  }

signals:
    void hasVideoChanged            ();
    void isGStreamerChanged         ();
    void videoSourceIDChanged       ();
    void fullScreenChanged          ();//设置全屏的信号，默认是小屏
    void isAutoStreamChanged        ();
    void isTaisyncChanged           ();
    void aspectRatioChanged         ();
    void autoStreamConfiguredChanged();

private slots:
    void _videoSourceChanged        ();
    void _udpPortChanged            ();
    void _udpPortChanged1            ();
    void _udpPortChanged2            ();
    void _udpPortChanged3            ();
    void _rtspUrlChanged            ();
    void _tcpUrlChanged             ();
    void _updateUVC                 ();
    void _setActiveVehicle          (Vehicle* vehicle);
    void _aspectRatioChanged        ();
    void _restartVideo              ();

private:
    void _updateSettings            ();

private:
    bool            _isTaisync          = false;
    //视频接受类
    VideoReceiver*  _videoReceiver      = nullptr;
    //视频设置接口
    VideoSettings*  _videoSettings      = nullptr;
    //私有字符串变量 可以通过函数接口查询
    QString         _videoSourceID;
    //是否为全屏
    bool            _fullScreen         = false;
    //设置启动的机器
    Vehicle*        _activeVehicle      = nullptr;
};

#endif
