#include "TcpHandler.h"
#include <QDebug>
#include <QMetaObject>

static constexpr int kReconnectMs = 3000;

TcpHandler::TcpHandler(QObject* parent) : QObject(parent) {
    connect(&m_socket, &QTcpSocket::connected,    this, &TcpHandler::onConnected);
    connect(&m_socket, &QTcpSocket::disconnected, this, &TcpHandler::onDisconnected);
    connect(&m_socket, &QTcpSocket::readyRead,    this, &TcpHandler::onReadyRead);
    connect(&m_socket, &QAbstractSocket::errorOccurred, this, &TcpHandler::onError);

    m_reconnectTimer.setSingleShot(true);
    m_reconnectTimer.setInterval(kReconnectMs);
    connect(&m_reconnectTimer, &QTimer::timeout, this, &TcpHandler::tryReconnect);
}

TcpHandler::~TcpHandler() {
    m_reconnectTimer.stop();
    m_socket.disconnectFromHost();
}

void TcpHandler::configure(const QString& host, quint16 port) {
    m_host = host;
    m_port = port;
}

bool TcpHandler::isConnected() const {
    return m_socket.state() == QAbstractSocket::ConnectedState;
}

void TcpHandler::connectToHost() {
    if (isConnected() || m_host.isEmpty()) return;
    m_socket.connectToHost(m_host, m_port);
}

void TcpHandler::disconnect() {
    m_reconnectTimer.stop();
    m_socket.disconnectFromHost();
}

void TcpHandler::sendCommand(const QString& cmd) {
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(this, [this, cmd]{ sendCommand(cmd); },
                                  Qt::QueuedConnection);
        return;
    }
    if (!isConnected()) return;
    m_socket.write((cmd + "\n").toUtf8());
}

void TcpHandler::onConnected() {
    m_reconnectTimer.stop();
    emit connectionStateChanged(true);
}

void TcpHandler::onDisconnected() {
    emit connectionStateChanged(false);
    m_reconnectTimer.start();
}

void TcpHandler::onReadyRead() {
    m_buffer.append(m_socket.readAll());
    while (true) {
        int idx = m_buffer.indexOf('\n');
        if (idx < 0) break;
        QString line = QString::fromUtf8(m_buffer.left(idx)).trimmed();
        m_buffer.remove(0, idx + 1);
        if (!line.isEmpty())
            emit messageReceived(line);
    }
}

void TcpHandler::onError(QAbstractSocket::SocketError) {
    emit errorOccurred(m_socket.errorString());
}

void TcpHandler::tryReconnect() {
    qDebug() << "TcpHandler: reconnecting to" << m_host << m_port;
    connectToHost();
}
