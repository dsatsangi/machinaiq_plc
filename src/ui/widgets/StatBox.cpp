#include "StatBox.h"
#include <QVBoxLayout>

StatBox::StatBox(const QString& label, Color color, QWidget* parent)
    : QFrame(parent)
{
    setObjectName("statBox");
    setMinimumHeight(80);

    auto* lay = new QVBoxLayout(this);
    lay->setAlignment(Qt::AlignCenter);

    m_value = new QLabel("0", this);
    m_value->setAlignment(Qt::AlignCenter);

    QString colorStr;
    switch (color) {
        case Green: colorStr = "#4caf50"; break;
        case Red:   colorStr = "#f44336"; break;
        default:    colorStr = "#e0e0e0"; break;
    }
    m_value->setStyleSheet(
        QStringLiteral("font-size: 28px; font-weight: bold; color: %1;").arg(colorStr));

    auto* lbl = new QLabel(label, this);
    lbl->setObjectName("statLabel");
    lbl->setAlignment(Qt::AlignCenter);

    lay->addWidget(m_value);
    lay->addWidget(lbl);
}

void StatBox::setValue(int v) {
    m_value->setText(QString::number(v));
}
