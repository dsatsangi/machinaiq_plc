#pragma once
#include <QString>
#include <QList>

struct Channel {
    int     id         = 1;
    QString name;
    int     screwCount = 0;
};

struct Pattern {
    QString       id;
    QString       name;
    QList<Channel> channels;  // max 8
};

struct Screwdriver {
    QString        id;
    QString        name;
    QList<Pattern> patterns;  // max 5
};

struct Station {
    QString            id;
    QString            name;
    QList<Screwdriver> screwdrivers;  // max 5
};
