#pragma once
#include <QObject>
#include <QMutex>
#include "models/HierarchyModels.h"

// Loads/saves Station hierarchy from a JSON file.
// Hot-reload: call reload() to apply changes without restart.
class ConfigManager : public QObject {
    Q_OBJECT
public:
    static ConfigManager& instance();

    bool load(const QString& path);
    bool save();

    Station station() const;
    void    setStation(const Station& s);

    // Convenience: look up by id
    Screwdriver screwdriverById(const QString& id) const;
    Pattern     patternById(const QString& sdId, const QString& patId) const;
    Channel     channelById(const QString& sdId, const QString& patId, int chId) const;

signals:
    void configReloaded(const Station& station);

public slots:
    void reload();

private:
    explicit ConfigManager(QObject* parent = nullptr);
    Q_DISABLE_COPY_MOVE(ConfigManager)

    Station stationFromJson(const QJsonObject& obj) const;
    QJsonObject stationToJson(const Station& s) const;

    mutable QMutex m_mutex;
    Station        m_station;
    QString        m_path;
};
