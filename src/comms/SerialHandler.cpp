#include "SerialHandler.h"
#include <QDebug>
#include <QMetaObject>
#include <QThread>

SerialHandler::SerialHandler(QObject* parent) : QObject(parent) {
    connect(&m_port, &QSerialPort::readyRead,   this, &SerialHandler::onReadyRead);
    connect(&m_port, &QSerialPort::errorOccurred, this, &SerialHandler::onError);
}

SerialHandler::~SerialHandler() {
    if (m_port.isOpen()) m_port.close();
}

void SerialHandler::configure(const QString& port, int baud) {
    m_portName = port;
    m_baud     = baud;
}

bool SerialHandler::isOpen() const { return m_port.isOpen(); }

void SerialHandler::open() {
    if (m_port.isOpen()) return;
    m_port.setPortName(m_portName);
    m_port.setBaudRate(m_baud);
    m_port.setDataBits(QSerialPort::Data8);
    m_port.setParity(QSerialPort::NoParity);
    m_port.setStopBits(QSerialPort::OneStop);
    m_port.setFlowControl(QSerialPort::NoFlowControl);
    if (!m_port.open(QIODevice::ReadWrite)) {
        emit errorOccurred(m_port.errorString());
        emit connectionStateChanged(false);
    } else {
        emit connectionStateChanged(true);
    }
}

void SerialHandler::close() {
    if (m_port.isOpen()) {
        m_port.close();
        emit connectionStateChanged(false);
    }
}

void SerialHandler::sendCommand(const QString& cmd) {
    // Allow cross-thread calls
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(this, [this, cmd]{ sendCommand(cmd); },
                                  Qt::QueuedConnection);
        return;
    }
    if (!m_port.isOpen()) return;
    m_port.write((cmd + "\n").toUtf8());
}

void SerialHandler::onReadyRead() {
    m_buffer.append(m_port.readAll());
    while (true) {
        int idx = m_buffer.indexOf('\n');
        if (idx < 0) break;
        QString line = QString::fromUtf8(m_buffer.left(idx)).trimmed();
        m_buffer.remove(0, idx + 1);
        if (!line.isEmpty())
            emit messageReceived(line);
    }
}

void SerialHandler::onError(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::NoError) return;
    emit errorOccurred(m_port.errorString());
    if (error == QSerialPort::ResourceError)
        emit connectionStateChanged(false);
}
