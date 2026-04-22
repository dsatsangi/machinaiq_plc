#pragma once
#include <QObject>
#include <QSerialPort>

// Runs in its own QThread. Owns the QSerialPort.
// Emits messageReceived() for each complete '\n'-terminated frame.
// Call sendCommand() from any thread (thread-safe via invokeMethod).
class SerialHandler : public QObject {
    Q_OBJECT
public:
    explicit SerialHandler(QObject* parent = nullptr);
    ~SerialHandler() override;

    void configure(const QString& port, int baud);
    bool isOpen() const;

public slots:
    void open();
    void close();
    void sendCommand(const QString& cmd);

signals:
    void messageReceived(const QString& msg);
    void connectionStateChanged(bool connected);
    void errorOccurred(const QString& error);

private slots:
    void onReadyRead();
    void onError(QSerialPort::SerialPortError error);

private:
    QSerialPort m_port;
    QByteArray  m_buffer;
    QString     m_portName;
    int         m_baud = 115200;
};
