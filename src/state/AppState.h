#pragma once
#include <QString>

enum class SystemStatus { Online, Offline, Error };
enum class CycleStatus { OK, NOK, Running, Idle };
enum class DriveMode   { Auto, Manual };

struct AppState {
    // Station identity
    QString stationId   = "000";
    QString stationName = "Unknown";

    // Active hierarchy selection
    QString activeScrewdriverId;
    QString activePatternId;
    int     activeChannel = 1;

    // Cycle data
    int currentScrew  = 0;
    int totalScrews   = 0;
    int cycleTimeMs   = 0;
    QString activeProgram;

    // Session counters
    int totalCount  = 0;
    int passCount   = 0;
    int rejectCount = 0;

    // Flags
    bool          reverseEnabled = false;
    DriveMode     driveMode      = DriveMode::Auto;
    SystemStatus  sysStatus      = SystemStatus::Offline;
    CycleStatus   cycleStatus    = CycleStatus::Idle;
};
