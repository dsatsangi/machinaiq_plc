#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QTimer>

// Runs in its own QThread.
// Maintains a persistent TCP client connection with auto-reconnect.
// Protocol: newline-delimited UTF-8 frames (same as Serial).
class TcpHandler : public QObject {
    Q_OBJECT
public:
    explicit TcpHandler(QObject* parent = nullptr);
    ~TcpHandler() override;

    void configure(const QString& host, quint16 port);
    bool isConnected() const;

public slots:
    void connectToHost();
    void disconnect();
    void sendCommand(const QString& cmd);

signals:
    void messageReceived(const QString& msg);
    void connectionStateChanged(bool connected);
    void errorOccurred(const QString& error);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);
    void tryReconnect();

private:
    QTcpSocket m_socket;
    QTimer     m_reconnectTimer;
    QByteArray m_buffer;
    QString    m_host;
    quint16    m_port = 5000;
};
