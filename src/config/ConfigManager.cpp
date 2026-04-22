#include "ConfigManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMutexLocker>
#include <QDebug>

ConfigManager& ConfigManager::instance() {
    static ConfigManager inst;
    return inst;
}

ConfigManager::ConfigManager(QObject* parent) : QObject(parent) {}

bool ConfigManager::load(const QString& path) {
    m_path = path;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "ConfigManager: cannot open" << path;
        return false;
    }
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "ConfigManager: JSON parse error:" << err.errorString();
        return false;
    }
    QMutexLocker lk(&m_mutex);
    m_station = stationFromJson(doc.object().value("station").toObject());
    return true;
}

bool ConfigManager::save() {
    if (m_path.isEmpty()) return false;
    QMutexLocker lk(&m_mutex);
    QJsonObject root;
    root["station"] = stationToJson(m_station);
    QFile f(m_path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

Station ConfigManager::station() const {
    QMutexLocker lk(&m_mutex);
    return m_station;
}

void ConfigManager::setStation(const Station& s) {
    { QMutexLocker lk(&m_mutex); m_station = s; }
    save();
    emit configReloaded(s);
}

void ConfigManager::reload() {
    if (load(m_path))
        emit configReloaded(m_station);
}

Screwdriver ConfigManager::screwdriverById(const QString& id) const {
    QMutexLocker lk(&m_mutex);
    for (const auto& sd : m_station.screwdrivers)
        if (sd.id == id) return sd;
    return {};
}

Pattern ConfigManager::patternById(const QString& sdId, const QString& patId) const {
    QMutexLocker lk(&m_mutex);
    for (const auto& sd : m_station.screwdrivers) {
        if (sd.id != sdId) continue;
        for (const auto& p : sd.patterns)
            if (p.id == patId) return p;
    }
    return {};
}

Channel ConfigManager::channelById(const QString& sdId, const QString& patId, int chId) const {
    auto pat = patternById(sdId, patId);
    for (const auto& ch : pat.channels)
        if (ch.id == chId) return ch;
    return {};
}

// ── JSON serialisation ────────────────────────────────────────────────────────

Station ConfigManager::stationFromJson(const QJsonObject& obj) const {
    Station s;
    s.id   = obj["id"].toString();
    s.name = obj["name"].toString();
    for (const auto& sdVal : obj["screwdrivers"].toArray()) {
        auto sdObj = sdVal.toObject();
        Screwdriver sd;
        sd.id   = sdObj["id"].toString();
        sd.name = sdObj["name"].toString();
        for (const auto& pVal : sdObj["patterns"].toArray()) {
            auto pObj = pVal.toObject();
            Pattern p;
            p.id   = pObj["id"].toString();
            p.name = pObj["name"].toString();
            for (const auto& chVal : pObj["channels"].toArray()) {
                auto chObj = chVal.toObject();
                Channel ch;
                ch.id         = chObj["id"].toInt();
                ch.name       = chObj["name"].toString();
                ch.screwCount = chObj["screwCount"].toInt();
                p.channels.append(ch);
            }
            sd.patterns.append(p);
        }
        s.screwdrivers.append(sd);
    }
    return s;
}

QJsonObject ConfigManager::stationToJson(const Station& s) const {
    QJsonObject obj;
    obj["id"]   = s.id;
    obj["name"] = s.name;
    QJsonArray sdArr;
    for (const auto& sd : s.screwdrivers) {
        QJsonObject sdObj;
        sdObj["id"]   = sd.id;
        sdObj["name"] = sd.name;
        QJsonArray pArr;
        for (const auto& p : sd.patterns) {
            QJsonObject pObj;
            pObj["id"]   = p.id;
            pObj["name"] = p.name;
            QJsonArray chArr;
            for (const auto& ch : p.channels) {
                QJsonObject chObj;
                chObj["id"]         = ch.id;
                chObj["name"]       = ch.name;
                chObj["screwCount"] = ch.screwCount;
                chArr.append(chObj);
            }
            pObj["channels"] = chArr;
            pArr.append(pObj);
        }
        sdObj["patterns"] = pArr;
        sdArr.append(sdObj);
    }
    obj["screwdrivers"] = sdArr;
    return obj;
}
