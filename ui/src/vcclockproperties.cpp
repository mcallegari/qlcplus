#include "vcclockproperties.h"


VCClockProperties::VCClockProperties(VCClock *clock)
    : QDialog(clock)
    , m_clock(clock)
{
    Q_ASSERT(clock != NULL);

    setupUi(this);

    switch(m_clock->clockType())
    {
        case VCClock::Stopwatch:
            m_stopWatchRadio->setChecked(true);
        break;
        case VCClock::Countdown:
        {
            m_countdownRadio->setChecked(true);
            m_hoursSpin->setValue(m_clock->getHours());
            m_minutesSpin->setValue(m_clock->getMinutes());
            m_secondsSpin->setValue(m_clock->getSeconds());
        }
        break;
        case VCClock::Clock:
        default:
            m_clockRadio->setChecked(true);
        break;
    }
}

VCClockProperties::~VCClockProperties()
{

}

void VCClockProperties::accept()
{
    if (m_clockRadio->isChecked())
        m_clock->setClockType(VCClock::Clock);
    else if (m_stopWatchRadio->isChecked())
        m_clock->setClockType(VCClock::Stopwatch);
    else if (m_countdownRadio->isChecked())
    {
        m_clock->setClockType(VCClock::Countdown);
        m_clock->setCountdown(m_hoursSpin->value(), m_minutesSpin->value(), m_secondsSpin->value());
    }

    QDialog::accept();
}
