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
    , m_currentShow(NULL)
    , m_timeScale(5.0)
    , m_stretchFunctions(false)
    , m_gridEnabled(false)
    , m_currentTime(0)
    , m_selectedTrack(-1)
    , m_itemsColor(Qt::gray)
{
    qmlRegisterType<Track>("org.qlcplus.classes", 1, 0, "Track");
    qmlRegisterType<ShowFunction>("org.qlcplus.classes", 1, 0, "ShowFunction");

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
    if (m_currentShow == NULL)
        return Function::invalidId();
    return m_currentShow->id();
}

void ShowManager::setCurrentShowID(int currentShowID)
{
    if (m_currentShow != NULL)
    {
        if (m_currentShow->id() == (quint32)currentShowID)
            return;
        disconnect(m_currentShow, SIGNAL(timeChanged(quint32)), this, SLOT(slotTimeChanged(quint32)));
    }

    m_currentShow = qobject_cast<Show*>(m_doc->function(currentShowID));
    emit currentShowIDChanged(currentShowID);
    if (m_currentShow != NULL)
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
    if (m_currentShow == NULL)
        return QString();

    return m_currentShow->name();
}

void ShowManager::setShowName(QString showName)
{
    if (m_currentShow == NULL || m_currentShow->name() == showName)
        return;

    Tardis::instance()->enqueueAction(Tardis::FunctionSetName, m_currentShow->id(), m_currentShow->name(), showName);

    m_currentShow->setName(showName);
    emit showNameChanged(showName);
}

QQmlListProperty<Track> ShowManager::tracks()
{
    m_tracksList.clear();
    if (m_currentShow)
        m_tracksList = m_currentShow->tracks();

    return QQmlListProperty<Track>(this, m_tracksList);
}

int ShowManager::selectedTrack() const
{
    return m_selectedTrack;
}

void ShowManager::setSelectedTrack(int selectedTrack)
{
    if (m_selectedTrack == selectedTrack)
        return;

    m_selectedTrack = selectedTrack;
    emit selectedTrackChanged(selectedTrack);
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
    emit timeScaleChanged(timeScale);
}

float ShowManager::tickSize() const
{
    return m_tickSize;
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
  * Show Items
  ********************************************************************/

void ShowManager::addItems(QQuickItem *parent, int trackIdx, int startTime, QVariantList idsList)
{
    if (idsList.count() == 0)
        return;

    // if no show is selected, then create a new one
    if (m_currentShow == NULL)
    {
        QString defaultName = QString("%1 %2").arg(tr("New Show")).arg(m_doc->nextFunctionID());
        m_currentShow = new Show(m_doc);
        m_currentShow->setName(defaultName);
        Function *f = qobject_cast<Function*>(m_currentShow);
        if (m_doc->addFunction(f) == false)
        {
            qDebug() << "Error in creating a new Show!";
            m_currentShow = NULL;
            return;
        }
        connect(m_currentShow,SIGNAL(timeChanged(quint32)), this, SLOT(slotTimeChanged(quint32)));
        emit currentShowIDChanged(m_currentShow->id());
        emit showNameChanged(m_currentShow->name());
    }

    Track *selectedTrack = NULL;

    // if no Track index is provided, then add a new one
    if (trackIdx == -1)
    {
        selectedTrack = new Track();
        selectedTrack->setName(tr("Track %1").arg(m_currentShow->tracks().count() + 1));
        m_currentShow->addTrack(selectedTrack);
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

    for (QVariant vID : idsList) // C++11
    {
        quint32 functionID = vID.toUInt();
        if (functionID == m_currentShow->id())
        {
            /* TODO: a popup displaying the user stupidity would be nice here... */
            continue;
        }

        // and now create the actual ShowFunction and the QML item
        Function *func = m_doc->function(functionID);
        if (func == NULL)
            continue;

        ShowFunction *showFunc = selectedTrack->createShowFunction(functionID);
        showFunc->setStartTime(startTime);
        showFunc->setDuration(func->totalDuration() ? func->totalDuration() : 5000);
        showFunc->setColor(ShowFunction::defaultColor(func->type()));

        QQuickItem *newItem = qobject_cast<QQuickItem*>(siComponent->create());

        newItem->setParentItem(parent);
        newItem->setProperty("trackIndex", trackIdx);
        newItem->setProperty("sfRef", QVariant::fromValue(showFunc));
        newItem->setProperty("funcRef", QVariant::fromValue(func));

        quint32 itemIndex = m_itemsMap.isEmpty() ? 0 : m_itemsMap.lastKey() + 1;
        quint32 itemID = trackIdx << 16 | itemIndex;
        m_itemsMap[itemID] = newItem;
        startTime += showFunc->duration();
    }

    emit showDurationChanged(m_currentShow->totalDuration());
}

void ShowManager::deleteShowItems(QVariantList data)
{
    Q_UNUSED(data)

    if (m_currentShow == NULL)
        return;

    foreach(SelectedShowItem ssi, m_selectedItems)
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
        track->removeShowFunction(ssi.m_showFunc, true);
        if (ssi.m_item != NULL)
        {
            quint32 key = m_itemsMap.key(ssi.m_item, UINT_MAX);
            if (key != UINT_MAX)
                m_itemsMap.remove(key);
            delete ssi.m_item;
        }
    }

    m_selectedItems.clear();
    emit selectedItemsCountChanged(0);
}

bool ShowManager::checkAndMoveItem(ShowFunction *sf, int originalTrackIdx, int newTrackIdx, int newStartTime)
{
    if (m_currentShow == NULL || sf == NULL)
        return false;

    //qDebug() << Q_FUNC_INFO << "origIdx:" << originalTrackIdx << "newIdx:" << newTrackIdx << "time:" << newStartTime;

    Track *dstTrack = NULL;

    // check if it's moving on a new track or an existing one
    if (newTrackIdx >= m_currentShow->tracks().count())
    {
        // create a new track here
        dstTrack = new Track();
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

    if (m_gridEnabled)
    {
        // calculate the X position from time and time scale
        float xPos = ((float)newStartTime * m_tickSize) / (m_timeScale * 1000.0); // timescale * 1000 : tickSize = time : x
        // round to the nearest snap position
        xPos = qRound(xPos / m_tickSize) * m_tickSize;
        // recalculate the time from pixels
        float time = xPos * (1000 * m_timeScale) / m_tickSize; // xPos : time = tickSize : timescale * 1000
        sf->setStartTime(time);
    }
    else
    {
        sf->setStartTime(newStartTime);
    }

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

void ShowManager::resetContents()
{
    resetView();
    m_currentTime = 0;
    m_selectedTrack = -1;
    emit currentTimeChanged(m_currentTime);
    m_currentShow = NULL;
}

void ShowManager::resetView()
{
    QMapIterator<quint32, QQuickItem*> it(m_itemsMap);
    while(it.hasNext())
    {
        it.next();
        delete it.value();
    }
    m_itemsMap.clear();
}

void ShowManager::renderView(QQuickItem *parent)
{
    resetView();

    if (m_currentShow == NULL)
        return;

    setContextItem(parent);

    int trkIdx = 0;

    foreach(Track *track, m_currentShow->tracks())
    {
        int itemIndex = 0;

        foreach(ShowFunction *sf, track->showFunctions())
        {
            Function *func = m_doc->function(sf->functionID());
            if (func == NULL)
                continue;

            QQuickItem *newItem = qobject_cast<QQuickItem*>(siComponent->create());

            newItem->setParentItem(parent);
            newItem->setProperty("trackIndex", trkIdx);
            newItem->setProperty("sfRef", QVariant::fromValue(sf));
            newItem->setProperty("funcRef", QVariant::fromValue(func));

            quint32 itemID = trkIdx << 16 | itemIndex;
            m_itemsMap[itemID] = newItem;
            itemIndex++;
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
    if (m_currentShow == NULL)
        return 0;

    return m_currentShow->totalDuration();
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

void ShowManager::playShow()
{
    if (m_currentShow == NULL)
        return;

    m_currentShow->start(m_doc->masterTimer(), FunctionParent::master(), m_currentTime);
    emit isPlayingChanged(true);
}

void ShowManager::stopShow()
{
    if (m_currentShow != NULL && m_currentShow->isRunning())
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
    if (m_currentShow != NULL && m_currentShow->isRunning())
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
    foreach(SelectedShowItem ssi, m_selectedItems)
    {
        if (ssi.m_item != NULL)
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
        if (func != NULL)
            names.append(func->name());
    }

    return names;
}

bool ShowManager::selectedItemsLocked()
{
    foreach (SelectedShowItem si, m_selectedItems)
    {
        if (si.m_showFunc != NULL && si.m_showFunc->isLocked())
            return true;
    }
    return false;
}

void ShowManager::setSelectedItemsLock(bool lock)
{
    foreach (SelectedShowItem si, m_selectedItems)
    {
        if (si.m_showFunc != NULL)
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
    if (track == NULL)
        return false;

    foreach(ShowFunction *sf, track->showFunctions())
    {
        if (sf == sourceFunc)
            continue;

        Function *func = m_doc->function(sf->functionID());
        if (func != NULL)
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
    if (f == NULL)
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
        default:
            data.append(RepeatingDuration);
            data.append(f->totalDuration());
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
        if (func == NULL)
            continue;

        Function *copyFunc = func->createCopy(m_doc);
        if (copyFunc == NULL)
            continue;

        copyFunc->setName(QString("%1 %2").arg(copyFunc->name()).arg(tr("(Copy)")));

        if (copyFunc->type() == Function::SequenceType)
        {
            Sequence *sequence = qobject_cast<Sequence*>(copyFunc);
            Scene *scene = qobject_cast<Scene*>(m_doc->function(sequence->boundSceneID()));
            if (scene == NULL)
                continue;

            Scene *copyScene = static_cast<Scene*>(scene->createCopy(m_doc, true));
            if (copyScene == NULL)
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




