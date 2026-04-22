#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include "HomeScreen.h"
#include "SettingsScreen.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    QStackedWidget* m_stack;
    HomeScreen*     m_home;
    SettingsScreen* m_settings;
};
