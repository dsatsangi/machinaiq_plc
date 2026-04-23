#include "HomeScreen.h"
#include "comms/CommManager.h"
#include "state/StateManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStyle>
#include <QLabel>
#include <QPushButton>
#include <QFrame>

HomeScreen::HomeScreen(QWidget* parent) : QWidget(parent) {
    buildUi();
    connectState();
    syncFromState();
}

void HomeScreen::buildUi() {
    setObjectName("homeScreen");
    auto* root = new QVBoxLayout(this);
    root->setSpacing(6);
    root->setContentsMargins(8, 8, 8, 8);

    // ── Header ────────────────────────────────────────────────────────────────
    auto* headerRow = new QHBoxLayout;
    auto* stationCol = new QVBoxLayout;
    m_stationLabel     = new QLabel("STATION 000", this);
    m_stationLabel->setObjectName("stationId");
    m_stationNameLabel = new QLabel("—", this);
    m_stationNameLabel->setObjectName("stationName");
    stationCol->addWidget(m_stationLabel);
    stationCol->addWidget(m_stationNameLabel);

    auto* statusRow = new QHBoxLayout;
    m_sysStatusLabel = new QLabel("SYS OFFLINE", this);
    m_sysStatusLabel->setObjectName("sysStatus");
    m_settingsBtn = new QPushButton("⚙", this);
    m_settingsBtn->setObjectName("iconButton");
    m_settingsBtn->setFixedSize(40, 40);
    connect(m_settingsBtn, &QPushButton::clicked, this, &HomeScreen::settingsRequested);
    statusRow->addWidget(m_sysStatusLabel);
    statusRow->addSpacing(8);
    statusRow->addWidget(m_settingsBtn);

    headerRow->addLayout(stationCol);
    headerRow->addStretch();
    headerRow->addLayout(statusRow);
    root->addLayout(headerRow);

    // ── Status banner ─────────────────────────────────────────────────────────
    m_statusBanner = new StatusBanner(this);
    root->addWidget(m_statusBanner);

    // ── Screw count / time / program ──────────────────────────────────────────
    m_screwCount = new ScrewCountWidget(this);
    root->addWidget(m_screwCount);

    // ── Control toggles ───────────────────────────────────────────────────────
    auto* toggleRow = new QHBoxLayout;
    toggleRow->setSpacing(8);

    // Reverse toggle
    auto* revFrame = new QFrame(this);
    revFrame->setObjectName("toggleCard");
    auto* revLay = new QVBoxLayout(revFrame);
    auto* revTitle = new QLabel("ENABLE REVERSE", revFrame);
    revTitle->setObjectName("toggleTitle");
    m_reverseToggle = new QPushButton("INACTIVE", revFrame);
    m_reverseToggle->setObjectName("toggleButton");
    m_reverseToggle->setCheckable(true);
    connect(m_reverseToggle, &QPushButton::toggled, this, &HomeScreen::onReverseToggled);
    revLay->addWidget(revTitle);
    revLay->addWidget(m_reverseToggle);

    // Auto/Manual toggle
    auto* modeFrame = new QFrame(this);
    modeFrame->setObjectName("toggleCard");
    auto* modeLay = new QHBoxLayout(modeFrame);
    auto* autoLabel = new QLabel("AUTO", modeFrame);
    autoLabel->setObjectName("modeLabel");
    m_modeToggle = new QPushButton("", modeFrame);
    m_modeToggle->setObjectName("toggleButton");
    m_modeToggle->setCheckable(true);
    auto* manLabel = new QLabel("MANUAL", modeFrame);
    manLabel->setObjectName("modeLabel");
    connect(m_modeToggle, &QPushButton::toggled, this, &HomeScreen::onModeToggled);
    modeLay->addWidget(autoLabel);
    modeLay->addWidget(m_modeToggle);
    modeLay->addWidget(manLabel);

    toggleRow->addWidget(revFrame);
    toggleRow->addWidget(modeFrame);
    root->addLayout(toggleRow);

    // ── Channel selector ──────────────────────────────────────────────────────
    m_channelSel = new ChannelSelector(8, this);
    connect(m_channelSel, &ChannelSelector::channelSelected,
            this,         &HomeScreen::onChannelSelected);
    root->addWidget(m_channelSel);

    // ── Stat boxes ────────────────────────────────────────────────────────────
    auto* statsRow = new QHBoxLayout;
    statsRow->setSpacing(8);
    m_totalBox  = new StatBox("Total",  StatBox::White, this);
    m_passBox   = new StatBox("Pass",   StatBox::Green, this);
    m_rejectBox = new StatBox("Reject", StatBox::Red,   this);
    statsRow->addWidget(m_totalBox);
    statsRow->addWidget(m_passBox);
    statsRow->addWidget(m_rejectBox);
    root->addLayout(statsRow);

    // ── Footer ────────────────────────────────────────────────────────────────
    auto* footerRow = new QHBoxLayout;
    auto* brandLabel = new QLabel("MachinaIQ UPLC", this);
    brandLabel->setObjectName("brandLabel");
    m_resetBtn = new QPushButton("⚠  RESET", this);
    m_resetBtn->setObjectName("resetButton");
    m_resetBtn->setMinimumHeight(48);
    connect(m_resetBtn, &QPushButton::clicked, this, &HomeScreen::onResetClicked);
    footerRow->addWidget(brandLabel);
    footerRow->addStretch();
    footerRow->addWidget(m_resetBtn);
    root->addLayout(footerRow);
}

void HomeScreen::connectState() {
    auto& sm = StateManager::instance();
    connect(&sm, &StateManager::cycleUpdated,        this, &HomeScreen::onCycleUpdated);
    connect(&sm, &StateManager::countersUpdated,     this, &HomeScreen::onCountersUpdated);
    connect(&sm, &StateManager::cycleStatusChanged,  this, &HomeScreen::onCycleStatusChanged);
    connect(&sm, &StateManager::systemStatusChanged, this, &HomeScreen::onSystemStatusChanged);
    connect(&sm, &StateManager::channelChanged,      this, &HomeScreen::onChannelChanged);
    connect(&sm, &StateManager::reverseFlagChanged,  this, &HomeScreen::onReverseFlagChanged);
    connect(&sm, &StateManager::driveModeChanged,    this, &HomeScreen::onDriveModeChanged);
    connect(&sm, &StateManager::activeProgramChanged,this, &HomeScreen::onActiveProgramChanged);
}

void HomeScreen::syncFromState() {
    const AppState s = StateManager::instance().snapshot();
    m_stationLabel->setText(QStringLiteral("STATION %1").arg(s.stationId));
    m_stationNameLabel->setText(s.stationName);
    m_screwCount->update(s.currentScrew, s.totalScrews, s.cycleTimeMs);
    m_screwCount->setProgram(s.activeProgram);
    m_statusBanner->setStatus(s.cycleStatus);
    m_totalBox->setValue(s.totalCount);
    m_passBox->setValue(s.passCount);
    m_rejectBox->setValue(s.rejectCount);
    m_channelSel->setActiveChannel(s.activeChannel);
    onReverseFlagChanged(s.reverseEnabled);
    onDriveModeChanged(s.driveMode);
    onSystemStatusChanged(s.sysStatus);
}

// ── State slots ───────────────────────────────────────────────────────────────

void HomeScreen::onCycleUpdated(int current, int total, int timeMs) {
    m_screwCount->update(current, total, timeMs);
}

void HomeScreen::onCountersUpdated(int total, int pass, int reject) {
    m_totalBox->setValue(total);
    m_passBox->setValue(pass);
    m_rejectBox->setValue(reject);
}

void HomeScreen::onCycleStatusChanged(CycleStatus status) {
    m_statusBanner->setStatus(status);
}

void HomeScreen::onSystemStatusChanged(SystemStatus status) {
    bool online = (status == SystemStatus::Online);
    m_sysStatusLabel->setText(online ? "SYS ONLINE" : "SYS OFFLINE");
    m_sysStatusLabel->setProperty("online", online);
    m_sysStatusLabel->style()->unpolish(m_sysStatusLabel);
    m_sysStatusLabel->style()->polish(m_sysStatusLabel);
}

void HomeScreen::onChannelChanged(int ch) {
    m_channelSel->setActiveChannel(ch);
}

void HomeScreen::onReverseFlagChanged(bool enabled) {
    m_reverseToggle->setChecked(enabled);
    m_reverseToggle->setText(enabled ? "ACTIVE" : "INACTIVE");
}

void HomeScreen::onDriveModeChanged(DriveMode mode) {
    m_modeToggle->setChecked(mode == DriveMode::Manual);
}

void HomeScreen::onActiveProgramChanged(const QString& program) {
    m_screwCount->setProgram(program);
}

// ── User action slots ─────────────────────────────────────────────────────────

void HomeScreen::onChannelSelected(int ch) {
    CommManager::instance().sendSetChannel(ch);
}

void HomeScreen::onResetClicked() {
    CommManager::instance().sendReset();
}

void HomeScreen::onReverseToggled(bool on) {
    CommManager::instance().sendSetReverse(on);
}

void HomeScreen::onModeToggled(bool manual) {
    CommManager::instance().sendSetMode(!manual);
}
