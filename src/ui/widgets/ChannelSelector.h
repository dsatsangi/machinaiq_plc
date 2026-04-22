#pragma once
#include <QWidget>
#include <QList>
#include <QPushButton>

// Row of CH1–CH8 touch buttons; highlights the active one.
class ChannelSelector : public QWidget {
    Q_OBJECT
public:
    explicit ChannelSelector(int count = 8, QWidget* parent = nullptr);

    void setActiveChannel(int ch);  // 1-based

signals:
    void channelSelected(int ch);   // 1-based

private:
    QList<QPushButton*> m_buttons;
    int m_active = 1;
};
