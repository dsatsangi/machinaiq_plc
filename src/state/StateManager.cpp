#include "StateManager.h"
#include <QMetaObject>
#include <QCoreApplication>

StateManager& StateManager::instance() {
    static StateManager inst;
    return inst;
}

StateManager::StateManager(QObject* parent) : QObject(parent) {}

AppState StateManager::snapshot() const {
    QMutexLocker lk(&m_mutex);
    return m_state;
}

// Helper: emit on main thread so UI slots need no special queued connection
#define EMIT_MAIN(signal, ...) \
    QMetaObject::invokeMethod(this, [this]{ emit signal(__VA_ARGS__); }, Qt::QueuedConnection)

void StateManager::updateCycleData(int current, int total, int timeMs) {
    { QMutexLocker lk(&m_mutex);
      m_state.currentScrew = current;
      m_state.totalScrews  = total;
      m_state.cycleTimeMs  = timeMs; }
    EMIT_MAIN(cycleUpdated, current, total, timeMs);
}

void StateManager::updateCounters(int total, int pass, int reject) {
    { QMutexLocker lk(&m_mutex);
      m_state.totalCount  = total;
      m_state.passCount   = pass;
      m_state.rejectCount = reject; }
    EMIT_MAIN(countersUpdated, total, pass, reject);
}

void StateManager::updateActiveProgram(const QString& program) {
    { QMutexLocker lk(&m_mutex); m_state.activeProgram = program; }
    EMIT_MAIN(activeProgramChanged, program);
}

void StateManager::updateChannel(int ch) {
    { QMutexLocker lk(&m_mutex); m_state.activeChannel = ch; }
    EMIT_MAIN(channelChanged, ch);
}

void StateManager::updateScrewdriver(const QString& id) {
    QMutexLocker lk(&m_mutex);
    m_state.activeScrewdriverId = id;
}

void StateManager::updatePattern(const QString& id) {
    QMutexLocker lk(&m_mutex);
    m_state.activePatternId = id;
}

void StateManager::setReverseEnabled(bool enabled) {
    { QMutexLocker lk(&m_mutex); m_state.reverseEnabled = enabled; }
    EMIT_MAIN(reverseFlagChanged, enabled);
}

void StateManager::setDriveMode(DriveMode mode) {
    { QMutexLocker lk(&m_mutex); m_state.driveMode = mode; }
    EMIT_MAIN(driveModeChanged, mode);
}

void StateManager::setSystemStatus(SystemStatus status) {
    { QMutexLocker lk(&m_mutex); m_state.sysStatus = status; }
    EMIT_MAIN(systemStatusChanged, status);
}

void StateManager::setCycleStatus(CycleStatus status) {
    { QMutexLocker lk(&m_mutex); m_state.cycleStatus = status; }
    EMIT_MAIN(cycleStatusChanged, status);
}

void StateManager::setStationInfo(const QString& id, const QString& name) {
    QMutexLocker lk(&m_mutex);
    m_state.stationId   = id;
    m_state.stationName = name;
}
