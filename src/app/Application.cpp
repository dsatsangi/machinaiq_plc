#include "Application.h"
#include "config/ConfigManager.h"
#include "state/StateManager.h"
#include "comms/CommManager.h"
#include "ui/MainWindow.h"

#include <QCommandLineParser>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

Application::Application(int& argc, char** argv) : QApplication(argc, argv) {
    setApplicationName("machinaiq_uplc");
    setApplicationVersion("1.0.0");
    setOrganizationName("MachinaIQ");
}

int Application::run() {
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption configOpt({"c", "config"}, "Path to config.json", "path");
    QCommandLineOption portOpt("serial-port", "Serial port device", "device", "/dev/ttyUSB0");
    QCommandLineOption baudOpt("baud", "Serial baud rate", "rate", "115200");
    QCommandLineOption tcpHostOpt("tcp-host", "TCP host", "host", "");
    QCommandLineOption tcpPortOpt("tcp-port", "TCP port", "port", "5000");
    parser.addOption(configOpt);
    parser.addOption(portOpt);
    parser.addOption(baudOpt);
    parser.addOption(tcpHostOpt);
    parser.addOption(tcpPortOpt);
    parser.process(*this);

    // Config path: CLI arg → /etc (Linux/Pi) → app data dir (dev/Mac)
    QString cfgPath = parser.value(configOpt);
    if (cfgPath.isEmpty()) {
#ifdef Q_OS_LINUX
        cfgPath = "/etc/machinaiq_uplc/config.json";
#else
        cfgPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
                  + "/config.json";
#endif
    }

    loadConfig();
    ConfigManager::instance().load(cfgPath);

    // Sync station info into state
    const Station s = ConfigManager::instance().station();
    StateManager::instance().setStationInfo(s.id, s.name);

    // Start comms
    const QString serialPort = parser.value(portOpt);
    const int     baud       = parser.value(baudOpt).toInt();
    const QString tcpHost    = parser.value(tcpHostOpt);
    const quint16 tcpPort    = static_cast<quint16>(parser.value(tcpPortOpt).toUInt());

    CommManager::instance().startSerial(serialPort, baud);
    if (!tcpHost.isEmpty())
        CommManager::instance().startTcp(tcpHost, tcpPort);

    MainWindow w;
    w.show();

    int ret = exec();
    CommManager::instance().stop();
    return ret;
}

void Application::loadConfig() {
#ifdef Q_OS_LINUX
    QDir("/etc/machinaiq_uplc").mkpath(".");
#else
    QDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)).mkpath(".");
#endif
}

void Application::startComms() {}
