#ifndef FUNCTIONTRACKER_H
#define FUNCTIONTRACKER_H

#include <QObject>
#include <QHash>
#include <QSet>
#include <QList>
#include <QMutex>

#include "scene.h"

class Doc;
class Function;

// FunctionTracker: tracks running Functions and the channels they control.
// It observes Function::running/quint32 and Function::stopped/quint32 signals
// emitted by Function instances managed by Doc, so that a UI can query which
// functions are active and which controller owns a channel.
class FunctionTracker : public QObject
{
    Q_OBJECT

public:
    explicit FunctionTracker(Doc *doc, QObject *parent = nullptr);
    ~FunctionTracker();

    // Returns true if the given function ID is currently active (running).
    bool isFunctionActive(quint32 fid) const;

    // Returns the set of currently active function IDs.
    QList<quint32> getActiveFunctions() const;

    // Return the list of SceneValue controlled for a given fixture ID by all active functions.
    QList<SceneValue> getControlledChannels(quint32 fixtureId) const;

    // Return all controller function IDs that affect a given fixture/channel.
    QList<quint32> getControllers(quint32 fixtureId, quint32 channel) const;

    // Heuristic: pick a single controller as primary owner for a channel.
    // For now, return the most recently started controller among those affecting the channel.
    quint32 getPrimaryController(quint32 fixtureId, quint32 channel) const;

public slots:
    void slotFunctionStarted(quint32 functionId);
    void slotFunctionStopped(quint32 functionId);

private:
    void addFunction(quint32 functionId);
    void removeFunction(quint32 functionId);
    void updateSceneChannels(Scene *scene, bool add);

private:
    Doc *m_doc;

    // Active function IDs (in start order)
    QList<quint32> m_activeFunctions;

    // Map: fixtureId -> list of channels controlled (deduped via QSet)
    QHash<quint32, QSet<quint32>> m_fixtureToChannels;

    // Map: key "fixtureId:channel" -> controllers affecting it (order = start order)
    QHash<QString, QList<quint32>> m_channelControllers;

    // Protects internal state
    mutable QMutex m_mutex;
};

#endif // FUNCTIONTRACKER_H

