#include "CommManager.h"
#include "state/StateManager.h"
#include <QStringList>
#include <QDebug>

CommManager& CommManager::instance() {
    static CommManager inst;
    return inst;
}

CommManager::CommManager(QObject* parent) : QObject(parent) {}

void CommManager::startSerial(const QString& port, int baud) {
    if (m_serial) return;
    m_serial = new SerialHandler;
    m_serial->configure(port, baud);
    m_serial->moveToThread(&m_serialThread);

    connect(&m_serialThread, &QThread::started, m_serial, &SerialHandler::open);
    connect(m_serial, &SerialHandler::messageReceived,
            this,     &CommManager::onMessageReceived, Qt::QueuedConnection);
    connect(m_serial, &SerialHandler::connectionStateChanged,
            this,     &CommManager::serialConnected,   Qt::QueuedConnection);

    m_serialThread.start();
}

void CommManager::startTcp(const QString& host, quint16 port) {
    if (m_tcp) return;
    m_tcp = new TcpHandler;
    m_tcp->configure(host, port);
    m_tcp->moveToThread(&m_tcpThread);

    connect(&m_tcpThread, &QThread::started, m_tcp, &TcpHandler::connectToHost);
    connect(m_tcp, &TcpHandler::messageReceived,
            this,  &CommManager::onMessageReceived, Qt::QueuedConnection);
    connect(m_tcp, &TcpHandler::connectionStateChanged,
            this,  &CommManager::tcpConnected,      Qt::QueuedConnection);

    m_tcpThread.start();
}

void CommManager::stop() {
    if (m_serial) { m_serialThread.quit(); m_serialThread.wait(); }
    if (m_tcp)    { m_tcpThread.quit();    m_tcpThread.wait();    }
}

void CommManager::sendSetChannel(int ch) {
    sendRaw(QStringLiteral("SET_CHANNEL:%1").arg(ch));
}

void CommManager::sendReset() {
    sendRaw("RESET");
}

void CommManager::sendSetReverse(bool enable) {
    sendRaw(enable ? "REVERSE:ENABLE" : "REVERSE:DISABLE");
}

void CommManager::sendSetMode(bool autoMode) {
    sendRaw(autoMode ? "MODE:AUTO" : "MODE:MANUAL");
}

void CommManager::sendRaw(const QString& cmd) {
    if (m_serial) m_serial->sendCommand(cmd);
    if (m_tcp)    m_tcp->sendCommand(cmd);
}

// ── Protocol parser ───────────────────────────────────────────────────────────
// Frame format: COMMAND:ARG1:ARG2...
// Examples:
//   SCREW:4:4:150      → current/total screw, cycle time ms
//   STATUS:OK          → cycle status
//   STATUS:NOK
//   COUNTER:7:7:0      → total/pass/reject
//   CHANNEL:3          → active channel changed
//   PROGRAM:M5_GROUND  → active program name
//   REVERSE:ON|OFF     → reverse state
//   MODE:AUTO|MANUAL
//   SYS:ONLINE|OFFLINE
void CommManager::onMessageReceived(const QString& msg) {
    parseMessage(msg);
}

void CommManager::parseMessage(const QString& msg) {
    const QStringList parts = msg.split(':');
    if (parts.isEmpty()) return;

    const QString& cmd = parts[0];
    auto& sm = StateManager::instance();

    if (cmd == "SCREW" && parts.size() >= 3) {
        int cur  = parts[1].toInt();
        int tot  = parts[2].toInt();
        int time = (parts.size() >= 4) ? parts[3].toInt() : 0;
        sm.updateCycleData(cur, tot, time);
    }
    else if (cmd == "STATUS" && parts.size() >= 2) {
        if      (parts[1] == "OK")      sm.setCycleStatus(CycleStatus::OK);
        else if (parts[1] == "NOK")     sm.setCycleStatus(CycleStatus::NOK);
        else if (parts[1] == "RUNNING") sm.setCycleStatus(CycleStatus::Running);
        else                            sm.setCycleStatus(CycleStatus::Idle);
    }
    else if (cmd == "COUNTER" && parts.size() >= 4) {
        sm.updateCounters(parts[1].toInt(), parts[2].toInt(), parts[3].toInt());
    }
    else if (cmd == "CHANNEL" && parts.size() >= 2) {
        sm.updateChannel(parts[1].toInt());
    }
    else if (cmd == "PROGRAM" && parts.size() >= 2) {
        sm.updateActiveProgram(parts[1]);
    }
    else if (cmd == "REVERSE" && parts.size() >= 2) {
        sm.setReverseEnabled(parts[1] == "ON");
    }
    else if (cmd == "MODE" && parts.size() >= 2) {
        sm.setDriveMode(parts[1] == "AUTO" ? DriveMode::Auto : DriveMode::Manual);
    }
    else if (cmd == "SYS" && parts.size() >= 2) {
        sm.setSystemStatus(parts[1] == "ONLINE" ? SystemStatus::Online
                                                 : SystemStatus::Offline);
    }
    else {
        qDebug() << "CommManager: unknown message:" << msg;
    }
}
