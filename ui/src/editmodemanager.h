/*
  Q Light Controller Plus
  editmodemanager.h

  Minimal manager for coordinating UI Edit Mode operations.
*/
#ifndef EDITMODEMANAGER_H
#define EDITMODEMANAGER_H

#include <QObject>
#include <QSet>

class EditModeManager : public QObject
{
    Q_OBJECT
public:
    explicit EditModeManager(QObject *parent = nullptr)
        : QObject(parent)
        , m_paused(false)
    {}

    bool isPaused() const { return m_paused; }

public slots:
    void pauseProcessing() { if (!m_paused) { m_paused = true; emit editModeChanged(true); } }
    void resumeProcessing() { if (m_paused) { m_paused = false; emit editModeChanged(false); } }
    void clearModifications() { m_modifiedFunctions.clear(); emit modificationsCleared(); }

    // Record that a function has been modified while in edit mode
    void markFunctionModified(quint32 fid) { m_modifiedFunctions.insert(fid); emit functionModified(fid); }

signals:
    void editModeChanged(bool active);
    void functionModified(quint32 functionId);
    void modificationsCleared();

private:
    bool m_paused;
    QSet<quint32> m_modifiedFunctions;
};

#endif // EDITMODEMANAGER_H

