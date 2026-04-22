#pragma once
#include <QWidget>
#include <QTreeWidget>
#include <QLineEdit>
#include <QSpinBox>
#include "config/models/HierarchyModels.h"

class QStackedWidget;
class QPushButton;

// Full CRUD for Station > Screwdriver > Pattern > Channel hierarchy.
// Changes are persisted immediately via ConfigManager.
class SettingsScreen : public QWidget {
    Q_OBJECT
public:
    explicit SettingsScreen(QWidget* parent = nullptr);

signals:
    void homeRequested();

private slots:
    void onTreeSelectionChanged();
    void onAddScrewdriver();
    void onAddPattern();
    void onAddChannel();
    void onDeleteSelected();
    void onSaveDetail();
    void onConfigReloaded(const Station& s);

private:
    void buildUi();
    void populateTree(const Station& s);
    void showStationDetail(const Station& s);
    void showScrewdriverDetail(const Screwdriver& sd);
    void showPatternDetail(const Pattern& p);
    void showChannelDetail(const Channel& ch, const QString& sdId, const QString& patId);
    void clearDetail();

    Station currentStationCopy();

    // Tree
    QTreeWidget*   m_tree;
    // Detail panel (stacked)
    QStackedWidget* m_detailStack;

    // Station detail fields
    QLineEdit* m_stId;
    QLineEdit* m_stName;

    // Screwdriver detail
    QLineEdit* m_sdId;
    QLineEdit* m_sdName;

    // Pattern detail
    QLineEdit* m_patId;
    QLineEdit* m_patName;

    // Channel detail
    QSpinBox*  m_chId;
    QLineEdit* m_chName;
    QSpinBox*  m_chScrews;

    // Context for save
    QString m_editSdId;
    QString m_editPatId;
    int     m_editChId = -1;
};
