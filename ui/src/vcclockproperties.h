#ifndef VCCLOCKPROPERTIES_H
#define VCCLOCKPROPERTIES_H

#include <QDialog>

#include "ui_vcclockproperties.h"
#include "vcclock.h"

class VCClockProperties : public QDialog, public Ui_VCClockProperties
{
    Q_OBJECT

public:
    VCClockProperties(VCClock *clock);
    ~VCClockProperties();

public slots:
    void accept();

private:
    VCClock *m_clock;

};

#endif // VCCLOCKPROPERTIES_H
