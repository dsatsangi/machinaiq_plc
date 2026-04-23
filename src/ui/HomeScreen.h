#pragma once
#include <QWidget>
#include "widgets/StatusBanner.h"
#include "widgets/ScrewCountWidget.h"
#include "widgets/ChannelSelector.h"
#include "widgets/StatBox.h"
#include "state/AppState.h"

class QLabel;
class QPushButton;

class HomeScreen : public QWidget {
    Q_OBJECT
public:
    explicit HomeScreen(QWidget* parent = nullptr);

signals:
    void settingsRequested();

private slots:
    void onCycleUpdated(int current, int total, int timeMs);
    void onCountersUpdated(int total, int pass, int reject);
    void onCycleStatusChanged(CycleStatus status);
    void onSystemStatusChanged(SystemStatus status);
    void onChannelChanged(int ch);
    void onReverseFlagChanged(bool enabled);
    void onDriveModeChanged(DriveMode mode);
    void onActiveProgramChanged(const QString& program);
    void onChannelSelected(int ch);
    void onResetClicked();
    void onReverseToggled(bool on);
    void onModeToggled(bool autoMode);

private:
    void buildUi();
    void connectState();
    void syncFromState();

    // Header
    QLabel*      m_stationLabel;
    QLabel*      m_stationNameLabel;
    QLabel*      m_sysStatusLabel;

    // Widgets
    StatusBanner*    m_statusBanner;
    ScrewCountWidget* m_screwCount;
    ChannelSelector* m_channelSel;

    // Controls
    QPushButton* m_reverseToggle;
    QPushButton* m_modeToggle;

    // Stats
    StatBox* m_totalBox;
    StatBox* m_passBox;
    StatBox* m_rejectBox;

    // Footer
    QPushButton* m_resetBtn;
    QPushButton* m_settingsBtn;
    QLabel*      m_logoLabel;
};
