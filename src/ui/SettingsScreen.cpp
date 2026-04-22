#include "SettingsScreen.h"
#include "config/ConfigManager.h"
#include "state/StateManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QStackedWidget>
#include <QTreeWidgetItem>
#include <QMessageBox>

// Tree item user data roles
static constexpr int kTypeRole = Qt::UserRole;
static constexpr int kIdRole   = Qt::UserRole + 1;
static constexpr int kPIdRole  = Qt::UserRole + 2;  // parent id
static constexpr int kGPIdRole = Qt::UserRole + 3;  // grandparent id

enum NodeType { NodeStation, NodeScrewdriver, NodePattern, NodeChannel };

SettingsScreen::SettingsScreen(QWidget* parent) : QWidget(parent) {
    buildUi();
    connect(&ConfigManager::instance(), &ConfigManager::configReloaded,
            this, &SettingsScreen::onConfigReloaded);
    populateTree(ConfigManager::instance().station());
}

void SettingsScreen::buildUi() {
    setObjectName("settingsScreen");
    auto* root = new QVBoxLayout(this);
    root->setSpacing(8);
    root->setContentsMargins(8, 8, 8, 8);

    // Header
    auto* hdr = new QHBoxLayout;
    auto* backBtn = new QPushButton("← Back", this);
    backBtn->setObjectName("secondaryButton");
    connect(backBtn, &QPushButton::clicked, this, &SettingsScreen::homeRequested);
    auto* title = new QLabel("CONFIGURATION", this);
    title->setObjectName("screenTitle");
    hdr->addWidget(backBtn);
    hdr->addStretch();
    hdr->addWidget(title);
    root->addLayout(hdr);

    // Toolbar
    auto* toolbar = new QHBoxLayout;
    auto* addSdBtn  = new QPushButton("+ Screwdriver", this);
    auto* addPatBtn = new QPushButton("+ Pattern", this);
    auto* addChBtn  = new QPushButton("+ Channel", this);
    auto* delBtn    = new QPushButton("Delete", this);
    delBtn->setObjectName("dangerButton");
    for (auto* b : {addSdBtn, addPatBtn, addChBtn, delBtn})
        b->setMinimumHeight(40);
    connect(addSdBtn,  &QPushButton::clicked, this, &SettingsScreen::onAddScrewdriver);
    connect(addPatBtn, &QPushButton::clicked, this, &SettingsScreen::onAddPattern);
    connect(addChBtn,  &QPushButton::clicked, this, &SettingsScreen::onAddChannel);
    connect(delBtn,    &QPushButton::clicked, this, &SettingsScreen::onDeleteSelected);
    toolbar->addWidget(addSdBtn);
    toolbar->addWidget(addPatBtn);
    toolbar->addWidget(addChBtn);
    toolbar->addStretch();
    toolbar->addWidget(delBtn);
    root->addLayout(toolbar);

    // Splitter: tree | detail
    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(2);

    m_tree = new QTreeWidget(splitter);
    m_tree->setObjectName("configTree");
    m_tree->setHeaderHidden(true);
    m_tree->setIndentation(16);
    m_tree->setMinimumWidth(160);
    connect(m_tree, &QTreeWidget::itemSelectionChanged,
            this,   &SettingsScreen::onTreeSelectionChanged);

    // ── Detail stack ─────────────────────────────────────────────────────────
    m_detailStack = new QStackedWidget(splitter);
    m_detailStack->addWidget(new QLabel("Select an item", m_detailStack)); // 0 = empty

    // Station page (index 1)
    auto* stPage  = new QWidget(m_detailStack);
    auto* stForm  = new QFormLayout(stPage);
    m_stId   = new QLineEdit(stPage); m_stId->setPlaceholderText("e.g. 082");
    m_stName = new QLineEdit(stPage); m_stName->setPlaceholderText("e.g. Precision Assembly");
    stForm->addRow("Station ID:", m_stId);
    stForm->addRow("Name:", m_stName);
    auto* stSave = new QPushButton("Save", stPage);
    connect(stSave, &QPushButton::clicked, this, &SettingsScreen::onSaveDetail);
    stForm->addRow(stSave);
    m_detailStack->addWidget(stPage); // 1

    // Screwdriver page (index 2)
    auto* sdPage  = new QWidget(m_detailStack);
    auto* sdForm  = new QFormLayout(sdPage);
    m_sdId   = new QLineEdit(sdPage);
    m_sdName = new QLineEdit(sdPage);
    sdForm->addRow("ID:", m_sdId);
    sdForm->addRow("Name:", m_sdName);
    auto* sdSave = new QPushButton("Save", sdPage);
    connect(sdSave, &QPushButton::clicked, this, &SettingsScreen::onSaveDetail);
    sdForm->addRow(sdSave);
    m_detailStack->addWidget(sdPage); // 2

    // Pattern page (index 3)
    auto* patPage  = new QWidget(m_detailStack);
    auto* patForm  = new QFormLayout(patPage);
    m_patId   = new QLineEdit(patPage);
    m_patName = new QLineEdit(patPage);
    patForm->addRow("ID:", m_patId);
    patForm->addRow("Name:", m_patName);
    auto* patSave = new QPushButton("Save", patPage);
    connect(patSave, &QPushButton::clicked, this, &SettingsScreen::onSaveDetail);
    patForm->addRow(patSave);
    m_detailStack->addWidget(patPage); // 3

    // Channel page (index 4)
    auto* chPage  = new QWidget(m_detailStack);
    auto* chForm  = new QFormLayout(chPage);
    m_chId     = new QSpinBox(chPage); m_chId->setRange(1, 8);
    m_chName   = new QLineEdit(chPage);
    m_chScrews = new QSpinBox(chPage); m_chScrews->setRange(0, 99);
    chForm->addRow("Channel ID:", m_chId);
    chForm->addRow("Name:", m_chName);
    chForm->addRow("Screw Count:", m_chScrews);
    auto* chSave = new QPushButton("Save", chPage);
    connect(chSave, &QPushButton::clicked, this, &SettingsScreen::onSaveDetail);
    chForm->addRow(chSave);
    m_detailStack->addWidget(chPage); // 4

    splitter->addWidget(m_tree);
    splitter->addWidget(m_detailStack);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);

    root->addWidget(splitter, 1);
}

// ── Tree population ───────────────────────────────────────────────────────────

void SettingsScreen::populateTree(const Station& s) {
    m_tree->clear();
    auto* stItem = new QTreeWidgetItem(m_tree, {QStringLiteral("STATION %1").arg(s.id)});
    stItem->setData(0, kTypeRole, NodeStation);
    stItem->setData(0, kIdRole,   s.id);

    for (const auto& sd : s.screwdrivers) {
        auto* sdItem = new QTreeWidgetItem(stItem, {sd.name});
        sdItem->setData(0, kTypeRole, NodeScrewdriver);
        sdItem->setData(0, kIdRole,   sd.id);

        for (const auto& p : sd.patterns) {
            auto* pItem = new QTreeWidgetItem(sdItem, {p.name});
            pItem->setData(0, kTypeRole, NodePattern);
            pItem->setData(0, kIdRole,   p.id);
            pItem->setData(0, kPIdRole,  sd.id);

            for (const auto& ch : p.channels) {
                auto* chItem = new QTreeWidgetItem(pItem,
                    {QStringLiteral("CH%1 – %2 screws").arg(ch.id).arg(ch.screwCount)});
                chItem->setData(0, kTypeRole, NodeChannel);
                chItem->setData(0, kIdRole,   ch.id);
                chItem->setData(0, kPIdRole,  p.id);
                chItem->setData(0, kGPIdRole, sd.id);
            }
        }
    }
    m_tree->expandAll();
}

void SettingsScreen::onTreeSelectionChanged() {
    auto items = m_tree->selectedItems();
    if (items.isEmpty()) { clearDetail(); return; }

    auto* item  = items.first();
    int type    = item->data(0, kTypeRole).toInt();
    QString id  = item->data(0, kIdRole).toString();

    const Station s = ConfigManager::instance().station();

    switch (type) {
        case NodeStation:
            showStationDetail(s);
            break;
        case NodeScrewdriver:
            showScrewdriverDetail(ConfigManager::instance().screwdriverById(id));
            break;
        case NodePattern: {
            QString sdId = item->data(0, kPIdRole).toString();
            showPatternDetail(ConfigManager::instance().patternById(sdId, id));
            break;
        }
        case NodeChannel: {
            QString patId = item->data(0, kPIdRole).toString();
            QString sdId  = item->data(0, kGPIdRole).toString();
            showChannelDetail(ConfigManager::instance().channelById(sdId, patId, id.toInt()),
                              sdId, patId);
            break;
        }
    }
}

void SettingsScreen::showStationDetail(const Station& s) {
    m_editSdId.clear(); m_editPatId.clear(); m_editChId = -1;
    m_stId->setText(s.id);
    m_stName->setText(s.name);
    m_detailStack->setCurrentIndex(1);
}

void SettingsScreen::showScrewdriverDetail(const Screwdriver& sd) {
    m_editSdId = sd.id; m_editPatId.clear(); m_editChId = -1;
    m_sdId->setText(sd.id);
    m_sdName->setText(sd.name);
    m_detailStack->setCurrentIndex(2);
}

void SettingsScreen::showPatternDetail(const Pattern& p) {
    m_editPatId = p.id; m_editChId = -1;
    m_patId->setText(p.id);
    m_patName->setText(p.name);
    m_detailStack->setCurrentIndex(3);
}

void SettingsScreen::showChannelDetail(const Channel& ch, const QString& sdId, const QString& patId) {
    m_editSdId = sdId; m_editPatId = patId; m_editChId = ch.id;
    m_chId->setValue(ch.id);
    m_chName->setText(ch.name);
    m_chScrews->setValue(ch.screwCount);
    m_detailStack->setCurrentIndex(4);
}

void SettingsScreen::clearDetail() {
    m_detailStack->setCurrentIndex(0);
    m_editSdId.clear(); m_editPatId.clear(); m_editChId = -1;
}

// ── CRUD ──────────────────────────────────────────────────────────────────────

Station SettingsScreen::currentStationCopy() {
    return ConfigManager::instance().station();
}

void SettingsScreen::onAddScrewdriver() {
    Station s = currentStationCopy();
    if (s.screwdrivers.size() >= 5) {
        QMessageBox::warning(this, "Limit", "Max 5 screwdrivers.");
        return;
    }
    Screwdriver sd;
    sd.id   = QStringLiteral("SD%1").arg(s.screwdrivers.size() + 1);
    sd.name = QStringLiteral("Screwdriver %1").arg(s.screwdrivers.size() + 1);
    s.screwdrivers.append(sd);
    ConfigManager::instance().setStation(s);
}

void SettingsScreen::onAddPattern() {
    auto items = m_tree->selectedItems();
    QString sdId;
    if (!items.isEmpty()) {
        auto* item = items.first();
        int type = item->data(0, kTypeRole).toInt();
        if      (type == NodeScrewdriver) sdId = item->data(0, kIdRole).toString();
        else if (type == NodePattern)     sdId = item->data(0, kPIdRole).toString();
        else if (type == NodeChannel)     sdId = item->data(0, kGPIdRole).toString();
    }
    if (sdId.isEmpty()) { QMessageBox::information(this, "Info", "Select a Screwdriver first."); return; }

    Station s = currentStationCopy();
    for (auto& sd : s.screwdrivers) {
        if (sd.id != sdId) continue;
        if (sd.patterns.size() >= 5) { QMessageBox::warning(this, "Limit", "Max 5 patterns."); return; }
        Pattern p;
        p.id   = QStringLiteral("PAT%1").arg(sd.patterns.size() + 1);
        p.name = QStringLiteral("Pattern %1").arg(sd.patterns.size() + 1);
        sd.patterns.append(p);
        break;
    }
    ConfigManager::instance().setStation(s);
}

void SettingsScreen::onAddChannel() {
    auto items = m_tree->selectedItems();
    QString sdId, patId;
    if (!items.isEmpty()) {
        auto* item = items.first();
        int type = item->data(0, kTypeRole).toInt();
        if      (type == NodePattern) { sdId = item->data(0, kPIdRole).toString(); patId = item->data(0, kIdRole).toString(); }
        else if (type == NodeChannel) { sdId = item->data(0, kGPIdRole).toString(); patId = item->data(0, kPIdRole).toString(); }
    }
    if (sdId.isEmpty() || patId.isEmpty()) { QMessageBox::information(this, "Info", "Select a Pattern first."); return; }

    Station s = currentStationCopy();
    for (auto& sd : s.screwdrivers) {
        if (sd.id != sdId) continue;
        for (auto& p : sd.patterns) {
            if (p.id != patId) continue;
            if (p.channels.size() >= 8) { QMessageBox::warning(this, "Limit", "Max 8 channels."); return; }
            Channel ch;
            ch.id = p.channels.size() + 1;
            ch.name = QStringLiteral("Channel %1").arg(ch.id);
            p.channels.append(ch);
        }
    }
    ConfigManager::instance().setStation(s);
}

void SettingsScreen::onDeleteSelected() {
    auto items = m_tree->selectedItems();
    if (items.isEmpty()) return;
    auto* item = items.first();
    int    type = item->data(0, kTypeRole).toInt();
    QString id  = item->data(0, kIdRole).toString();

    if (type == NodeStation) return; // can't delete station root

    Station s = currentStationCopy();

    if (type == NodeScrewdriver) {
        s.screwdrivers.removeIf([&](const Screwdriver& sd){ return sd.id == id; });
    } else if (type == NodePattern) {
        QString sdId = item->data(0, kPIdRole).toString();
        for (auto& sd : s.screwdrivers)
            if (sd.id == sdId)
                sd.patterns.removeIf([&](const Pattern& p){ return p.id == id; });
    } else if (type == NodeChannel) {
        QString patId = item->data(0, kPIdRole).toString();
        QString sdId  = item->data(0, kGPIdRole).toString();
        int chId = id.toInt();
        for (auto& sd : s.screwdrivers)
            if (sd.id == sdId)
                for (auto& p : sd.patterns)
                    if (p.id == patId)
                        p.channels.removeIf([&](const Channel& ch){ return ch.id == chId; });
    }
    ConfigManager::instance().setStation(s);
    clearDetail();
}

void SettingsScreen::onSaveDetail() {
    int page = m_detailStack->currentIndex();
    Station s = currentStationCopy();

    if (page == 1) {
        s.id   = m_stId->text().trimmed();
        s.name = m_stName->text().trimmed();
        StateManager::instance().setStationInfo(s.id, s.name);
    } else if (page == 2) {
        for (auto& sd : s.screwdrivers) {
            if (sd.id != m_editSdId) continue;
            sd.id   = m_sdId->text().trimmed();
            sd.name = m_sdName->text().trimmed();
            m_editSdId = sd.id;
            break;
        }
    } else if (page == 3) {
        for (auto& sd : s.screwdrivers)
            for (auto& p : sd.patterns)
                if (p.id == m_editPatId) {
                    p.id   = m_patId->text().trimmed();
                    p.name = m_patName->text().trimmed();
                    m_editPatId = p.id;
                }
    } else if (page == 4 && m_editChId >= 0) {
        for (auto& sd : s.screwdrivers)
            if (sd.id == m_editSdId)
                for (auto& p : sd.patterns)
                    if (p.id == m_editPatId)
                        for (auto& ch : p.channels)
                            if (ch.id == m_editChId) {
                                ch.id         = m_chId->value();
                                ch.name       = m_chName->text().trimmed();
                                ch.screwCount = m_chScrews->value();
                                m_editChId    = ch.id;
                            }
    }
    ConfigManager::instance().setStation(s);
}

void SettingsScreen::onConfigReloaded(const Station& s) {
    populateTree(s);
}
