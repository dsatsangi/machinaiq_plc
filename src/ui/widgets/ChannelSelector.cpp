#include "ChannelSelector.h"
#include <QHBoxLayout>

ChannelSelector::ChannelSelector(int count, QWidget* parent) : QWidget(parent) {
    setObjectName("channelSelector");
    auto* lay = new QHBoxLayout(this);
    lay->setSpacing(4);
    lay->setContentsMargins(0, 0, 0, 0);

    for (int i = 1; i <= count; ++i) {
        auto* btn = new QPushButton(QStringLiteral("CH%1").arg(i), this);
        btn->setObjectName("chButton");
        btn->setCheckable(true);
        btn->setMinimumHeight(44);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        connect(btn, &QPushButton::clicked, this, [this, i]{
            setActiveChannel(i);
            emit channelSelected(i);
        });
        m_buttons.append(btn);
        lay->addWidget(btn);
    }

    setActiveChannel(1);
}

void ChannelSelector::setActiveChannel(int ch) {
    m_active = ch;
    for (int i = 0; i < m_buttons.size(); ++i)
        m_buttons[i]->setChecked(i + 1 == ch);
}
