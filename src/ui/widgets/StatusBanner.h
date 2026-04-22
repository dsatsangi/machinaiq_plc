#pragma once
#include <QWidget>
#include <QLabel>
#include "state/AppState.h"

class StatusBanner : public QWidget {
    Q_OBJECT
public:
    explicit StatusBanner(QWidget* parent = nullptr);

public slots:
    void setStatus(CycleStatus status);

private:
    QLabel* m_label;
};
