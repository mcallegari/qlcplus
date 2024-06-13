#include "vcscrollarea.h"
#include "qevent.h"

#include <QDebug>

VCScrollArea::VCScrollArea(QWidget *parent) : QScrollArea(parent) {
    // nothing more right now
}

void VCScrollArea::keyPressEvent(QKeyEvent *ev) {
    unsigned int keyCode = ev->key() | ev->modifiers();
    QList<unsigned int>::const_iterator it = std::find(m_overriddenKeyCodes.constBegin(), m_overriddenKeyCodes.constEnd(), keyCode);
    if (it != m_overriddenKeyCodes.constEnd()) {
        ev->ignore();
    } else {
        QScrollArea::keyPressEvent(ev);
    }
}

bool VCScrollArea::needsOverride(const QKeySequence& ks) {
    return ks == QKeySequence::MoveToPreviousPage
           || ks == QKeySequence::MoveToNextPage
           || ks == QKeySequence(Qt::Key_Up)
           || ks == QKeySequence(Qt::Key_Down)
           || ks == QKeySequence(Qt::Key_Left)
           || ks == QKeySequence(Qt::Key_Right);
}

void VCScrollArea::setKeyOverrides(const QList<QKeySequence> &keyOverrides) {
    m_overriddenKeyCodes.clear();
    for (QList<QKeySequence>::const_iterator it = keyOverrides.constBegin();
         it != keyOverrides.constEnd(); ++it) {
        if (needsOverride(*it)) {
            // all our key sequences have only one key
            unsigned int keyCode = (*it)[0];
            m_overriddenKeyCodes.append(keyCode);
        }
    }
}

void VCScrollArea::clearKeyOverrides() {
    m_overriddenKeyCodes.clear();
}
