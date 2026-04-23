#include "MainWindow.h"
#include <QApplication>
#include <QFile>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("MachinaIQ UPLC");
#ifdef Q_OS_LINUX
    showFullScreen();
#else
    resize(480, 800);  // portrait, matches Pi 7" display
    show();
#endif

    m_stack    = new QStackedWidget(this);
    m_home     = new HomeScreen(m_stack);
    m_settings = new SettingsScreen(m_stack);

    m_stack->addWidget(m_home);     // index 0
    m_stack->addWidget(m_settings); // index 1

    setCentralWidget(m_stack);

    connect(m_home,     &HomeScreen::settingsRequested,
            this,       [this]{ m_stack->setCurrentIndex(1); });
    connect(m_settings, &SettingsScreen::homeRequested,
            this,       [this]{ m_stack->setCurrentIndex(0); });

    // Load stylesheet
    QFile qss(":/style.qss");
    if (qss.open(QIODevice::ReadOnly))
        qApp->setStyleSheet(qss.readAll());
}
