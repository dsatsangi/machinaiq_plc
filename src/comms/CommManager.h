#pragma once
#include <QObject>
#include <QThread>
#include "SerialHandler.h"
#include "TcpHandler.h"

// Owns the Serial and TCP worker threads.
// Parses inbound messages and routes them to StateManager.
// Exposes sendCommand() for outbound messages (broadcasts to both transports).
class CommManager : public QObject {
    Q_OBJECT
public:
    static CommManager& instance();

    void startSerial(const QString& port, int baud = 115200);
    void startTcp(const QString& host, quint16 port = 5000);
    void stop();

public slots:
    // Outbound helpers – can be called from UI thread
    void sendSetChannel(int ch);
    void sendReset();
    void sendSetReverse(bool enable);
    void sendSetMode(bool autoMode);
    void sendRaw(const QString& cmd);

signals:
    void serialConnected(bool connected);
    void tcpConnected(bool connected);

private slots:
    void onMessageReceived(const QString& msg);

private:
    explicit CommManager(QObject* parent = nullptr);
    Q_DISABLE_COPY_MOVE(CommManager)

    void parseMessage(const QString& msg);

    SerialHandler* m_serial = nullptr;
    TcpHandler*    m_tcp    = nullptr;
    QThread        m_serialThread;
    QThread        m_tcpThread;
};
