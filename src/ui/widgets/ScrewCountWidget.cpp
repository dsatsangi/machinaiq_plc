#include "ScrewCountWidget.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

ScrewCountWidget::ScrewCountWidget(QWidget* parent) : QWidget(parent) {
    setObjectName("screwCountWidget");

    auto* outerLay = new QHBoxLayout(this);
    outerLay->setSpacing(8);

    // ── Left: cycle count ────────────────────────────────────────────────────
    auto* cycleFrame = new QFrame(this);
    cycleFrame->setObjectName("dataCard");
    auto* cycleLay = new QVBoxLayout(cycleFrame);
    cycleLay->setAlignment(Qt::AlignCenter);

    auto* cycleTitle = new QLabel("CURRENT CYCLE", cycleFrame);
    cycleTitle->setObjectName("cardTitle");
    cycleTitle->setAlignment(Qt::AlignCenter);

    auto* countRow = new QHBoxLayout;
    countRow->setAlignment(Qt::AlignCenter);
    m_countLabel = new QLabel("0", cycleFrame);
    m_countLabel->setObjectName("bigCount");
    m_totalLabel = new QLabel("/ 0", cycleFrame);
    m_totalLabel->setObjectName("totalCount");
    countRow->addWidget(m_countLabel);
    countRow->addWidget(m_totalLabel);

    auto* cycleSubtitle = new QLabel("SCREW COUNT", cycleFrame);
    cycleSubtitle->setObjectName("cardSubtitle");
    cycleSubtitle->setAlignment(Qt::AlignCenter);

    cycleLay->addWidget(cycleTitle);
    cycleLay->addLayout(countRow);
    cycleLay->addWidget(cycleSubtitle);

    // ── Right: time + program ────────────────────────────────────────────────
    auto* rightCol = new QVBoxLayout;
    rightCol->setSpacing(8);

    auto* timeFrame = new QFrame(this);
    timeFrame->setObjectName("dataCard");
    auto* timeLay = new QVBoxLayout(timeFrame);
    timeLay->setAlignment(Qt::AlignCenter);
    auto* timeTitle = new QLabel("TIME", timeFrame);
    timeTitle->setObjectName("cardTitle");
    timeTitle->setAlignment(Qt::AlignCenter);
    m_timeLabel = new QLabel("0", timeFrame);
    m_timeLabel->setObjectName("medCount");
    m_timeLabel->setAlignment(Qt::AlignCenter);
    auto* msLabel = new QLabel("ms", timeFrame);
    msLabel->setObjectName("cardSubtitle");
    msLabel->setAlignment(Qt::AlignCenter);
    timeLay->addWidget(timeTitle);
    timeLay->addWidget(m_timeLabel);
    timeLay->addWidget(msLabel);

    auto* progFrame = new QFrame(this);
    progFrame->setObjectName("dataCard");
    auto* progLay = new QVBoxLayout(progFrame);
    progLay->setAlignment(Qt::AlignCenter);
    auto* progTitle = new QLabel("ACTIVE PROGRAM", progFrame);
    progTitle->setObjectName("cardTitle");
    progTitle->setAlignment(Qt::AlignCenter);
    m_programLabel = new QLabel("—", progFrame);
    m_programLabel->setObjectName("programName");
    m_programLabel->setAlignment(Qt::AlignCenter);
    progLay->addWidget(progTitle);
    progLay->addWidget(m_programLabel);

    rightCol->addWidget(timeFrame);
    rightCol->addWidget(progFrame);

    outerLay->addWidget(cycleFrame, 3);
    outerLay->addLayout(rightCol, 2);
}

void ScrewCountWidget::update(int current, int total, int timeMs) {
    m_countLabel->setText(QString::number(current));
    m_totalLabel->setText(QStringLiteral("/ %1").arg(total));
    m_timeLabel->setText(QString::number(timeMs));
}

void ScrewCountWidget::setProgram(const QString& program) {
    m_programLabel->setText(program.isEmpty() ? "—" : program);
}
