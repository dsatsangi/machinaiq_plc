#pragma once
#include <QApplication>

// Wires up config, state, and comms before the UI is shown.
class Application : public QApplication {
    Q_OBJECT
public:
    Application(int& argc, char** argv);
    int run();

private:
    void loadConfig();
    void startComms();
};
