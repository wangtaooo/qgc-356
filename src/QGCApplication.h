/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


/**
 * @file
 *   @brief Definition of main class
 *
 *   @author Lorenz Meier <mavteam@student.ethz.ch>
 *
 */

#ifndef QGCAPPLICATION_H
#define QGCAPPLICATION_H

#include <QApplication>
#include <QTimer>
#include <QQmlApplicationEngine>

#include "LinkConfiguration.h"
#include "LinkManager.h"
#include "MAVLinkProtocol.h"
#include "FlightMapSettings.h"
#include "FirmwarePluginManager.h"
#include "MultiVehicleManager.h"
#include "JoystickManager.h"
#include "AudioOutput.h"
#include "UASMessageHandler.h"
#include "FactSystem.h"
#include "GPSRTKFactGroup.h"

#ifdef QGC_RTLAB_ENABLED
#include "OpalLink.h"
#endif

// Work around circular header includes
class QGCSingleton;
class MainWindow;
class QGCToolbox;
class QGCFileDownload;

/**
 * @brief The main application and management class.
 *
 * This class is started by the main method and provides
 * the central management unit of the groundstation application.
 *
 **/
class QGCApplication : public
#ifdef __mobile__
    QGuiApplication // Native Qml based application 基于本地Qml的应用程序
#else
    QApplication    // QtWidget based application 基于QtWidget的应用
#endif
{
    Q_OBJECT

public:
    QGCApplication(int &argc, char* argv[], bool unitTesting);
    ~QGCApplication();

    /// @brief Sets the persistent flag to delete all settings the next time QGroundControl is started.
    void deleteAllSettingsNextBoot(void);

    /// @brief Clears the persistent flag to delete all settings the next time QGroundControl is started.
    void clearDeleteAllSettingsNextBoot(void);

    /// @brief Returns true if unit tests are being run
    ///如果正在运行单元测试，则返回true
    bool runningUnitTests(void) { return _runningUnitTests; }

    /// @brief Returns true if Qt debug output should be logged to a file
    bool logOutput(void) { return _logOutput; }

    /// Used to report a missing Parameter. Warning will be displayed to user. Method may be called
    /// multiple times.
    void reportMissingParameter(int componentId, const QString& name);

    /// Show a non-modal message to the user
    void showMessage(const QString& message);

    /// @return true: Fake ui into showing mobile interface
    bool fakeMobile(void) { return _fakeMobile; }

    // Still working on getting rid of this and using dependency injection instead for everything
    // 我们仍在努力摆脱这些，并使用依赖注入来代替所有的东西
    QGCToolbox* toolbox(void) { return _toolbox; }

    /// Do we have Bluetooth Support?
    bool isBluetoothAvailable() { return _bluetoothAvailable; }

    /// Is Internet available?
    bool isInternetAvailable();

    FactGroup* gpsRtkFactGroup(void)  { return _gpsRtkFactGroup; }

    static QString cachedParameterMetaDataFile(void);
    static QString cachedAirframeMetaDataFile(void);

public slots:
    /// You can connect to this slot to show an information message box from a different thread.
    void informationMessageBoxOnMainThread(const QString& title, const QString& msg);

    /// You can connect to this slot to show a warning message box from a different thread.
    void warningMessageBoxOnMainThread(const QString& title, const QString& msg);

    /// You can connect to this slot to show a critical message box from a different thread.
    void criticalMessageBoxOnMainThread(const QString& title, const QString& msg);

    void showSetupView(void);

    void qmlAttemptWindowClose(void);

    /// Save the specified telemetry Log
    void saveTelemetryLogOnMainThread(QString tempLogfile);

    /// Check that the telemetry save path is set correctly
    void checkTelemetrySavePathOnMainThread(void);

signals:
    /// This is connected to MAVLinkProtocol::checkForLostLogFiles. We signal this to ourselves to call the slot
    /// on the MAVLinkProtocol thread;
    void checkForLostLogFiles(void);

public:
    // Although public, these methods are internal and should only be called by UnitTest code

    /// @brief Perform initialize which is common to both normal application running and unit tests.
    ///         Although public should only be called by main.
    void _initCommon(void);

    /// @brief Initialize the application for normal application boot. Or in other words we are not going to run
    ///         unit tests. Although public should only be called by main.
    /// 初始化应用程序以使应用程序正常启动。换句话说，我们不会运行单元测试。尽管public只能由main调用。
    bool _initForNormalAppBoot(void);

    /// @brief Initialize the application for normal application boot. Or in other words we are not going to run
    ///         unit tests. Although public should only be called by main.
    ///初始化应用程序以使应用程序正常启动。换句话说，我们不会运行单元测试。尽管public只能由main调用。
    bool _initForUnitTests(void);

    void _loadCurrentStyleSheet(void);
    //自己的单例。应该由qgcApp直接引用吗
    static QGCApplication*  _app;   ///< Our own singleton. Should be reference directly by qgcApp

public:
    // Although public, these methods are internal and should only be called by UnitTest code

    /// Shutdown the application object
    void _shutdown(void);

    bool _checkTelemetrySavePath(bool useMessageBox);

private slots:
    void _missingParamsDisplay(void);
    void _currentVersionDownloadFinished(QString remoteFile, QString localFile);
    void _currentVersionDownloadError(QString errorMsg);
    bool _parseVersionText(const QString& versionString, int& majorVersion, int& minorVersion, int& buildVersion);
    void _onGPSConnect();
    void _onGPSDisconnect();
    void _gpsSurveyInStatus(float duration, float accuracyMM,  double latitude, double longitude, float altitude, bool valid, bool active);
    void _gpsNumSatellites(int numSatellites);

private:
    QObject* _rootQmlObject(void);
    void _checkForNewVersion(void);

#ifdef __mobile__
    QQmlApplicationEngine* _qmlAppEngine;
#endif

    bool _runningUnitTests; ///< true: running unit tests, false: normal app
    bool _logOutput;        ///< true: Log Qt debug output to file

    static const char*  _darkStyleFile;
    static const char*  _lightStyleFile;
    static const int    _missingParamsDelayedDisplayTimerTimeout = 1000;    ///< Timeout to wait for next missing fact to come in before display
    QTimer              _missingParamsDelayedDisplayTimer;                  ///< Timer use to delay missing fact display
    QStringList         _missingParams;                                     ///< List of missing facts to be displayed
    bool				_fakeMobile;                                        ///< true: Fake ui into displaying mobile interface
    bool                _settingsUpgraded;                                  ///< true: Settings format has been upgrade to new version
    int                 _majorVersion;
    int                 _minorVersion;
    int                 _buildVersion;
    QGCFileDownload*    _currentVersionDownload;
    GPSRTKFactGroup*    _gpsRtkFactGroup;

    QGCToolbox* _toolbox;//所有的工具集

    QTranslator _QGCTranslator;//翻译

    bool _bluetoothAvailable;
    //设置键，保持设置版本
    static const char* _settingsVersionKey;             ///< Settings key which hold settings version
    //如果在启动时设置此设置键，所有设置将被删除
    static const char* _deleteAllSettingsKey;           ///< If this settings key is set on boot, all settings will be deleted

    /// Unit Test have access to creating and destroying singletons
    friend class UnitTest;

};

/// @brief Returns the QGCApplication object singleton.
QGCApplication* qgcApp(void);

#endif
