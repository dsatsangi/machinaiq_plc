#pragma once
#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include "AppState.h"

// Central read/write state hub. All writes go through setters (thread-safe).
// UI connects to signals emitted on the main thread via Qt::QueuedConnection.
class StateManager : public QObject {
    Q_OBJECT
public:
    static StateManager& instance();

    AppState snapshot() const;

    void updateCycleData(int current, int total, int timeMs);
    void updateCounters(int total, int pass, int reject);
    void updateActiveProgram(const QString& program);
    void updateChannel(int ch);
    void updateScrewdriver(const QString& id);
    void updatePattern(const QString& id);
    void setReverseEnabled(bool enabled);
    void setDriveMode(DriveMode mode);
    void setSystemStatus(SystemStatus status);
    void setCycleStatus(CycleStatus status);
    void setStationInfo(const QString& id, const QString& name);

signals:
    // All emitted on main thread – UI can connect directly
    void stateChanged(const AppState& state);
    void cycleUpdated(int current, int total, int timeMs);
    void countersUpdated(int total, int pass, int reject);
    void systemStatusChanged(SystemStatus status);
    void cycleStatusChanged(CycleStatus status);
    void channelChanged(int ch);
    void reverseFlagChanged(bool enabled);
    void driveModeChanged(DriveMode mode);
    void activeProgramChanged(const QString& program);

private:
    explicit StateManager(QObject* parent = nullptr);
    Q_DISABLE_COPY_MOVE(StateManager)

    mutable QMutex m_mutex;
    AppState       m_state;
};
