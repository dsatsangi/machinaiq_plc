#pragma once
#include <QFrame>
#include <QLabel>

// Reusable stat tile: label above, big coloured number below.
class StatBox : public QFrame {
    Q_OBJECT
public:
    enum Color { White, Green, Red };
    explicit StatBox(const QString& label, Color color = White, QWidget* parent = nullptr);
    void setValue(int v);

private:
    QLabel* m_value;
};
