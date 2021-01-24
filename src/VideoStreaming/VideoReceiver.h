/****************************************************************************
 *
 *   (c) 2009-2018 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


/**
 * @file
 *   @brief QGC Video Receiver
 *   @author Gus Grubba <mavlink@grubba.com>
 */

#pragma once

#include "QGCLoggingCategory.h"
#include <QObject>
#include <QTimer>
#include <QTcpSocket>

#include "VideoSurface.h"

#if defined(QGC_GST_STREAMING)
#include <gst/gst.h>
#endif

Q_DECLARE_LOGGING_CATEGORY(VideoReceiverLog)
//视频设置类
class VideoSettings;
//视频接受类
class VideoReceiver : public QObject
{
    Q_OBJECT
public:
#if defined(QGC_GST_STREAMING)
    Q_PROPERTY(bool             recording           READ    recording           NOTIFY recordingChanged)
#endif
    Q_PROPERTY(VideoSurface*    videoSurface        READ    videoSurface        CONSTANT)
    Q_PROPERTY(bool             videoRunning        READ    videoRunning        NOTIFY  videoRunningChanged)
    Q_PROPERTY(QString          imageFile           READ    imageFile           NOTIFY  imageFileChanged)
    Q_PROPERTY(QString          videoFile           READ    videoFile           NOTIFY  videoFileChanged)
    Q_PROPERTY(bool             showFullScreen      READ    showFullScreen      WRITE   setShowFullScreen     NOTIFY showFullScreenChanged)

    explicit VideoReceiver(QObject* parent = nullptr);
    ~VideoReceiver();

#if defined(QGC_GST_STREAMING)
    virtual bool            running         () { return _running;   }
    virtual bool            recording       () { return _recording; }
    virtual bool            streaming       () { return _streaming; }
    virtual bool            starting        () { return _starting;  }
    virtual bool            stopping        () { return _stopping;  }
#endif

    virtual VideoSurface*   videoSurface    () { return _videoSurface; }
    virtual bool            videoRunning    () { return _videoRunning; }
    virtual QString         imageFile       () { return _imageFile; }//
    virtual QString         videoFile       () { return _videoFile; }
    virtual bool            showFullScreen  () { return _showFullScreen; }

    virtual void            grabImage       (QString imageFile);//捉取图像

    virtual void        setShowFullScreen   (bool show) { _showFullScreen = show; emit showFullScreenChanged(); }

signals:
    void videoRunningChanged                ();
    void imageFileChanged                   ();
    void videoFileChanged                   ();
    void showFullScreenChanged              ();
#if defined(QGC_GST_STREAMING)
    void recordingChanged                   ();
    void msgErrorReceived                   ();
    void msgEOSReceived                     ();//signal when stop
    void msgStateChangedReceived            ();
#endif

public slots:
    virtual void start                      ();
    virtual void stop                       ();
    virtual void setUri                     (const QString& uri);
    virtual void stopRecording              ();
    virtual void startRecording             (const QString& videoFile = QString());

protected slots:
    virtual void _updateTimer               ();
#if defined(QGC_GST_STREAMING)
    virtual void _timeout                   ();
    virtual void _connected                 ();
    virtual void _socketError               (QAbstractSocket::SocketError socketError);
    virtual void _handleError               ();
    //处理视频流结束的接口
    virtual void _handleEOS                 ();
    virtual void _handleStateChanged        ();//处理状态变化的函数
#endif

protected:
#if defined(QGC_GST_STREAMING)

    typedef struct
    {
        GstPad*         teepad;
        GstElement*     queue;
        GstElement*     mux;
        GstElement*     filesink;
        GstElement*     parse;
        gboolean        removing;
    } Sink;//用于文件存储的Sink

    bool                _running;//记录PipeLine的状态
    bool                _recording;
    bool                _streaming;
    bool                _starting;//启动中标志位
    bool                _stopping;
    bool                _stop;
    Sink*               _sink;
    GstElement*         _tee;

    static gboolean             _onBusMessage           (GstBus* bus, GstMessage* message, gpointer user_data);//
    static GstPadProbeReturn    _unlinkCallBack         (GstPad* pad, GstPadProbeInfo* info, gpointer user_data);
    static GstPadProbeReturn    _keyframeWatch          (GstPad* pad, GstPadProbeInfo* info, gpointer user_data);

    virtual void                _detachRecordingBranch  (GstPadProbeInfo* info);
    virtual void                _shutdownRecordingBranch();
    virtual void                _shutdownPipeline       ();
    virtual void                _cleanupOldVideos       ();
    virtual void                _setVideoSink           (GstElement* sink);

    GstElement*     _pipeline;//传输用到的PIPELINE
    GstElement*     _pipelineStopRec;//Gst 成员 我们要创建的
    GstElement*     _videoSink;

    //-- Wait for Video Server to show up before starting
    QTimer          _frameTimer;
    QTimer          _timer;//定时器
    QTcpSocket*     _socket;//连接服务器
    bool            _serverPresent;//判断是否连接到服务器
    int             _rtspTestInterval_ms;//rtsp测试时间间隔

    //-- RTSP UDP reconnect timeout
    uint64_t        _udpReconnect_us;

#endif

    QString         _uri;//会被转化QUri
    QString         _imageFile;
    QString         _videoFile;
    VideoSurface*   _videoSurface;
    bool            _videoRunning;
    bool            _showFullScreen;
    //视频设置类
    VideoSettings*  _videoSettings;
};

