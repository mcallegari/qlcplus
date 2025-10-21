#include "functiontracker.h"

#include <QMutexLocker>
#include <QDebug>

#include "doc.h"
#include "function.h"

static inline QString makeKey(quint32 fxi, quint32 ch)
{
    return QString::number(fxi) + ":" + QString::number(ch);
}

FunctionTracker::FunctionTracker(Doc *doc, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
{
    Q_ASSERT(doc != nullptr);
    // Connect to Doc signals to know when functions are added/removed so we can
    // connect to their running/stopped notifications.
    connect(m_doc, &Doc::functionAdded, this, [this](quint32 fid){
        if (Function *f = m_doc->function(fid)) {
            connect(f, &Function::running, this, &FunctionTracker::slotFunctionStarted);
            connect(f, qOverload<quint32>(&Function::stopped), this, &FunctionTracker::slotFunctionStopped);
        }
    });
    connect(m_doc, &Doc::functionRemoved, this, [this](quint32 fid){
        // If this function was still tracked as active, remove it
        removeFunction(fid);
    });

    // Also connect to already existing functions' signals (if any).
    const QList<Function*> funcs = m_doc->functions();
    for (Function *f : funcs)
    {
        connect(f, &Function::running, this, &FunctionTracker::slotFunctionStarted);
        connect(f, qOverload<quint32>(&Function::stopped), this, &FunctionTracker::slotFunctionStopped);
    }
}

FunctionTracker::~FunctionTracker()
{
}

bool FunctionTracker::isFunctionActive(quint32 fid) const
{
    QMutexLocker locker(&m_mutex);
    return m_activeFunctions.contains(fid);
}

QList<quint32> FunctionTracker::getActiveFunctions() const
{
    QMutexLocker locker(&m_mutex);
    return m_activeFunctions;
}

QList<SceneValue> FunctionTracker::getControlledChannels(quint32 fixtureId) const
{
    QMutexLocker locker(&m_mutex);
    QList<SceneValue> res;
    auto it = m_fixtureToChannels.find(fixtureId);
    if (it == m_fixtureToChannels.end())
        return res;
    for (quint32 ch : it.value())
        res << SceneValue(fixtureId, ch, 0);
    return res;
}

QList<quint32> FunctionTracker::getControllers(quint32 fixtureId, quint32 channel) const
{
    QMutexLocker locker(&m_mutex);
    return m_channelControllers.value(makeKey(fixtureId, channel));
}

quint32 FunctionTracker::getPrimaryController(quint32 fixtureId, quint32 channel) const
{
    QMutexLocker locker(&m_mutex);
    const QList<quint32> ctrls = m_channelControllers.value(makeKey(fixtureId, channel));
    if (ctrls.isEmpty())
        return Function::invalidId();
    return ctrls.constLast(); // most recently started wins
}

void FunctionTracker::slotFunctionStarted(quint32 functionId)
{
    addFunction(functionId);
}

void FunctionTracker::slotFunctionStopped(quint32 functionId)
{
    removeFunction(functionId);
}

void FunctionTracker::addFunction(quint32 functionId)
{
    Function *f = m_doc->function(functionId);
    if (f == nullptr)
        return;

    QMutexLocker locker(&m_mutex);
    if (!m_activeFunctions.contains(functionId))
        m_activeFunctions.append(functionId);

    // Track channels for Scene functions (extendable for others later)
    if (Scene *scene = qobject_cast<Scene*>(f))
        updateSceneChannels(scene, true);
}

void FunctionTracker::removeFunction(quint32 functionId)
{
    Function *f = m_doc->function(functionId);
    QMutexLocker locker(&m_mutex);
    m_activeFunctions.removeAll(functionId);

    if (Scene *scene = qobject_cast<Scene*>(f))
        updateSceneChannels(scene, false);

    // Remove this controller from all channel controller lists
    for (auto it = m_channelControllers.begin(); it != m_channelControllers.end(); )
    {
        QList<quint32> &lst = it.value();
        lst.removeAll(functionId);
        if (lst.isEmpty())
            it = m_channelControllers.erase(it);
        else
            ++it;
    }
}

void FunctionTracker::updateSceneChannels(Scene *scene, bool add)
{
    const QList<SceneValue> vals = scene->values();
    for (const SceneValue &sv : vals)
    {
        auto &set = m_fixtureToChannels[sv.fxi];
        if (add) set.insert(sv.channel); else set.remove(sv.channel);

        QString key = makeKey(sv.fxi, sv.channel);
        QList<quint32> &ctrls = m_channelControllers[key];
        if (add)
        {
            if (!ctrls.contains(scene->id()))
                ctrls.append(scene->id());
        }
        else
        {
            ctrls.removeAll(scene->id());
            if (ctrls.isEmpty())
                m_channelControllers.remove(key);
        }
    }
}

