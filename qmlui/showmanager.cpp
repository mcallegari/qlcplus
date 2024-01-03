/*
  Q Light Controller Plus
  showmanager.cpp

  Copyright (c) Massimo Callegari

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <QQmlContext>

#include "showmanager.h"
#include "sequence.h"
#include "tardis.h"
#include "chaser.h"
#include "track.h"
#include "show.h"
#include "doc.h"
#include "app.h"

ShowManager::ShowManager(QQuickView *view, Doc *doc, QObject *parent)
    : PreviewContext(view, doc, "SHOWMGR", parent)
    , m_currentShow(nullptr)
    , m_stretchFunctions(false)
    , m_gridEnabled(false)
    , m_timeScale(5.0)
    , m_currentTime(0)
    , m_selectedTrackIndex(-1)
    , m_itemsColor(Qt::gray)
{
    view->rootContext()->setContextProperty("showManager", this);
    qmlRegisterUncreatableType<Show>("org.qlcplus.classes", 1, 0, "Show", "Can't create a Show");
    qmlRegisterType<Track>("org.qlcplus.classes", 1, 0, "Track");
    qmlRegisterUncreatableType<ShowFunction>("org.qlcplus.classes", 1, 0, "ShowFunction", "Can't create a ShowFunction");

    setContextResource("qrc:/ShowManager.qml");
    setContextTitle(tr("Show Manager"));

    App *app = qobject_cast<App *>(m_view);
    m_tickSize = app->pixelDensity() * 18;

    siComponent = new QQmlComponent(m_view->engine(), QUrl("qrc:/ShowItem.qml"));
    if (siComponent->isError())
        qDebug() << siComponent->errors();
}

int ShowManager::currentShowID() const
{
    if (m_currentShow == nullptr)
        return Function::invalidId();

    return m_currentShow->id();
}

Show *ShowManager::currentShow() const
{
    return m_currentShow;
}

bool ShowManager::isEditing()
{
    return m_currentShow == nullptr ? false : true;
}

void ShowManager::setCurrentShowID(int currentShowID)
{
    if (m_currentShow != nullptr)
    {
        if (m_currentShow->id() == (quint32)currentShowID)
            return;
        disconnect(m_currentShow, SIGNAL(timeChanged(quint32)), this, SLOT(slotTimeChanged(quint32)));
    }

    m_currentShow = qobject_cast<Show*>(m_doc->function(currentShowID));
    emit currentShowIDChanged(currentShowID);
    emit isEditingChanged();

    if (m_currentShow != nullptr)
    {
        connect(m_currentShow, SIGNAL(timeChanged(quint32)), this, SLOT(slotTimeChanged(quint32)));
        emit showDurationChanged(m_currentShow->totalDuration());
        emit showNameChanged(m_currentShow->name());
    }
    else
    {
        emit showDurationChanged(0);
        emit showNameChanged("");
    }
    emit tracksChanged();
}

QString ShowManager::showName() const
{
    if (m_currentShow == nullptr)
        return QString();

    return m_currentShow->name();
}

void ShowManager::setShowName(QString showName)
{
    if (m_currentShow == nullptr || m_currentShow->name() == showName)
        return;

    Tardis::instance()->enqueueAction(Tardis::FunctionSetName, m_currentShow->id(), m_currentShow->name(), showName);

    m_currentShow->setName(showName);
    emit showNameChanged(showName);
}

bool ShowManager::stretchFunctions() const
{
    return m_stretchFunctions;
}

void ShowManager::setStretchFunctions(bool stretchFunctions)
{
    if (m_stretchFunctions == stretchFunctions)
        return;

    m_stretchFunctions = stretchFunctions;
    emit stretchFunctionsChanged(stretchFunctions);
}

bool ShowManager::gridEnabled() const
{
    return m_gridEnabled;
}

void ShowManager::setGridEnabled(bool gridEnabled)
{
    if (m_gridEnabled == gridEnabled)
        return;

    m_gridEnabled = gridEnabled;
    emit gridEnabledChanged(m_gridEnabled);
}

/*********************************************************************
 * Time
 ********************************************************************/

Show::TimeDivision ShowManager::timeDivision()
{
    if (m_currentShow == nullptr)
        return Show::Time;

    return m_currentShow->timeDivisionType();
}

void ShowManager::setTimeDivision(Show::TimeDivision division)
{
    if (m_currentShow == nullptr)
        return;

    if (division == m_currentShow->timeDivisionType())
        return;

    if (division == Show::Time)
    {
        setTimeScale(5.0);
        m_currentShow->setTempoType(Function::Time);
    }
    else
    {
        setTimeScale(1.0);
        m_currentShow->setTempoType(Function::Beats);
    }
    m_currentShow->setTimeDivisionType(division);
    emit timeDivisionChanged(division);

    if (division != Show::Time)
        emit beatsDivisionChanged(m_currentShow->beatsDivision());
}

int ShowManager::beatsDivision()
{
    if (m_currentShow == nullptr)
        return 0;

    return m_currentShow->beatsDivision();
}

float ShowManager::timeScale() const
{
    return m_timeScale;
}

void ShowManager::setTimeScale(float timeScale)
{
    if (m_timeScale == timeScale)
        return;

    m_timeScale = timeScale;
    float tickScale = timeDivision() == Show::Time ? 1.0 : timeScale;

    App *app = qobject_cast<App *>(m_view);
    m_tickSize = app->pixelDensity() * (18 * tickScale);

    emit tickSizeChanged(m_tickSize);
    emit timeScaleChanged(timeScale);
}

float ShowManager::tickSize() const
{
    return m_tickSize;
}

int ShowManager::currentTime() const
{
    return m_currentTime;
}

void ShowManager::setCurrentTime(int currentTime)
{
    if (m_currentTime == currentTime)
        return;

    m_currentTime = currentTime;
    emit currentTimeChanged(currentTime);
}

/*********************************************************************
 * Tracks
 ********************************************************************/

QVariant ShowManager::tracks()
{
    m_tracksList.clear();
    if (m_currentShow)
        m_tracksList = m_currentShow->tracks();

    return QVariant::fromValue(m_tracksList);
}

int ShowManager::selectedTrackIndex() const
{
    return m_selectedTrackIndex;
}

void ShowManager::setSelectedTrackIndex(int index)
{
    if (m_selectedTrackIndex == index)
        return;

    m_selectedTrackIndex = index;
    emit selectedTrackIndexChanged(index);
}

void ShowManager::setTrackSolo(int index, bool solo)
{
    QList<Track*> tracks = m_currentShow->tracks();

    if (index < 0 || index >= tracks.count())
        return;

    for (int i = 0; i < tracks.count(); i++)
    {
        if (i == index)
            tracks.at(i)->setMute(false);
        else
            tracks.at(i)->setMute(solo);
    }
}

void ShowManager::moveTrack(int index, int direction)
{
    QList<Track*> tracks = m_currentShow->tracks();

    if (index < 0 || index >= tracks.count())
        return;

    m_currentShow->moveTrack(tracks.at(index), direction);
    m_doc->setModified();

    emit tracksChanged();
}

/*********************************************************************
  * Show Items
  ********************************************************************/

void ShowManager::addItems(QQuickItem *parent, int trackIdx, int startTime, QVariantList idsList)
{
    if (idsList.count() == 0)
        return;

    // if no show is selected, then create a new one
    if (m_currentShow == nullptr)
    {
        QString defaultName = QString("%1 %2").arg(tr("New Show")).arg(m_doc->nextFunctionID());
        m_currentShow = new Show(m_doc);
        m_currentShow->setName(defaultName);
        Function *f = qobject_cast<Function*>(m_currentShow);
        if (m_doc->addFunction(f) == false)
        {
            qDebug() << "Error in creating a new Show!";
            m_currentShow = nullptr;
            return;
        }

        Tardis::instance()->enqueueAction(Tardis::FunctionCreate, m_currentShow->id(), QVariant(),
                                          Tardis::instance()->actionToByteArray(Tardis::FunctionCreate, m_currentShow->id()));

        connect(m_currentShow, SIGNAL(timeChanged(quint32)), this, SLOT(slotTimeChanged(quint32)));
        emit currentShowIDChanged(m_currentShow->id());
        emit showNameChanged(m_currentShow->name());
        emit isEditingChanged();
    }

    Track *selectedTrack = nullptr;

    // if no Track index is provided, then add a new one
    if (trackIdx == -1)
    {
        selectedTrack = new Track(Function::invalidId(), m_currentShow);
        selectedTrack->setName(tr("Track %1").arg(m_currentShow->tracks().count() + 1));
        m_currentShow->addTrack(selectedTrack);

        Tardis::instance()->enqueueAction(
            Tardis::ShowManagerAddTrack, m_currentShow->id(), QVariant(),
            Tardis::instance()->actionToByteArray(Tardis::ShowManagerAddTrack, m_currentShow->id(), selectedTrack->id()));

        trackIdx = m_currentShow->tracks().count() - 1;
        emit tracksChanged();
    }
    else
    {
        if (trackIdx >= m_currentShow->tracks().count())
        {
            qDebug() << "Track index out of bounds!" << trackIdx;
            return;
        }
        selectedTrack = m_currentShow->tracks().at(trackIdx);
    }

    for (QVariant &vID : idsList) // C++11
    {
        quint32 functionID = vID.toUInt();
        if (functionID == m_currentShow->id())
        {
            /* TODO: a popup displaying the user stupidity would be nice here... */
            continue;
        }

        // and now create the actual ShowFunction and the QML item
        Function *func = m_doc->function(functionID);
        if (func == nullptr)
            continue;

        ShowFunction *showFunc = selectedTrack->createShowFunction(functionID);

        if (timeDivision() == Show::Time)
        {
            func->setTempoType(Function::Time);
            showFunc->setDuration(func->totalDuration() ? func->totalDuration() : 5000);
        }
        else
        {
            func->setTempoType(Function::Beats);
            if (func->type() == Function::AudioType || func->type() == Function::VideoType)
                func->setTotalDuration(func->duration());
            showFunc->setDuration(func->totalDuration() ? func->totalDuration() : 4000);
        }
        showFunc->setStartTime(startTime);
        showFunc->setColor(ShowFunction::defaultColor(func->type()));

        Tardis::instance()->enqueueAction(
            Tardis::ShowManagerAddFunction, m_currentShow->id(), QVariant(),
            Tardis::instance()->actionToByteArray(Tardis::ShowManagerAddFunction, m_currentShow->id(), showFunc->id()));

        QQuickItem *newItem = qobject_cast<QQuickItem*>(siComponent->create());

        newItem->setParentItem(parent);
        newItem->setProperty("trackIndex", trackIdx);
        newItem->setProperty("sfRef", QVariant::fromValue(showFunc));
        newItem->setProperty("funcRef", QVariant::fromValue(func));

        m_itemsMap[showFunc->id()] = newItem;
        startTime += showFunc->duration();
    }

    emit showDurationChanged(m_currentShow->totalDuration());
}

void ShowManager::addShowItem(ShowFunction *sf, quint32 trackId)
{
    QQuickItem *itemsArea = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("showItemsArea"));
    QQuickItem *contentItem = qobject_cast<QQuickItem*>(itemsArea->findChild<QObject *>("contentItem"));
    QQuickItem *newItem = qobject_cast<QQuickItem*>(siComponent->create());
    Function *func = m_doc->function(sf->functionID());

    newItem->setParentItem(contentItem);
    newItem->setProperty("trackIndex", trackId);
    newItem->setProperty("sfRef", QVariant::fromValue(sf));
    newItem->setProperty("funcRef", QVariant::fromValue(func));
    m_itemsMap[sf->id()] = newItem;
}

void ShowManager::deleteShowItems(QVariantList data)
{
    Q_UNUSED(data);

    if (m_currentShow == nullptr)
        return;

    foreach (SelectedShowItem ssi, m_selectedItems)
    {
        quint32 trackIndex = ssi.m_trackIndex;
        qDebug() << "Selected item has track index:" << trackIndex;

        for (int i = 0; i < m_clipboard.count(); i++)
        {
            SelectedShowItem cItem = m_clipboard.at(i);
            if (cItem.m_showFunc == ssi.m_showFunc)
                m_clipboard.removeAt(i);
        }

        Track *track = m_currentShow->tracks().at(trackIndex);
        quint32 sfId = ssi.m_showFunc->id();
        track->removeShowFunction(ssi.m_showFunc, true);
        if (ssi.m_item != nullptr)
        {
            m_itemsMap.remove(sfId);
            delete ssi.m_item;
        }
    }

    m_selectedItems.clear();
    emit selectedItemsCountChanged(0);
}

void ShowManager::deleteShowItem(ShowFunction *sf)
{
    quint32 sfId = sf->id();
    QQuickItem *item = m_itemsMap.value(sfId, nullptr);
    if (item != nullptr)
    {
        m_itemsMap.remove(sfId);
        delete item;
    }
}

bool ShowManager::checkAndMoveItem(ShowFunction *sf, int originalTrackIdx, int newTrackIdx, int newStartTime)
{
    if (m_currentShow == nullptr || sf == nullptr)
        return false;

    //qDebug() << Q_FUNC_INFO << "origIdx:" << originalTrackIdx << "newIdx:" << newTrackIdx << "time:" << newStartTime;

    Track *dstTrack = nullptr;

    // check if it's moving on a new track or an existing one
    if (newTrackIdx >= m_currentShow->tracks().count())
    {
        // create a new track here
        dstTrack = new Track(Function::invalidId(), m_currentShow);
        dstTrack->setName(tr("Track %1").arg(m_currentShow->tracks().count() + 1));
        m_currentShow->addTrack(dstTrack);
        emit tracksChanged();
    }
    else
    {
        dstTrack = m_currentShow->tracks().at(newTrackIdx);

        bool overlapping = checkOverlapping(dstTrack, sf, newStartTime, sf->duration());
        if (overlapping == true)
            return false;
    }

    int newTime = newStartTime;

    if (m_gridEnabled)
    {
        // calculate the X position from time and time scale
        // timescale * 1000 : tickSize = time : x
        float xPos = ((float)newStartTime * m_tickSize) / (m_timeScale * 1000.0);
        // round to the nearest snap position
        xPos = qRound(xPos / m_tickSize) * m_tickSize;
        // recalculate the time from pixels
        // xPos : time = tickSize : timescale * 1000
        newTime = xPos * (1000 * m_timeScale) / m_tickSize;
    }

    Tardis::instance()->enqueueAction(Tardis::ShowManagerItemSetStartTime, sf->id(), sf->startTime(), newTime);
    sf->setStartTime(newTime);

    // check if we need to move the ShowFunction to a different Track
    if (newTrackIdx != originalTrackIdx)
    {
        Track *srcTrack = m_currentShow->tracks().at(originalTrackIdx);
        srcTrack->removeShowFunction(sf, false);
        dstTrack->addShowFunction(sf);
    }

    m_doc->setModified();

    return true;
}

bool ShowManager::setShowItemStartTime(ShowFunction *sf, int startTime)
{
    if (sf == nullptr)
        return false;

    Track *track = m_currentShow->getTrackFromShowFunctionID(sf->id());
    if (track == nullptr)
        return false;

    bool overlapping = checkOverlapping(track, sf, startTime, sf->duration());
    if (overlapping)
        return false;

    Tardis::instance()->enqueueAction(Tardis::ShowManagerItemSetStartTime, sf->id(), sf->startTime(), startTime);
    sf->setStartTime(startTime);

    return true;
}

bool ShowManager::setShowItemDuration(ShowFunction *sf, int duration)
{
    if (sf == nullptr)
        return false;

    Track *track = m_currentShow->getTrackFromShowFunctionID(sf->id());
    if (track == nullptr)
        return false;

    bool overlapping = checkOverlapping(track, sf, sf->startTime(), duration);
    if (overlapping)
        return false;

    Tardis::instance()->enqueueAction(Tardis::ShowManagerItemSetDuration, sf->id(), sf->duration(), duration);
    sf->setDuration(duration);

    return true;
}

void ShowManager::resetContents()
{
    resetView();
    m_currentTime = 0;
    m_selectedTrackIndex = -1;
    emit currentTimeChanged(m_currentTime);
    m_currentShow = nullptr;
}

void ShowManager::resetView()
{
    QMapIterator<quint32, QQuickItem*> it(m_itemsMap);
    while (it.hasNext())
    {
        it.next();
        delete it.value();
    }
    m_itemsMap.clear();
}

void ShowManager::renderView(QQuickItem *parent)
{
    resetView();

    if (m_currentShow == nullptr)
        return;

    setContextItem(parent);

    int trkIdx = 0;

    foreach (Track *track, m_currentShow->tracks())
    {
        foreach (ShowFunction *sf, track->showFunctions())
        {
            Function *func = m_doc->function(sf->functionID());
            if (func == nullptr)
                continue;

            QQuickItem *newItem = qobject_cast<QQuickItem*>(siComponent->create());

            newItem->setParentItem(parent);
            newItem->setProperty("trackIndex", trkIdx);
            newItem->setProperty("sfRef", QVariant::fromValue(sf));
            newItem->setProperty("funcRef", QVariant::fromValue(func));

            m_itemsMap[sf->id()] = newItem;
        }

        trkIdx++;
    }
}

void ShowManager::enableFlicking(bool enable)
{
    QQuickItem *flickable = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("showItemsArea"));
    flickable->setProperty("interactive", enable);
}

int ShowManager::showDuration() const
{
    if (m_currentShow == nullptr)
        return 0;

    return m_currentShow->totalDuration();
}

void ShowManager::playShow()
{
    if (m_currentShow == nullptr)
        return;

    m_currentShow->start(m_doc->masterTimer(), FunctionParent::master(), m_currentTime);
    emit isPlayingChanged(true);
}

void ShowManager::stopShow()
{
    if (m_currentShow != nullptr && m_currentShow->isRunning())
    {
        m_currentShow->stop(FunctionParent::master());
        emit isPlayingChanged(false);
        return;
    }
    m_currentTime = 0;
    emit currentTimeChanged(m_currentTime);
}

bool ShowManager::isPlaying() const
{
    if (m_currentShow != nullptr && m_currentShow->isRunning())
        return true;
    return false;
}

QColor ShowManager::itemsColor() const
{
    return m_itemsColor;
}

void ShowManager::setItemsColor(QColor itemsColor)
{
    if (m_itemsColor == itemsColor)
        return;

    m_itemsColor = itemsColor;
    emit itemsColorChanged(itemsColor);
}

int ShowManager::selectedItemsCount() const
{
    return m_selectedItems.count();
}

void ShowManager::setItemSelection(int trackIdx, ShowFunction *sf, QQuickItem *item, bool selected)
{
    if (selected == true)
    {
        SelectedShowItem selection;
        selection.m_trackIndex = trackIdx;
        selection.m_showFunc = sf;
        selection.m_item = item;
        m_selectedItems.append(selection);
    }
    else
    {
        for (int i = 0; i < m_selectedItems.count(); i++)
        {
            SelectedShowItem si = m_selectedItems.at(i);
            if (si.m_showFunc == sf)
            {
                m_selectedItems.removeAt(i);
                break;
            }
        }
    }
    emit selectedItemsCountChanged(m_selectedItems.count());
}

void ShowManager::resetItemsSelection()
{
    foreach (SelectedShowItem ssi, m_selectedItems)
    {
        if (ssi.m_item != nullptr)
            ssi.m_item->setProperty("isSelected", false);
    }
    m_selectedItems.clear();
}

QVariantList ShowManager::selectedItemRefs()
{
    QVariantList list;
    /*
    for (int i = 0; i < m_selectedItems.count(); i++)
    {
        list.append(QVariant::fromValue(m_selectedItems.at(i)));
    }
    */
    return list;
}

QStringList ShowManager::selectedItemNames()
{
    QStringList names;
    foreach (SelectedShowItem si, m_selectedItems)
    {
        Function *func = m_doc->function(si.m_showFunc->functionID());
        if (func != nullptr)
            names.append(func->name());
    }

    return names;
}

bool ShowManager::selectedItemsLocked()
{
    foreach (SelectedShowItem si, m_selectedItems)
    {
        if (si.m_showFunc != nullptr && si.m_showFunc->isLocked())
            return true;
    }
    return false;
}

void ShowManager::setSelectedItemsLock(bool lock)
{
    foreach (SelectedShowItem si, m_selectedItems)
    {
        if (si.m_showFunc != nullptr)
            si.m_showFunc->setLocked(lock);
    }
}

void ShowManager::slotTimeChanged(quint32 msec_time)
{
    m_currentTime = (int)msec_time;
    emit currentTimeChanged(m_currentTime);
}

bool ShowManager::checkOverlapping(Track *track, ShowFunction *sourceFunc,
                                   quint32 startTime, quint32 duration)
{
    if (track == nullptr)
        return false;

    foreach (ShowFunction *sf, track->showFunctions())
    {
        if (sf == sourceFunc)
            continue;

        Function *func = m_doc->function(sf->functionID());
        if (func != nullptr)
        {
            quint32 fst = sf->startTime();
            if ((startTime >= fst && startTime <= fst + sf->duration()) ||
                (fst >= startTime && fst <= startTime + duration))
            {
                return true;
            }
        }
    }

    return false;
}

QVariantList ShowManager::previewData(Function *f) const
{
    QVariantList data;
    if (f == nullptr)
        return data;

    switch (f->type())
    {
        case Function::ChaserType:
        case Function::SequenceType:
        {
            Chaser *chaser = qobject_cast<Chaser *>(f);
            quint32 stepsTimeCounter = 0;

            foreach (ChaserStep step, chaser->steps())
            {
                uint stepFadeIn = step.fadeIn;
                uint stepFadeOut = step.fadeOut;
                uint stepDuration = step.duration;
                if (chaser->fadeInMode() == Chaser::Common)
                    stepFadeIn = chaser->fadeInSpeed();
                if (chaser->fadeOutMode() == Chaser::Common)
                    stepFadeOut = chaser->fadeOutSpeed();
                if (chaser->durationMode() == Chaser::Common)
                    stepDuration = chaser->duration();

                stepsTimeCounter += stepDuration;

                if (stepFadeIn > 0)
                {
                    data.append(FadeIn);
                    data.append(stepFadeIn);
                }
                data.append(StepDivider);
                data.append(stepsTimeCounter);

                if (stepFadeOut > 0)
                {
                    data.append(FadeOut);
                    data.append(stepFadeOut);
                }
            }
        }
        break;

        /* All the other Function types */
        case Function::AudioType:
        case Function::VideoType:
        {
            data.append(RepeatingDuration);
            data.append(f->totalDuration());
            data.append(FadeIn);
            data.append(f->fadeInSpeed());
            data.append(FadeOut);
            data.append(f->fadeOutSpeed());
        }
        break;
        default:
        {
            data.append(RepeatingDuration);
            data.append(f->totalDuration());
        }
        break;
    }

    return data;
}

void ShowManager::copyToClipboard()
{
    m_clipboard.clear();

    for (SelectedShowItem item : m_selectedItems)
        m_clipboard.append(item);
}

void ShowManager::pasteFromClipboard()
{
    quint32 lowerTime = UINT_MAX;

    // pre-parse copied items to find the one with lowest start time
    for (SelectedShowItem item : m_clipboard)
    {
        if (item.m_showFunc->startTime() < lowerTime)
            lowerTime = item.m_showFunc->startTime();
    }

    // now clone and add Functions and ShowFunctions on the proper tracks
    // and keeping the delta time of the original items
    for (SelectedShowItem item : m_clipboard)
    {
        Track *track = m_currentShow->tracks().at(item.m_trackIndex);

        if (checkOverlapping(track, item.m_showFunc, m_currentTime, item.m_showFunc->duration()))
            continue;

        Function *func = m_doc->function(item.m_showFunc->functionID());
        if (func == nullptr)
            continue;

        Function *copyFunc = func->createCopy(m_doc);
        if (copyFunc == nullptr)
            continue;

        copyFunc->setName(QString("%1 %2").arg(copyFunc->name()).arg(tr("(Copy)")));

        if (copyFunc->type() == Function::SequenceType)
        {
            Sequence *sequence = qobject_cast<Sequence*>(copyFunc);
            Scene *scene = qobject_cast<Scene*>(m_doc->function(sequence->boundSceneID()));
            if (scene == nullptr)
                continue;

            Scene *copyScene = static_cast<Scene*>(scene->createCopy(m_doc, true));
            if (copyScene == nullptr)
                continue;

            copyScene->setName(QString("%1 %2").arg(copyScene->name()).arg(tr("(Copy)")));

            m_doc->addFunction(copyScene);
            sequence->setBoundSceneID(copyScene->id());
        }

        m_doc->addFunction(copyFunc);

        addItems(contextItem(), item.m_trackIndex,
                 m_currentTime + item.m_showFunc->startTime() - lowerTime,
                 QVariantList() << copyFunc->id());
    }
}




