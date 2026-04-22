#pragma once
#include <QWidget>
#include <QLabel>

class ScrewCountWidget : public QWidget {
    Q_OBJECT
public:
    explicit ScrewCountWidget(QWidget* parent = nullptr);

public slots:
    void update(int current, int total, int timeMs);
    void setProgram(const QString& program);

private:
    QLabel* m_countLabel;
    QLabel* m_totalLabel;
    QLabel* m_timeLabel;
    QLabel* m_programLabel;
};
