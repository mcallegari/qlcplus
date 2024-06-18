#include "vcscrollarea.h"
#include "qevent.h"

VCScrollArea::VCScrollArea(QWidget *parent) : QScrollArea(parent) {
    // nothing more right now
}

void VCScrollArea::keyPressEvent(QKeyEvent *ev) {
    if (m_keyPassthru) {
        ev->ignore();
    } else {
        QScrollArea::keyPressEvent(ev);
    }
}

void VCScrollArea::setKeyPassthruEnabled(bool enable) {
    m_keyPassthru = enable;
}
