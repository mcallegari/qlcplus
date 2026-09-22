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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QQmlContext>
#include <QtCore/QBuffer>
#include <QSettings>
#include <QtMath>
#include <QVector>
#include <algorithm>

#include "waveformimageprovider.h"
#include "showmanager.h"
#include "sequence.h"
#include "tardis.h"
#include "chaser.h"
#include "scene.h"
#include "track.h"
#include "show.h"
#include "doc.h"
#include "app.h"

#define SETTINGS_SNAP_TO_ITEMS QStringLiteral("showmanager/snaptoitems")
#define KXMLQLCShowManagerCurrentShow QStringLiteral("CurrentShow")
#define KXMLQLCShowManagerTimeScale   QStringLiteral("TimeScale")

/* Timeline zoom limits. Every item position and size is computed by
   dividing by the time scale, so it must never reach zero or go
   negative: that would produce infinite, NaN or negative geometry */
#define SHOWMGR_MIN_TIME_SCALE 0.1f
#define SHOWMGR_MAX_TIME_SCALE 100.0f

ShowManager::ShowManager(QQuickView *view, Doc *doc, QObject *parent)
    : PreviewContext(view, doc, "SHOWMGR", parent)
    , m_cursorMovedDuringPause(false)
    , m_isPlaying(false)
    , m_isPaused(false)
    , m_currentShow(nullptr)
    , m_stretchFunctions(false)
    , m_gridEnabled(false)
    , m_snapToItems(true)
    , m_snapGuideX(-1.0)
    , m_timeScale(5.0)
    , m_currentTime(0)
    , m_selectedTrackId(-1)
    , m_itemsColor(Qt::gray)
    , m_multipleSelection(false)
    , m_groupDragActive(false)
    , m_boxSelectMode(false)
    , m_clipboardIsCut(false)
{
    QSettings settings;
    QVariant snap = settings.value(SETTINGS_SNAP_TO_ITEMS);
    if (snap.isValid())
        m_snapToItems = snap.toBool();

    view->rootContext()->setContextProperty("showManager", this);
    qmlRegisterUncreatableType<Show>("org.qlcplus.classes", 1, 0, "Show", "Can't create a Show");
    qmlRegisterType<Track>("org.qlcplus.classes", 1, 0, "Track");
    qmlRegisterUncreatableType<ShowFunction>("org.qlcplus.classes", 1, 0, "ShowFunction", "Can't create a ShowFunction");

    /* Create and register a Waveform image provider */
    m_waveformProvider = new WaveformImageProvider(doc);
    view->engine()->addImageProvider(QLatin1String("waveform"), m_waveformProvider);
    view->rootContext()->setContextProperty("waveformProvider", m_waveformProvider);

    /* Relay Function changes to the UI, so Show Items can update
       their preview lines when the referenced Function is edited */
    connect(m_doc, SIGNAL(functionChanged(quint32)),
            this, SIGNAL(functionChanged(quint32)));

    /* Close the Show being edited if it gets deleted */
    connect(m_doc, SIGNAL(functionRemoved(quint32)),
            this, SLOT(slotFunctionRemoved(quint32)));

    setContextResource("qrc:/ShowManager.qml");
    setContextTitle(tr("Show Manager"));
}

void ShowManager::initialize()
{
    App *app = qobject_cast<App *>(m_view);
    m_tickSize = app->pixelDensity() * 18;

    if (m_waveformProvider)
        m_waveformProvider->setPixelDensity(app->pixelDensity());

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

bool ShowManager::isEditing() const
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
        disconnect(m_currentShow, SIGNAL(showFinished()), this, SLOT(slotShowFinished()));
        disconnect(m_currentShow, SIGNAL(stopped(quint32)), this, SLOT(slotShowStopped()));
    }

    m_currentShow = qobject_cast<Show*>(m_doc->function(currentShowID));
    m_cursorMovedDuringPause = false;
    emit currentShowIDChanged(currentShowID);
    emit isEditingChanged();

    if (m_currentShow != nullptr)
    {
        connect(m_currentShow, SIGNAL(timeChanged(quint32)), this, SLOT(slotTimeChanged(quint32)));
        connect(m_currentShow, SIGNAL(showFinished()), this, SLOT(slotShowFinished()));
        connect(m_currentShow, SIGNAL(stopped(quint32)), this, SLOT(slotShowStopped()));
        emit showDurationChanged(m_currentShow->totalDuration());
        emit showNameChanged(m_currentShow->name());
    }
    else
    {
        emit showDurationChanged(0);
        emit showNameChanged("");
    }

    /* Emit time/beat change in case the new Show differs */
    emit timeDivisionChanged(timeDivision());
    emit tempoSectionsChanged();
    emit beatsDivisionChanged(beatsDivision());
    m_timeScale = 0.0; // force setTimeScale() to recompute and notify
    setTimeScale(timeDivision() == Show::Time ? 5.0 : 1.0);

    emit tracksChanged();
    setPlaybackState(m_currentShow != nullptr ? m_currentShow->isRunning() : false,
                     m_currentShow != nullptr ? m_currentShow->isPaused() : false);
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

bool ShowManager::snapToItems() const
{
    return m_snapToItems;
}

void ShowManager::setSnapToItems(bool snapToItems)
{
    if (m_snapToItems == snapToItems)
        return;

    m_snapToItems = snapToItems;

    QSettings settings;
    settings.setValue(SETTINGS_SNAP_TO_ITEMS, m_snapToItems);

    emit snapToItemsChanged(m_snapToItems);
}

double ShowManager::snapGuideX() const
{
    return m_snapGuideX;
}

void ShowManager::setSnapGuideX(double snapGuideX)
{
    if (qFuzzyCompare(m_snapGuideX, snapGuideX))
        return;

    m_snapGuideX = snapGuideX;
    emit snapGuideXChanged();
}

QVariantList ShowManager::getSnapEdges(quint32 excludeItemId,
                                       double viewportLeft, double viewportRight) const
{
    QVariantList edges;

    if (m_currentShow == nullptr)
        return edges;

    int beatsDivision = m_currentShow->beatsDivision();
    int bpm = m_doc->inputOutputMap()->bpmNumber();

    for (Track *track : m_currentShow->tracks())
    {
        for (ShowFunction *sf : track->showFunctions())
        {
            if (sf->id() == excludeItemId)
                continue;

            // an item's times are in its Function's own unit (ms or beats as ms),
            // so convert them the same way ShowItem.qml updateGeometry() does
            Function *func = m_doc->function(sf->functionID());
            bool itemIsBeats = itemInBeats(func);
            double startTime = sf->startTime();
            double endTime = startTime + sf->duration();
            double startX, endX;

            if (timeDivision() == Show::Time)
            {
                if (itemIsBeats)
                {
                    // beats as ms -> real ms
                    double beatMs = bpm > 0 ? 60000.0 / bpm : 0;
                    startTime = (startTime / 1000.0) * beatMs;
                    endTime = (endTime / 1000.0) * beatMs;
                }
                startX = (startTime * m_tickSize) / (m_timeScale * 1000.0);
                endX = (endTime * m_tickSize) / (m_timeScale * 1000.0);
            }
            else if (itemIsBeats)
            {
                startX = (m_tickSize / beatsDivision) * (startTime / 1000.0);
                endX = (m_tickSize / beatsDivision) * (endTime / 1000.0);
            }
            else
            {
                // real ms -> position on the bar-based ruler
                double barDuration = bpm > 0 ? (60000.0 / bpm) * beatsDivision : 0;
                startX = barDuration > 0 ? (m_tickSize * startTime) / barDuration : 0;
                endX = barDuration > 0 ? (m_tickSize * endTime) / barDuration : 0;
            }

            // filter: skip items entirely outside the visible viewport
            if (viewportLeft >= 0 && viewportRight >= 0)
            {
                if (endX < viewportLeft || startX > viewportRight)
                    continue;
            }

            edges.append(startX);
            edges.append(endX);
        }
    }

    // the tempo section edges are snap targets too
    if (tempoGridActive())
    {
        for (const TempoSection &section : m_currentShow->tempoMap().sections())
        {
            double startX = timeToPosition(section.startTime);
            double endX = timeToPosition(section.endTime());

            if (viewportLeft >= 0 && viewportRight >= 0 &&
                (endX < viewportLeft || startX > viewportRight))
                continue;

            edges.append(startX);
            edges.append(endX);
        }
    }

    return edges;
}

/*********************************************************************
 * Time
 ********************************************************************/

Show::TimeDivision ShowManager::timeDivision() const
{
    if (m_currentShow == nullptr)
        return Show::Time;

    return m_currentShow->timeDivisionType();
}

bool ShowManager::hasBeatBasedItems() const
{
    if (m_currentShow == nullptr)
        return false;

    // nothing is snapped on a BPM ruler when all the items are in ms
    if (m_currentShow->itemsInMs())
        return false;

    foreach (Track *track, m_currentShow->tracks())
    {
        foreach (ShowFunction *sf, track->showFunctions())
        {
            Function *func = m_doc->function(sf->functionID());
            if (func != nullptr && func->tempoType() == Function::Beats)
                return true;
        }
    }

    return false;
}

void ShowManager::setTimeDivision(Show::TimeDivision division)
{
    if (m_currentShow == nullptr)
        return;

    if (division == m_currentShow->timeDivisionType())
        return;


    /* A beat tempo Function's items are always positioned in "beats as ms"
       (1000 units per beat) regardless of the Show's own timeline
       division, and are not affected by this switch. However, since they
       can be freely dragged/resized in pixels while the Show is showing a
       Time-based ruler, they may end up sitting at an arbitrary fractional
       beat position instead of on a beat. When the user switches to a BPM
       ruler, tidy those up by snapping them to the nearest whole beat (the
       user is warned about this beforehand, see hasBeatBasedItems()) */
    /* The time division is display only for a Show whose items are all
       positioned in ms, so nothing is snapped there */
    if (division != Show::Time && m_currentShow->timeDivisionType() == Show::Time &&
        m_currentShow->itemsInMs() == false)
    {
        foreach (Track *track, m_currentShow->tracks())
        {
            foreach (ShowFunction *sf, track->showFunctions())
            {
                Function *func = m_doc->function(sf->functionID());
                if (func == nullptr || func->tempoType() != Function::Beats)
                    continue;

                quint32 startBeats = qRound((double)sf->startTime() / 1000.0);
                quint32 durationBeats = qRound((double)sf->duration() / 1000.0);
                if (durationBeats == 0)
                    durationBeats = 1;

                sf->setStartTime(startBeats * 1000);
                sf->setDuration(durationBeats * 1000);
            }
        }
    }

    /* Set the division type first: setTimeScale needs it to
       calculate the tick size against the new time division */
    m_currentShow->setTimeDivisionType(division);

    /* Notify the new beats division before any geometry-related signal.
       setTimeScale emits tickSizeChanged/timeScaleChanged, which make the
       UI recalculate the items geometry right away. If the beats division
       is still the previous one, beat sizes are computed with a stale
       (possibly zero) divider, messing up the whole timeline preview */
    if (division != Show::Time)
        emit beatsDivisionChanged(m_currentShow->beatsDivision());

    if (division == Show::Time)
    {
        m_currentShow->setTempoType(Function::Time);
        setTimeScale(5.0);
    }
    else
    {
        m_currentShow->setTempoType(Function::Beats);
        setTimeScale(1.0);
    }
    emit timeDivisionChanged(division);
    // the tempo grid is shown on a Time ruler only
    emit tempoSectionsChanged();
}

int ShowManager::beatsDivision() const
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
    timeScale = qBound(SHOWMGR_MIN_TIME_SCALE, timeScale, SHOWMGR_MAX_TIME_SCALE);

    if (m_timeScale == timeScale)
        return;

    m_timeScale = timeScale;
    float tickScale = timeDivision() == Show::Time ? 1.0 : timeScale;

    if (m_detached)
    {
        m_tickSize = pixelDensity() * (18 * tickScale);
    }
    else
    {
        /* On shutdown the Doc is destroyed as a child of App, which happens
           once ~App() has already returned: m_view is no longer an App by
           then, so the cast fails. Nothing is on screen at that point, so
           the current tick size can simply be kept */
        App *app = qobject_cast<App *>(m_view);
        if (app != nullptr)
            m_tickSize = app->pixelDensity() * (18 * tickScale);
    }

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

    if (m_currentShow != nullptr && m_currentShow->isPaused())
        m_cursorMovedDuringPause = true;

    m_currentTime = currentTime;
    emit currentTimeChanged(currentTime);
}

/*********************************************************************
 * Tempo sections
 ********************************************************************/

/* The smallest distance in pixels between two tempo grid lines */
#define TEMPO_GRID_MIN_SPACING  8.0

QVariantList ShowManager::tempoSections() const
{
    QVariantList list;

    if (m_currentShow == nullptr)
        return list;

    const QList<TempoSection> &sections = m_currentShow->tempoMap().sections();
    for (int i = 0; i < sections.count(); i++)
    {
        const TempoSection &section = sections.at(i);
        QVariantMap map;
        map.insert("index", i);
        map.insert("startTime", section.startTime);
        map.insert("duration", section.duration);
        map.insert("bpm", section.bpm);
        map.insert("beatsPerBar", section.beatsPerBar);
        map.insert("name", section.name);
        list.append(map);
    }

    return list;
}

bool ShowManager::itemsInMs() const
{
    return m_currentShow != nullptr && m_currentShow->itemsInMs();
}

bool ShowManager::tempoGridActive() const
{
    return m_currentShow != nullptr && timeDivision() == Show::Time &&
           m_currentShow->tempoMap().isEmpty() == false;
}

QByteArray ShowManager::tempoStateToByteArray(const Show *show)
{
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);
    show->saveXMLTempoMap(&xmlWriter);
    xmlWriter.writeEndDocument();
    buffer.close();

    return data;
}

void ShowManager::restoreTempoState(quint32 showId, const QByteArray &state)
{
    Show *show = qobject_cast<Show*>(m_doc->function(showId));
    if (show == nullptr)
        return;

    TempoMap map;
    bool itemsInMs = false;

    // an empty state is a Show whose items were never converted to ms
    if (state.isEmpty() == false)
    {
        QBuffer buffer;
        buffer.setData(state);
        buffer.open(QIODevice::ReadOnly | QIODevice::Text);
        QXmlStreamReader xmlReader(&buffer);
        xmlReader.readNextStartElement();
        itemsInMs = xmlReader.attributes().value(KXMLQLCTempoMapItemUnit) == KXMLQLCTempoMapItemUnitMs;
        map.loadXML(xmlReader);
    }

    show->restoreTempoMap(map, itemsInMs);

    if (show == m_currentShow)
    {
        emit tempoSectionsChanged();
        emit showDurationChanged(m_currentShow->totalDuration());
    }
}

void ShowManager::setShowItemFunction(quint32 itemId, quint32 functionId)
{
    if (m_currentShow == nullptr)
        return;

    ShowFunction *sf = m_currentShow->showFunction(itemId);
    Function *func = m_doc->function(functionId);
    if (sf == nullptr || func == nullptr)
        return;

    sf->setFunctionID(functionId);

    QQuickItem *item = m_itemsMap.value(itemId, nullptr);
    if (item != nullptr)
        item->setProperty("funcRef", QVariant::fromValue(func));
}

void ShowManager::setTempoMap(const TempoMap &tempoMap)
{
    QByteArray oldState = tempoStateToByteArray(m_currentShow);

    // the first section converts the Beats tempo items to ms: keep their
    // times, to record the conversion as part of the same undo step
    QMap<ShowFunction *, QPair<quint32, quint32>> beatItems;
    if (m_currentShow->itemsInMs() == false)
    {
        for (Track *track : m_currentShow->tracks())
        {
            for (ShowFunction *sf : track->showFunctions())
            {
                Function *func = m_doc->function(sf->functionID());
                if (func != nullptr && func->tempoType() == Function::Beats)
                    beatItems.insert(sf, qMakePair(sf->startTime(), sf->duration()));
            }
        }
    }

    m_currentShow->setTempoMap(tempoMap);

    for (auto it = beatItems.constBegin(); it != beatItems.constEnd(); ++it)
    {
        ShowFunction *sf = it.key();
        if (sf->startTime() != it.value().first)
            Tardis::instance()->enqueueAction(Tardis::ShowManagerItemSetStartTime, sf->id(),
                                              it.value().first, sf->startTime());
        if (sf->duration() != it.value().second)
            Tardis::instance()->enqueueAction(Tardis::ShowManagerItemSetDuration, sf->id(),
                                              it.value().second, sf->duration());
    }

    Tardis::instance()->enqueueAction(Tardis::ShowManagerSetTempoMap, m_currentShow->id(),
                                      oldState, tempoStateToByteArray(m_currentShow));

    emit tempoSectionsChanged();
    emit showDurationChanged(m_currentShow->totalDuration());
}

int ShowManager::addTempoSection(int time)
{
    if (m_currentShow == nullptr || timeDivision() != Show::Time || time < 0)
        return -1;

    TempoMap map = m_currentShow->tempoMap();
    if (map.sectionIndexAt(time) != -1)
        return -1;

    // up to the next section, or a minute
    quint32 duration = 60000;
    for (const TempoSection &section : map.sections())
    {
        if (section.startTime > (quint32)time)
        {
            duration = qMin(duration, section.startTime - (quint32)time);
            break;
        }
    }

    int index = map.addSection(TempoSection(time, duration, 120.0, 4, tr("Section %1").arg(map.count() + 1)));
    if (index == -1)
        return -1;

    setTempoMap(map);
    return index;
}

QList<TempoSection> ShowManager::selectedAudioSections() const
{
    QList<TempoSection> sections;

    for (const SelectedShowItem &ssi : std::as_const(m_selectedItems))
    {
        if (ssi.m_showFunc.isNull())
            continue;

        Function *func = m_doc->function(ssi.m_showFunc->functionID());
        if (func == nullptr || func->type() != Function::AudioType)
            continue;

        sections.append(TempoSection(ssi.m_showFunc->startTime(), ssi.m_showFunc->duration(m_doc),
                                     120.0, 4, func->name()));
    }

    std::sort(sections.begin(), sections.end(),
              [](const TempoSection &a, const TempoSection &b) { return a.startTime < b.startTime; });

    return sections;
}

QVariantMap ShowManager::tempoSelectionInfo() const
{
    QVariantMap info;
    int audio = 0;
    int overlapping = 0;

    if (m_currentShow != nullptr)
    {
        const TempoMap &map = m_currentShow->tempoMap();
        for (const TempoSection &section : selectedAudioSections())
        {
            audio++;
            if (map.canPlace(section) == false)
                overlapping++;
        }
    }

    info.insert("audio", audio);
    info.insert("overlapping", overlapping);
    return info;
}

QVariantList ShowManager::addTempoSectionsFromSelection(bool startPrecedence)
{
    QVariantList indices;

    if (m_currentShow == nullptr || timeDivision() != Show::Time)
        return indices;

    TempoMap map = m_currentShow->tempoMap();
    QList<quint32> startTimes;

    for (const TempoSection &section : selectedAudioSections())
    {
        int index = startPrecedence ? map.insertSection(section) : map.addSection(section);
        if (index != -1)
            startTimes.append(section.startTime);
    }

    if (startTimes.isEmpty())
        return indices;

    setTempoMap(map);

    // the indices are known only once all the sections are in place
    for (quint32 startTime : startTimes)
        indices.append(map.sectionIndexAt(startTime));

    return indices;
}

bool ShowManager::updateTempoSection(int index, int startTime, int duration,
                                     double bpm, int beatsPerBar, QString name)
{
    if (m_currentShow == nullptr || startTime < 0 || duration <= 0)
        return false;

    TempoMap map = m_currentShow->tempoMap();
    if (map.updateSection(index, TempoSection(startTime, duration, bpm, beatsPerBar, name)) == false)
        return false;

    setTempoMap(map);
    return true;
}

bool ShowManager::splitTempoSection(int index, int time)
{
    if (m_currentShow == nullptr || time < 0)
        return false;

    TempoMap map = m_currentShow->tempoMap();
    TempoSection section = map.section(index);
    double beatMs = section.beatDuration();
    double splitTime = section.startTime + std::round((time - section.startTime) / beatMs) * beatMs;

    if (map.splitSection(index, qRound(splitTime)) == false)
        return false;

    setTempoMap(map);
    return true;
}

void ShowManager::removeTempoSection(int index)
{
    if (m_currentShow == nullptr)
        return;

    TempoMap map = m_currentShow->tempoMap();
    if (map.removeSection(index))
        setTempoMap(map);
}

double ShowManager::tempoBeatDuration(double time) const
{
    int bpm = m_doc->inputOutputMap()->bpmNumber();

    if (m_currentShow == nullptr)
        return 60000.0 / (bpm > 0 ? bpm : 120);

    return m_currentShow->tempoMap().beatDurationAt(time, bpm);
}

double ShowManager::timeToPosition(double time) const
{
    return (time * m_tickSize) / (m_timeScale * 1000.0);
}

double ShowManager::positionToTime(double xPos) const
{
    return m_tickSize > 0 ? (xPos * m_timeScale * 1000.0) / m_tickSize : 0;
}

double ShowManager::tempoGridStep(const TempoSection &section) const
{
    double beatWidth = timeToPosition(section.beatDuration());
    if (beatWidth <= 0)
        return 0;

    // quarter beats, half beats, beats, then bars and groups of bars
    const double steps[] = { 0.25, 0.5, 1.0, 1.0 * section.beatsPerBar,
                             4.0 * section.beatsPerBar, 16.0 * section.beatsPerBar };

    for (double step : steps)
    {
        if (step * beatWidth >= TEMPO_GRID_MIN_SPACING)
            return step;
    }

    return steps[5];
}

QVariantList ShowManager::tempoGridLines(double fromX, double toX) const
{
    QVariantList lines;

    if (tempoGridActive() == false || m_tickSize <= 0)
        return lines;

    double fromTime = positionToTime(fromX);
    double toTime = positionToTime(toX);

    for (const TempoSection &section : m_currentShow->tempoMap().sections())
    {
        if (section.endTime() < fromTime || section.startTime > toTime)
            continue;

        double step = tempoGridStep(section);
        double beatMs = section.beatDuration();
        double stepMs = step * beatMs;
        if (stepMs <= 0)
            continue;

        double first = qMax(0.0, std::ceil((fromTime - section.startTime) / stepMs));
        for (double n = first; ; n++)
        {
            double time = section.startTime + (n * stepMs);
            if (time >= section.endTime() || time > toTime)
                break;

            double beat = n * step;
            int weight = 0;
            int bar = 0;
            double barPos = beat / section.beatsPerBar;

            if (qAbs(barPos - std::round(barPos)) < 0.001)
            {
                weight = 2;
                bar = qRound(barPos) + 1;
            }
            else if (qAbs(beat - std::round(beat)) < 0.001)
            {
                weight = 1;
            }

            lines.append(timeToPosition(time));
            lines.append(weight);
            lines.append(bar);
        }
    }

    return lines;
}

double ShowManager::snapToTempoGrid(double xPos, double fallbackStep) const
{
    if (tempoGridActive())
    {
        double time = positionToTime(xPos);
        const TempoMap &map = m_currentShow->tempoMap();
        int index = map.sectionIndexAt(time);

        if (index >= 0)
        {
            TempoSection section = map.section(index);
            double stepMs = tempoGridStep(section) * section.beatDuration();
            if (stepMs > 0)
            {
                double snapped = section.startTime + std::round((time - section.startTime) / stepMs) * stepMs;
                return timeToPosition(qMin(snapped, (double)section.endTime()));
            }
        }
    }

    if (fallbackStep > 0)
        return std::round(xPos / fallbackStep) * fallbackStep;

    return xPos;
}

bool ShowManager::itemInBeats(const Function *func) const
{
    return func != nullptr && func->tempoType() == Function::Beats && itemsInMs() == false;
}

/*********************************************************************
 * Tracks
 ********************************************************************/

QVariant ShowManager::tracks() const
{
    if (m_currentShow)
        return QVariant::fromValue(m_currentShow->tracks());

    return QVariant();
}

int ShowManager::selectedTrackId() const
{
    return m_selectedTrackId;
}

void ShowManager::setSelectedTrackId(int id)
{
    if (m_selectedTrackId == id)
        return;

    m_selectedTrackId = id;
    emit selectedTrackIdChanged(id);
    emit itemClicked(App::TrackDragItem);
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

void ShowManager::selectTrackOfSelectedItems()
{
    if (m_currentShow == nullptr || m_selectedItems.isEmpty())
        return;

    QList<Track *> tracks = m_currentShow->tracks();
    int topIdx = -1;

    for (const SelectedShowItem &ssi : std::as_const(m_selectedItems))
    {
        if (ssi.m_showFunc == nullptr)
            continue;

        int idx = tracks.indexOf(m_currentShow->getTrackFromShowFunctionID(ssi.m_showFunc->id()));
        if (idx >= 0 && (topIdx < 0 || idx < topIdx))
            topIdx = idx;
    }

    if (topIdx < 0 || int(tracks.at(topIdx)->id()) == m_selectedTrackId)
        return;

    // unlike setSelectedTrackId(), the items keep the keyboard shortcuts
    // (e.g. Delete must remove the items, not the Track)
    m_selectedTrackId = tracks.at(topIdx)->id();
    emit selectedTrackIdChanged(m_selectedTrackId);
}

void ShowManager::deleteSelectedTrack()
{
    if (m_currentShow == nullptr)
        return;

    Track *track = m_currentShow->track(selectedTrackId());
    if (track == nullptr)
        return;

    qDebug() << "Deleting track" << track->id();

    // serialize the track (and its Functions) before removing it, as
    // the undo action needs to restore it from its XML representation
    Tardis::instance()->enqueueAction(
        Tardis::ShowManagerDeleteTrack, m_currentShow->id(),
        Tardis::instance()->actionToByteArray(Tardis::ShowManagerDeleteTrack, m_currentShow->id(), track->id()),
        QVariant());

    // removeTrack() destroys the track ShowFunctions too, so drop every
    // reference to them (items, selection, clipboard) beforehand
    QList <ShowFunction *> sfList = track->showFunctions();
    for (ShowFunction *sf : sfList)
        deleteShowItem(sf);

    m_currentShow->removeTrack(selectedTrackId());
    m_doc->setModified();

    // the deleted Track can't stay selected, or pasting would silently
    // fall back to the items own Tracks while the UI shows no selection
    m_selectedTrackId = -1;
    emit selectedTrackIdChanged(m_selectedTrackId);

    QQuickItem *itemsArea = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("showItemsArea"));
    renderView(itemsArea);

    emit tracksChanged();
}

/*********************************************************************
  * Show Items
  ********************************************************************/

void ShowManager::addItems(QQuickItem *parent, int trackIdx, int startTime, QVariantList idsList,
                           ShowFunction *sourceFunc)
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
        connect(m_currentShow, SIGNAL(showFinished()), this, SLOT(slotShowFinished()));
        connect(m_currentShow, SIGNAL(stopped(quint32)), this, SLOT(slotShowStopped()));
        emit currentShowIDChanged(m_currentShow->id());
        emit showNameChanged(m_currentShow->name());
        emit isEditingChanged();
        setPlaybackState(false, false);
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

        /* A Function keeps its own tempo type when dropped on a track: a
           Show can freely mix time-based and beat-based items regardless
           of its own timeline division, so dropping a Function here must
           not silently override a tempo type the user already chose for
           it in its own editor */
        if (func->tempoType() == Function::Time)
        {
            showFunc->setDuration(func->totalDuration() ? func->totalDuration() : 5000);
        }
        else
        {
            if (func->type() == Function::AudioType || func->type() == Function::VideoType)
                func->setTotalDuration(func->duration());
            showFunc->setDuration(func->totalDuration() ? func->totalDuration() : 4000);

            // with tempo sections the item is in ms: the Function beats last
            // as long as they do at the tempo where the item is dropped
            if (itemsInMs())
                showFunc->setDuration(qRound((showFunc->duration() / 1000.0) * tempoBeatDuration(startTime)));
        }

        /* startTime is the drop position translated by the caller using
           the Show's own ruler (Time or BPM), i.e. it is only guaranteed
           to be in the dropped Function's own unit when that Function's
           tempo type matches the Show's current division. Since a
           Function keeps its own tempo type regardless of the Show's
           division, convert it to that Function's unit when they differ,
           using the live BPM */
        quint32 itemStartTime = (quint32)startTime;
        bool showIsBeats = timeDivision() != Show::Time;
        bool funcIsBeats = itemInBeats(func);

        if (showIsBeats != funcIsBeats)
        {
            int bpm = m_doc->inputOutputMap()->bpmNumber();
            int beatDuration = bpm > 0 ? (60000 / bpm) : 500;

            itemStartTime = showIsBeats
                    ? Function::beatsToTime(itemStartTime, beatDuration)  // Show is BPM, Function is Time
                    : Function::timeToBeats(itemStartTime, beatDuration); // Show is Time, Function is Beats
        }

        showFunc->setStartTime(itemStartTime);
        showFunc->setColor(ShowFunction::defaultColor(func->type()));

        // when pasting, inherit the customized properties of the source item
        if (sourceFunc != nullptr)
        {
            /* sourceFunc->duration() is expressed in the unit of ITS OWN
               Function (which may not even be the same Function as the one
               being pasted here, in a mixed selection), so it needs the
               same unit conversion as startTime above, relative to the
               Function this ShowFunction actually wraps */
            quint32 pastedDuration = sourceFunc->duration();
            Function *sourceOwnerFunc = m_doc->function(sourceFunc->functionID());
            bool sourceIsBeats = (sourceOwnerFunc != nullptr) ?
                        itemInBeats(sourceOwnerFunc) : funcIsBeats;

            if (sourceIsBeats != funcIsBeats)
            {
                int bpm = m_doc->inputOutputMap()->bpmNumber();
                int beatDuration = bpm > 0 ? (60000 / bpm) : 500;

                pastedDuration = funcIsBeats
                        ? Function::timeToBeats(pastedDuration, beatDuration)
                        : Function::beatsToTime(pastedDuration, beatDuration);
            }

            showFunc->setDuration(pastedDuration);
            showFunc->setColor(sourceFunc->color());
            showFunc->setLocked(sourceFunc->isLocked());
        }

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
    if (m_currentShow == nullptr || sf == nullptr)
        return;

    // items are parented to the same item used by renderView()
    QQuickItem *parent = contextItem();
    if (parent == nullptr)
        return;

    Function *func = m_doc->function(sf->functionID());
    if (func == nullptr)
        return;

    // ShowItem places itself vertically by track *index*, not by track ID
    int trackIndex = m_currentShow->tracks().indexOf(m_currentShow->track(trackId));
    if (trackIndex < 0)
        return;

    QQuickItem *newItem = qobject_cast<QQuickItem*>(siComponent->create());

    newItem->setParentItem(parent);
    newItem->setProperty("trackIndex", trackIndex);
    newItem->setProperty("sfRef", QVariant::fromValue(sf));
    newItem->setProperty("funcRef", QVariant::fromValue(func));
    m_itemsMap[sf->id()] = newItem;
}

void ShowManager::deleteShowItems(QVariantList data)
{
    Q_UNUSED(data);

    if (m_currentShow == nullptr)
        return;

    int clipboardCount = m_clipboard.count();

    foreach (SelectedShowItem ssi, m_selectedItems)
    {
        quint32 trackIndex = ssi.m_trackIndex;
        qDebug() << "Selected item has track index:" << trackIndex;

        ShowFunction *showFunc = ssi.m_showFunc.data();
        if (showFunc == nullptr)
            continue;

        // drop any clipboard reference to the item being deleted to
        // avoid dangling pointers when pasting later
        for (int i = m_clipboard.count() - 1; i >= 0; i--)
        {
            if (m_clipboard.at(i).m_showFunc == showFunc)
                m_clipboard.removeAt(i);
        }

        if (trackIndex >= quint32(m_currentShow->tracks().count()))
            continue;

        Track *track = m_currentShow->tracks().at(trackIndex);
        quint32 sfId = showFunc->id();

        // serialize the item before removing it, as the undo action
        // needs to restore it from its XML representation
        Tardis::instance()->enqueueAction(
            Tardis::ShowManagerDeleteFunction, m_currentShow->id(),
            Tardis::instance()->actionToByteArray(Tardis::ShowManagerDeleteFunction, m_currentShow->id(), sfId),
            QVariant());

        track->removeShowFunction(showFunc, true);
        m_itemsMap.remove(sfId);
        if (ssi.m_item != nullptr)
            delete ssi.m_item.data();
    }

    m_selectedItems.clear();
    emit selectedItemsCountChanged(0);

    if (m_clipboard.count() != clipboardCount)
    {
        emit clipboardItemsCountChanged(m_clipboard.count());
        emit cutItemIdsChanged();
    }
}

void ShowManager::refreshView()
{
    if (contextItem() != nullptr)
        renderView(contextItem());

    emit tracksChanged();
    if (m_currentShow != nullptr)
        emit showDurationChanged(m_currentShow->totalDuration());
}

void ShowManager::deleteShowItem(ShowFunction *sf)
{
    if (sf == nullptr)
        return;

    // the caller deletes the ShowFunction right after this, so drop
    // every reference to it before it becomes dangling
    int selectedCount = m_selectedItems.count();
    for (int i = m_selectedItems.count() - 1; i >= 0; i--)
    {
        if (m_selectedItems.at(i).m_showFunc == sf)
            m_selectedItems.removeAt(i);
    }
    if (m_selectedItems.count() != selectedCount)
        emit selectedItemsCountChanged(m_selectedItems.count());

    int clipboardCount = m_clipboard.count();
    for (int i = m_clipboard.count() - 1; i >= 0; i--)
    {
        if (m_clipboard.at(i).m_showFunc == sf)
            m_clipboard.removeAt(i);
    }
    if (m_clipboard.count() != clipboardCount)
    {
        emit clipboardItemsCountChanged(m_clipboard.count());
        emit cutItemIdsChanged();
    }

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

        Tardis::instance()->enqueueAction(
            Tardis::ShowManagerAddTrack, m_currentShow->id(), QVariant(),
            Tardis::instance()->actionToByteArray(Tardis::ShowManagerAddTrack, m_currentShow->id(), dstTrack->id()));

        // the item is going to be moved on the newly created Track
        newTrackIdx = m_currentShow->tracks().count() - 1;
        emit tracksChanged();
    }
    else
    {
        dstTrack = m_currentShow->tracks().at(newTrackIdx);

        bool overlapping = checkOverlapping(dstTrack, sf, newStartTime, sf->duration());
        if (overlapping == true)
            return false;
    }

    Tardis::instance()->enqueueAction(Tardis::ShowManagerItemSetStartTime, sf->id(), sf->startTime(), newStartTime);
    sf->setStartTime(newStartTime);

    // check if we need to move the ShowFunction to a different Track
    if (newTrackIdx != originalTrackIdx)
    {
        Track *srcTrack = m_currentShow->tracks().at(originalTrackIdx);
        srcTrack->removeShowFunction(sf, false);
        dstTrack->addShowFunction(sf);

        Tardis::instance()->enqueueAction(Tardis::ShowManagerItemSetTrack, sf->id(),
                                          originalTrackIdx, newTrackIdx);

        // keep the selection in sync, as deleting items relies on it
        for (SelectedShowItem &ssi : m_selectedItems)
        {
            if (ssi.m_showFunc == sf)
                ssi.m_trackIndex = newTrackIdx;
        }
    }

    m_doc->setModified();

    return true;
}

bool ShowManager::moveShowItems(QVariantList items, QVariantList trackIndexes, QVariantList startTimes)
{
    if (m_currentShow == nullptr)
        return false;

    if (items.count() != trackIndexes.count() || items.count() != startTimes.count())
        return false;

    struct ItemMove
    {
        QQuickItem *item;
        ShowFunction *sf;
        Track *srcTrack;
        int srcTrackIdx;
        int dstTrackIdx;
        quint32 startTime;
    };

    QList<Track *> tracks = m_currentShow->tracks();
    QList<ItemMove> moves;
    QList<ShowFunction *> movingFuncs;

    for (int i = 0; i < items.count(); i++)
    {
        ItemMove move;
        move.item = items.at(i).value<QQuickItem *>();
        if (move.item == nullptr)
            return false;

        move.sf = move.item->property("sfRef").value<ShowFunction *>();
        move.srcTrack = move.sf ? m_currentShow->getTrackFromShowFunctionID(move.sf->id()) : nullptr;
        if (move.srcTrack == nullptr)
            return false;

        move.srcTrackIdx = tracks.indexOf(move.srcTrack);
        move.dstTrackIdx = trackIndexes.at(i).toInt();
        if (move.dstTrackIdx < 0 || move.dstTrackIdx >= tracks.count())
            return false;

        int startTime = startTimes.at(i).toInt();
        move.startTime = startTime < 0 ? 0 : quint32(startTime);

        moves.append(move);
        movingFuncs.append(move.sf);
    }

    // check all the destinations before moving anything
    for (int i = 0; i < moves.count(); i++)
    {
        const ItemMove &move = moves.at(i);

        if (checkOverlapping(tracks.at(move.dstTrackIdx), movingFuncs, move.startTime, move.sf->duration()))
            return false;

        for (int j = i + 1; j < moves.count(); j++)
        {
            const ItemMove &other = moves.at(j);
            if (other.dstTrackIdx != move.dstTrackIdx)
                continue;

            if (move.startTime < other.startTime + other.sf->duration() &&
                other.startTime < move.startTime + move.sf->duration())
                return false;
        }
    }

    for (const ItemMove &move : moves)
    {
        if (move.sf->startTime() != move.startTime)
        {
            Tardis::instance()->enqueueAction(Tardis::ShowManagerItemSetStartTime, move.sf->id(),
                                              move.sf->startTime(), move.startTime);
            move.sf->setStartTime(move.startTime);
        }

        if (move.dstTrackIdx != move.srcTrackIdx)
        {
            move.srcTrack->removeShowFunction(move.sf, false);
            tracks.at(move.dstTrackIdx)->addShowFunction(move.sf);

            Tardis::instance()->enqueueAction(Tardis::ShowManagerItemSetTrack, move.sf->id(),
                                              move.srcTrackIdx, move.dstTrackIdx);

            move.item->setProperty("trackIndex", move.dstTrackIdx);

            for (SelectedShowItem &ssi : m_selectedItems)
            {
                if (ssi.m_showFunc == move.sf)
                    ssi.m_trackIndex = move.dstTrackIdx;
            }
        }
    }

    m_doc->setModified();

    return true;
}

int ShowManager::tracksCount() const
{
    if (m_currentShow == nullptr)
        return 0;

    return m_currentShow->tracks().count();
}

bool ShowManager::moveShowItemToTrack(ShowFunction *sf, int trackIdx)
{
    if (m_currentShow == nullptr || sf == nullptr)
        return false;

    if (trackIdx < 0 || trackIdx >= m_currentShow->tracks().count())
        return false;

    Track *dstTrack = m_currentShow->tracks().at(trackIdx);
    Track *srcTrack = m_currentShow->getTrackFromShowFunctionID(sf->id());

    if (dstTrack == nullptr || srcTrack == dstTrack)
        return false;

    if (srcTrack != nullptr)
        srcTrack->removeShowFunction(sf, false);

    dstTrack->addShowFunction(sf);

    m_doc->setModified();

    // the item didn't move through the UI, so the view has to be rebuilt
    refreshView();

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

bool ShowManager::setShowItemStartTimeAndDuration(ShowFunction *sf, int startTime, int duration)
{
    if (sf == nullptr)
        return false;

    Track *track = m_currentShow->getTrackFromShowFunctionID(sf->id());
    if (track == nullptr)
        return false;

    bool overlapping = checkOverlapping(track, sf, startTime, duration);
    if (overlapping)
        return false;

    Tardis::instance()->enqueueAction(Tardis::ShowManagerItemSetStartTime, sf->id(), sf->startTime(), startTime);
    sf->setStartTime(startTime);
    Tardis::instance()->enqueueAction(Tardis::ShowManagerItemSetDuration, sf->id(), sf->duration(), duration);
    sf->setDuration(duration);

    return true;
}

int ShowManager::minimumTimelineDuration(Show::TimeDivision division) const
{
    return division == Show::Time ? 1 : 125;
}

quint32 ShowManager::itemRelativeTimeFromCursor(const ShowFunction *sf, int cursorTime) const
{
    if (sf == nullptr)
        return 0;

    const quint32 currentTimeValue = quint32(qMax(0, cursorTime));
    if (currentTimeValue <= sf->startTime())
        return 0;

    return qMin(sf->duration(), currentTimeValue - sf->startTime());
}

quint32 ShowManager::mapCursorToChaserTime(const ShowFunction *sf, Chaser *chaser, int cursorTime) const
{
    if (sf == nullptr || chaser == nullptr)
        return 0;

    quint32 itemRelativeTime = itemRelativeTimeFromCursor(sf, cursorTime);
    quint32 chaserRelativeTime = itemRelativeTime;
    quint32 chaserTotal = chaser->totalDuration();
    if (sf->duration() > 0 && chaserTotal > 0)
    {
        chaserRelativeTime = quint32(qRound((double(itemRelativeTime) * double(chaserTotal))
                                            / double(sf->duration())));
    }

    return chaserRelativeTime;
}

quint32 ShowManager::chaserStepDuration(Chaser *chaser, int index) const
{
    if (chaser == nullptr || index < 0 || index >= chaser->stepsCount())
        return 0;

    if (chaser->durationMode() == Chaser::Common)
        return chaser->duration();

    ChaserStep *step = chaser->stepAt(index);
    return step ? step->duration : 0;
}

int ShowManager::chaserStepIndexFromTime(Chaser *chaser, quint32 timeValue) const
{
    if (chaser == nullptr || chaser->stepsCount() == 0)
        return -1;

    quint32 elapsed = 0;
    for (int i = 0; i < chaser->stepsCount(); ++i)
    {
        quint32 stepDuration = chaserStepDuration(chaser, i);
        if (stepDuration == 0)
            stepDuration = 1;

        if (timeValue < elapsed + stepDuration)
            return i;

        elapsed += stepDuration;
    }

    return chaser->stepsCount() - 1;
}

bool ShowManager::setChaserStepDurationWithUndo(Chaser *chaser, int stepIndex, quint32 newDuration)
{
    if (chaser == nullptr || stepIndex < 0 || stepIndex >= chaser->stepsCount())
        return false;

    ChaserStep *stepRef = chaser->stepAt(stepIndex);
    if (stepRef == nullptr)
        return false;

    ChaserStep step = *stepRef;
    newDuration = qMax(quint32(1), newDuration);
    if (step.duration == newDuration)
        return true;

    UIntPair oldDuration(stepIndex, step.duration);
    UIntPair oldHold(stepIndex, step.hold);

    step.duration = newDuration;
    step.hold = Function::speedSubtract(step.duration, step.fadeIn);

    Tardis::instance()->enqueueAction(Tardis::ChaserSetStepDuration, chaser->id(),
                                      QVariant::fromValue(oldDuration),
                                      QVariant::fromValue(UIntPair(stepIndex, step.duration)));
    Tardis::instance()->enqueueAction(Tardis::ChaserSetStepHold, chaser->id(),
                                      QVariant::fromValue(oldHold),
                                      QVariant::fromValue(UIntPair(stepIndex, step.hold)));
    chaser->replaceStep(step, stepIndex);
    return true;
}

void ShowManager::convertChaserCommonToPerStep(Chaser *chaser)
{
    if (chaser == nullptr || chaser->durationMode() != Chaser::Common)
        return;

    quint32 commonDuration = qMax(quint32(1), chaser->duration());
    chaser->setDurationMode(Chaser::PerStep);
    for (int i = 0; i < chaser->stepsCount(); ++i)
    {
        ChaserStep *stepRef = chaser->stepAt(i);
        if (stepRef == nullptr)
            continue;

        setChaserStepDurationWithUndo(chaser, i, commonDuration);
    }
}

void ShowManager::setShowItemDurationWithUndo(ShowFunction *sf, int newDuration)
{
    if (sf == nullptr)
        return;

    Tardis::instance()->enqueueAction(Tardis::ShowManagerItemSetDuration, sf->id(), sf->duration(), newDuration);
    sf->setDuration(newDuration);
}

bool ShowManager::moveAllItemsAfterCursor(int cursorTime, int delta)
{
    if (m_currentShow == nullptr || delta == 0)
        return true;

    QList<ShowFunction *> itemsToMove;
    foreach (Track *track, m_currentShow->tracks())
    {
        foreach (ShowFunction *sf, track->showFunctions())
        {
            if (sf == nullptr)
                continue;

            if (int(sf->startTime()) <= cursorTime)
                continue;

            itemsToMove.append(sf);
        }
    }

    std::sort(itemsToMove.begin(), itemsToMove.end(),
              [delta](ShowFunction *a, ShowFunction *b)
              {
                  if (delta > 0)
                      return a->startTime() > b->startTime();
                  return a->startTime() < b->startTime();
              });

    for (ShowFunction *sf : itemsToMove)
    {
        int newStart = int(sf->startTime()) + delta;
        if (newStart < 0)
            newStart = 0;

        if (setShowItemStartTime(sf, newStart) == false)
            return false;
    }

    return true;
}

bool ShowManager::insertShowItemTime(ShowFunction *sf, int length)
{
    return insertShowItemTimeAt(sf, length, m_currentTime);
}

bool ShowManager::insertShowItemTimeAt(ShowFunction *sf, int length, int cursorTime)
{
    if (m_currentShow == nullptr || sf == nullptr || length <= 0)
        return false;

    Function *func = m_doc->function(sf->functionID());
    if (func == nullptr)
        return false;

    Track *track = m_currentShow->getTrackFromShowFunctionID(sf->id());
    if (track == nullptr)
        return false;

    int minDuration = minimumTimelineDuration(timeDivision());

    switch (func->type())
    {
        case Function::AudioType:
        case Function::VideoType:
        {
            if (func->runOrder() != Function::Loop)
                return false;
        }
        Q_FALLTHROUGH();
        case Function::SceneType:
        case Function::CollectionType:
        case Function::EFXType:
        case Function::RGBMatrixType:
        {
            int newDuration = sf->duration() + length;
            if (newDuration < minDuration)
                newDuration = minDuration;

            if (checkOverlapping(track, sf, sf->startTime(), newDuration))
                return false;

            setShowItemDurationWithUndo(sf, newDuration);
            m_doc->setModified();
            return true;
        }
        case Function::ChaserType:
        case Function::SequenceType:
        {
            Chaser *chaser = qobject_cast<Chaser *>(func);
            if (chaser == nullptr)
                return false;

            int stepsCount = chaser->stepsCount();
            if (stepsCount == 0 && func->type() != Function::SequenceType)
                return false;

            int newItemDuration = int(sf->duration()) + length;
            if (newItemDuration < minDuration)
                newItemDuration = minDuration;
            if (checkOverlapping(track, sf, sf->startTime(), newItemDuration))
                return false;

            quint32 chaserRelativeTime = mapCursorToChaserTime(sf, chaser, cursorTime);

            int insertIndex = chaserStepIndexFromTime(chaser, chaserRelativeTime);
            if (insertIndex < 0)
                return false;

            // In Common mode, all steps share one duration: switch to PerStep first
            // so we can stretch only the step covering the cursor.
            convertChaserCommonToPerStep(chaser);

            ChaserStep *targetStepRef = chaser->stepAt(insertIndex);
            if (targetStepRef == nullptr)
                return false;

            quint32 targetDuration = targetStepRef->duration + quint32(length);
            if (setChaserStepDurationWithUndo(chaser, insertIndex, targetDuration) == false)
                return false;

            setShowItemDurationWithUndo(sf, newItemDuration);
            m_doc->setModified();
            return true;
        }
        default:
        break;
    }

    return false;
}

bool ShowManager::cutShowItemTime(ShowFunction *sf, int length)
{
    return cutShowItemTimeAt(sf, length, m_currentTime);
}

bool ShowManager::cutShowItemTimeAt(ShowFunction *sf, int length, int cursorTime)
{
    if (m_currentShow == nullptr || sf == nullptr || length <= 0)
        return false;

    Function *func = m_doc->function(sf->functionID());
    if (func == nullptr)
        return false;

    int minDuration = minimumTimelineDuration(timeDivision());
    int maxCutDuration = int(sf->duration()) - minDuration;
    if (maxCutDuration <= 0)
        return false;

    int targetCutDuration = qMin(length, maxCutDuration);

    switch (func->type())
    {
        case Function::AudioType:
        case Function::VideoType:
        {
            if (func->runOrder() != Function::Loop)
                return false;
        }
        Q_FALLTHROUGH();
        case Function::SceneType:
        case Function::CollectionType:
        case Function::EFXType:
        case Function::RGBMatrixType:
        {
            int newDuration = int(sf->duration()) - targetCutDuration;
            if (newDuration < minDuration)
                newDuration = minDuration;

            setShowItemDurationWithUndo(sf, newDuration);
            m_doc->setModified();
            return true;
        }
        case Function::ChaserType:
        case Function::SequenceType:
        {
            Chaser *chaser = qobject_cast<Chaser *>(func);
            if (chaser == nullptr || chaser->stepsCount() == 0)
                return false;

            convertChaserCommonToPerStep(chaser);

            quint32 chaserRelativeTime = mapCursorToChaserTime(sf, chaser, cursorTime);

            int cutStartIndex = chaserStepIndexFromTime(chaser, chaserRelativeTime);
            if (cutStartIndex < 0)
                return false;

            quint32 stepStartTime = 0;
            for (int i = 0; i < cutStartIndex; ++i)
                stepStartTime += qMax(quint32(1), chaserStepDuration(chaser, i));

            int cutRemaining = targetCutDuration;
            int cutDuration = 0;
            int stepIndex = cutStartIndex;
            quint32 cursorOffset = chaserRelativeTime > stepStartTime ? (chaserRelativeTime - stepStartTime) : 0;

            while (cutRemaining > 0 && stepIndex < chaser->stepsCount())
            {
                ChaserStep *stepRef = chaser->stepAt(stepIndex);
                if (stepRef == nullptr)
                    break;

                ChaserStep step = *stepRef;
                quint32 stepDuration = qMax(quint32(1), step.duration);
                quint32 offset = qMin(cursorOffset, stepDuration);
                int removable = (stepIndex == cutStartIndex) ? int(stepDuration - offset) : int(stepDuration);
                if (removable <= 0)
                {
                    cursorOffset = 0;
                    stepIndex++;
                    continue;
                }

                int consume = qMin(cutRemaining, removable);

                if (stepIndex == cutStartIndex && offset > 0)
                {
                    quint32 newStepDuration = stepDuration;
                    if (consume < removable)
                        newStepDuration = qMax(quint32(1), quint32(int(stepDuration) - consume));
                    else
                        newStepDuration = qMax(quint32(1), quint32(offset));
                    setChaserStepDurationWithUndo(chaser, stepIndex, newStepDuration);

                    cutDuration += consume;
                    cutRemaining -= consume;
                    cursorOffset = 0;
                    if (consume < removable)
                        break;
                    stepIndex++;
                    continue;
                }

                if (consume < removable)
                {
                    quint32 newStepDuration = qMax(quint32(1), quint32(int(stepDuration) - consume));
                    setChaserStepDurationWithUndo(chaser, stepIndex, newStepDuration);

                    cutDuration += consume;
                    cutRemaining = 0;
                    break;
                }

                if (chaser->stepsCount() <= 1)
                {
                    quint32 newStepDuration = 1;
                    int actualConsume = int(stepDuration - newStepDuration);
                    if (actualConsume <= 0)
                        break;

                    setChaserStepDurationWithUndo(chaser, stepIndex, newStepDuration);

                    cutDuration += actualConsume;
                    cutRemaining -= actualConsume;
                    break;
                }

                Tardis::instance()->enqueueAction(Tardis::ChaserRemoveStep, chaser->id(),
                                                  Tardis::instance()->actionToByteArray(Tardis::ChaserRemoveStep,
                                                                                        chaser->id(), stepIndex),
                                                  QVariant());
                if (chaser->removeStep(stepIndex) == false)
                    break;

                cutDuration += consume;
                cutRemaining -= consume;
            }

            if (cutDuration <= 0)
                return false;

            int newDuration = int(sf->duration()) - cutDuration;
            if (newDuration < minDuration)
                newDuration = minDuration;

            setShowItemDurationWithUndo(sf, newDuration);
            m_doc->setModified();
            return true;
        }
        default:
        break;
    }

    return false;
}

bool ShowManager::insertTimeAtCursor(int length, int cursorTime)
{
    if (m_currentShow == nullptr || length <= 0)
        return false;

    bool hasTarget = false;
    bool hasItemsAfterCursor = false;
    foreach (Track *track, m_currentShow->tracks())
    {
        foreach (ShowFunction *sf, track->showFunctions())
        {
            if (sf == nullptr || sf->isLocked())
                continue;

            int startTime = int(sf->startTime());
            if (startTime > cursorTime)
                hasItemsAfterCursor = true;

            int endTime = startTime + int(sf->duration());
            if (cursorTime < startTime || cursorTime > endTime)
                continue;

            hasTarget = true;
            if (hasItemsAfterCursor)
                break;
        }

        if (hasTarget && hasItemsAfterCursor)
            break;
    }

    if (!hasTarget && !hasItemsAfterCursor)
        return false;

    if (moveAllItemsAfterCursor(cursorTime, length) == false)
        return false;

    bool changed = false;
    foreach (Track *track, m_currentShow->tracks())
    {
        foreach (ShowFunction *sf, track->showFunctions())
        {
            if (sf == nullptr || sf->isLocked())
                continue;

            int startTime = int(sf->startTime());
            int endTime = startTime + int(sf->duration());
            if (cursorTime < startTime || cursorTime > endTime)
                continue;

            changed |= insertShowItemTimeAt(sf, length, cursorTime);
        }
    }

    if (hasTarget && !hasItemsAfterCursor && !changed)
        moveAllItemsAfterCursor(cursorTime, -length);

    return changed || hasItemsAfterCursor;
}

bool ShowManager::cutTimeAtCursor(int length, int cursorTime)
{
    if (m_currentShow == nullptr || length <= 0)
        return false;

    bool changed = false;
    foreach (Track *track, m_currentShow->tracks())
    {
        foreach (ShowFunction *sf, track->showFunctions())
        {
            if (sf == nullptr || sf->isLocked())
                continue;

            int startTime = int(sf->startTime());
            int endTime = startTime + int(sf->duration());
            if (cursorTime < startTime || cursorTime > endTime)
                continue;

            changed |= cutShowItemTimeAt(sf, length, cursorTime);
        }
    }

    if (!changed)
        return false;

    moveAllItemsAfterCursor(cursorTime, -length);

    return changed;
}

void ShowManager::resetContents()
{
    resetView();
    m_currentTime = 0;
    emit currentTimeChanged(m_currentTime);

    m_selectedTrackId = -1;
    emit selectedTrackIdChanged(m_selectedTrackId);
    m_cursorMovedDuringPause = false;

    if (m_currentShow != nullptr)
    {
        disconnect(m_currentShow, SIGNAL(timeChanged(quint32)), this, SLOT(slotTimeChanged(quint32)));
        disconnect(m_currentShow, SIGNAL(showFinished()), this, SLOT(slotShowFinished()));
        disconnect(m_currentShow, SIGNAL(stopped(quint32)), this, SLOT(slotShowStopped()));
    }

    m_currentShow = nullptr;
    emit currentShowIDChanged(Function::invalidId());
    emit showNameChanged(QString());
    emit showDurationChanged(0);
    emit timeDivisionChanged(Show::Time);
    emit tempoSectionsChanged();

    m_timeScale = 0.0; // force setTimeScale() to recompute and notify
    setTimeScale(5.0);

    // the clipboard holds ShowFunction pointers belonging to the show
    // being closed, so drop them to avoid dangling references
    // (the selection is already cleared by resetView() above)
    if (m_clipboard.isEmpty() == false)
    {
        m_clipboard.clear();
        emit clipboardItemsCountChanged(0);
    }
    clearCutState();

    emit tracksChanged();
    emit isEditingChanged();
    setPlaybackState(false, false);
}

void ShowManager::resetView()
{
    // the selection references the items about to be destroyed,
    // so clear it before deleting anything
    if (m_selectedItems.isEmpty() == false)
    {
        m_selectedItems.clear();
        emit selectedItemsCountChanged(0);
    }

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

    if (m_currentShow->isRunning() == false)
    {
        m_cursorMovedDuringPause = false;
        m_currentShow->start(m_doc->masterTimer(), FunctionParent::master(), m_currentTime);
        setPlaybackState(true, false);
        return;
    }

    if (m_currentShow->isPaused())
    {
        if (m_cursorMovedDuringPause)
        {
            m_currentShow->stop(FunctionParent::master());
            m_currentShow->stopAndWait();
            m_cursorMovedDuringPause = false;
            m_currentShow->start(m_doc->masterTimer(), FunctionParent::master(), m_currentTime);
        }
        else
        {
            m_currentShow->setPause(false);
        }

        setPlaybackState(true, false);
        return;
    }

    m_currentShow->setPause(true);
    setPlaybackState(true, true);
}

void ShowManager::stopShow()
{
    if (m_currentShow != nullptr && m_currentShow->isRunning())
    {
        m_cursorMovedDuringPause = false;
        m_currentShow->stop(FunctionParent::master());
        setPlaybackState(false, false);
        return;
    }

    setPlaybackState(false, false);

    if (m_currentTime != 0)
    {
        m_currentTime = 0;
        emit currentTimeChanged(m_currentTime);
    }
}

bool ShowManager::isPlaying() const
{
    return m_isPlaying;
}

bool ShowManager::isPaused() const
{
    return m_isPaused;
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

int ShowManager::clipboardItemsCount() const
{
    return m_clipboard.count();
}

bool ShowManager::multipleSelection() const
{
    return m_multipleSelection;
}

void ShowManager::setMultipleSelection(bool multipleSelection)
{
    if (m_multipleSelection == multipleSelection)
        return;

    m_multipleSelection = multipleSelection;
    emit multipleSelectionChanged();
}

bool ShowManager::boxSelectMode() const
{
    return m_boxSelectMode;
}

void ShowManager::setBoxSelectMode(bool enable)
{
    if (m_boxSelectMode == enable)
        return;

    m_boxSelectMode = enable;
    emit boxSelectModeChanged();
}

void ShowManager::setItemSelection(int trackIdx, ShowFunction *sf, QQuickItem *item, bool selected, int keyModifiers)
{
    bool allowMulti = m_multipleSelection
            || (keyModifiers & Qt::ControlModifier)
            || (keyModifiers & Qt::ShiftModifier);
    bool changed = false;

    if (selected == true)
    {
        if (!allowMulti)
        {
            for (int i = m_selectedItems.count() - 1; i >= 0; --i)
            {
                SelectedShowItem si = m_selectedItems.at(i);
                if (si.m_showFunc == sf)
                    continue;
                if (si.m_item != nullptr)
                    si.m_item->setProperty("isSelected", false);
                m_selectedItems.removeAt(i);
                changed = true;
            }
        }

        bool alreadySelected = false;
        foreach (SelectedShowItem si, m_selectedItems)
        {
            if (si.m_showFunc == sf)
            {
                alreadySelected = true;
                break;
            }
        }

        if (!alreadySelected)
        {
            SelectedShowItem selection;
            selection.m_trackIndex = trackIdx;
            selection.m_showFunc = sf;
            selection.m_item = item;
            m_selectedItems.append(selection);
            changed = true;
        }
    }
    else
    {
        for (int i = 0; i < m_selectedItems.count(); i++)
        {
            SelectedShowItem si = m_selectedItems.at(i);
            if (si.m_showFunc == sf)
            {
                m_selectedItems.removeAt(i);
                changed = true;
                break;
            }
        }
    }
    if (changed)
    {
        selectTrackOfSelectedItems();
        emit selectedItemsCountChanged(m_selectedItems.count());
    }
    emit itemClicked(App::ShowDragItem);
}

void ShowManager::resetItemsSelection()
{
    foreach (SelectedShowItem ssi, m_selectedItems)
    {
        if (ssi.m_item != nullptr)
            ssi.m_item->setProperty("isSelected", false);
    }
    m_selectedItems.clear();
    emit selectedItemsCountChanged(m_selectedItems.count());
}

bool ShowManager::selectAllTrackItems()
{
    if (m_currentShow == nullptr)
        return false;

    QList<Track *> tracks = m_currentShow->tracks();
    int trackIdx = -1;

    for (int i = 0; i < tracks.count(); i++)
    {
        if (int(tracks.at(i)->id()) == m_selectedTrackId)
        {
            trackIdx = i;
            break;
        }
    }

    // no Track selected: use the one of the last selected item
    if (trackIdx < 0 && m_selectedItems.isEmpty() == false)
        trackIdx = m_selectedItems.last().m_trackIndex;

    if (trackIdx < 0 || trackIdx >= tracks.count())
        return false;

    foreach (SelectedShowItem ssi, m_selectedItems)
    {
        if (ssi.m_item != nullptr)
            ssi.m_item->setProperty("isSelected", false);
    }
    m_selectedItems.clear();

    foreach (ShowFunction *sf, tracks.at(trackIdx)->showFunctions())
    {
        QQuickItem *item = m_itemsMap.value(sf->id(), nullptr);
        if (item == nullptr)
            continue;

        item->setProperty("isSelected", true);

        SelectedShowItem selection;
        selection.m_trackIndex = trackIdx;
        selection.m_showFunc = sf;
        selection.m_item = item;
        m_selectedItems.append(selection);
    }

    selectTrackOfSelectedItems();
    emit selectedItemsCountChanged(m_selectedItems.count());

    return true;
}

QVariantList ShowManager::selectedItemViews() const
{
    QVariantList list;
    foreach (SelectedShowItem si, m_selectedItems)
    {
        if (si.m_item != nullptr)
            list.append(QVariant::fromValue(si.m_item.data()));
    }
    return list;
}

bool ShowManager::groupDragActive() const
{
    return m_groupDragActive;
}

void ShowManager::setGroupDragActive(bool active)
{
    if (m_groupDragActive == active)
        return;

    m_groupDragActive = active;
    emit groupDragActiveChanged();
}

QPointF ShowManager::groupDragOffset() const
{
    return m_groupDragOffset;
}

void ShowManager::setGroupDragOffset(QPointF offset)
{
    if (m_groupDragOffset == offset)
        return;

    m_groupDragOffset = offset;
    emit groupDragOffsetChanged();
}

void ShowManager::selectItemsInRect(QRectF rect, bool addToSelection)
{
    if (m_currentShow == nullptr)
        return;

    int prevCount = m_selectedItems.count();

    if (addToSelection == false)
    {
        foreach (SelectedShowItem ssi, m_selectedItems)
        {
            if (ssi.m_item != nullptr)
                ssi.m_item->setProperty("isSelected", false);
        }
        m_selectedItems.clear();
    }

    int trkIdx = 0;

    foreach (Track *track, m_currentShow->tracks())
    {
        foreach (ShowFunction *sf, track->showFunctions())
        {
            QQuickItem *item = m_itemsMap.value(sf->id(), nullptr);
            if (item == nullptr)
                continue;

            QRectF itemRect(item->x(), item->y(), item->width(), item->height());
            if (rect.contains(itemRect) == false)
                continue;

            bool alreadySelected = false;
            foreach (SelectedShowItem si, m_selectedItems)
            {
                if (si.m_showFunc == sf)
                {
                    alreadySelected = true;
                    break;
                }
            }

            if (alreadySelected)
                continue;

            item->setProperty("isSelected", true);

            SelectedShowItem selection;
            selection.m_trackIndex = trkIdx;
            selection.m_showFunc = sf;
            selection.m_item = item;
            m_selectedItems.append(selection);
        }

        trkIdx++;
    }

    if (addToSelection == false || m_selectedItems.count() != prevCount)
    {
        selectTrackOfSelectedItems();
        emit selectedItemsCountChanged(m_selectedItems.count());
    }
    emit itemClicked(App::ShowDragItem);
}

QVariantList ShowManager::selectedItemRefs() const
{
    QVariantList list;
    foreach (SelectedShowItem si, m_selectedItems)
    {
        if (si.m_showFunc != nullptr)
            list.append(QVariant::fromValue(si.m_showFunc.data()));
    }
    return list;
}

QStringList ShowManager::selectedItemNames() const
{
    QStringList names;
    foreach (SelectedShowItem si, m_selectedItems)
    {
        if (si.m_showFunc == nullptr)
            continue;

        Function *func = m_doc->function(si.m_showFunc->functionID());
        if (func != nullptr)
            names.append(func->name());
    }

    return names;
}

bool ShowManager::selectedItemsLocked() const
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

void ShowManager::slotFunctionRemoved(quint32 id)
{
    /* The Function is still valid at this point, but it is about to be
       destroyed: drop every reference to it and its items before that */
    if (m_currentShow != nullptr && m_currentShow->id() == id)
        resetContents();
}

/*********************************************************************
 * Chaser tempo conversion
 *********************************************************************/

bool ShowManager::selectionHasChasers() const
{
    for (const SelectedShowItem &ssi : m_selectedItems)
    {
        if (ssi.m_showFunc.isNull())
            continue;

        Function *func = m_doc->function(ssi.m_showFunc->functionID());
        if (func != nullptr &&
            (func->type() == Function::ChaserType || func->type() == Function::SequenceType))
            return true;
    }

    return false;
}

QList<ShowManager::TempoConversionPlan> ShowManager::tempoConversionPlans(const QVariantMap &options,
                                                                            QString &error) const
{
    QList<TempoConversionPlan> plans;
    bool toBeats = options.value("toBeats", true).toBool();
    bool clone = options.value("clone", false).toBool();
    bool allItems = options.value("allItems", false).toBool();
    bool perTempo = options.value("perTempo", true).toBool();
    bool fixedBpm = options.value("bpmMode").toString() == "fixed";
    double bpm = options.value("bpm", 120.0).toDouble();
    int globalBpm = m_doc->inputOutputMap()->bpmNumber();
    Function::TempoType sourceType = toBeats ? Function::Time : Function::Beats;

    // the Chasers to convert, with the selected items using each of them
    QList<Chaser *> chasers;
    QMap<Chaser *, QList<ShowFunction *>> selectedItems;
    QVariantList chaserIds = options.value("chaserIds").toList();

    if (chaserIds.isEmpty())
    {
        for (const SelectedShowItem &ssi : m_selectedItems)
        {
            if (ssi.m_showFunc.isNull())
                continue;

            Chaser *chaser = qobject_cast<Chaser *>(m_doc->function(ssi.m_showFunc->functionID()));
            if (chaser == nullptr || chaser->tempoType() != sourceType)
                continue;

            if (chasers.contains(chaser) == false)
                chasers.append(chaser);
            selectedItems[chaser].append(ssi.m_showFunc);
        }
    }
    else
    {
        for (const QVariant &id : chaserIds)
        {
            Chaser *chaser = qobject_cast<Chaser *>(m_doc->function(id.toUInt()));
            if (chaser != nullptr && chaser->tempoType() == sourceType && chasers.contains(chaser) == false)
                chasers.append(chaser);
        }
    }

    if (chasers.isEmpty())
    {
        error = toBeats ? tr("There is no Time tempo Chaser to convert.")
                        : tr("There is no Beats tempo Chaser to convert.");
        return plans;
    }

    if (fixedBpm && bpm <= 0)
    {
        error = tr("The BPM must be greater than zero.");
        return plans;
    }

    for (Chaser *chaser : chasers)
    {
        TempoConversionPlan plan;
        plan.chaser = chaser;

        // the items that follow the conversion
        QList<ShowFunction *> items = selectedItems.value(chaser);
        if (clone && allItems && m_currentShow != nullptr)
        {
            items.clear();
            for (Track *track : m_currentShow->tracks())
                for (ShowFunction *sf : track->showFunctions())
                    if (sf->functionID() == chaser->id())
                        items.append(sf);
        }

        for (ShowFunction *sf : items)
        {
            double itemBpm = bpm;
            if (fixedBpm == false)
            {
                itemBpm = globalBpm > 0 ? globalBpm : 120;
                if (m_currentShow != nullptr)
                    itemBpm = 60000.0 / m_currentShow->tempoMap().beatDurationAt(sf->startTime(), itemBpm);
            }
            itemBpm = qRound(itemBpm * 100) / 100.0;
            if (plan.itemBpms.contains(itemBpm) == false)
                plan.itemBpms.append(itemBpm);

            // one group per tempo for copies, one group otherwise
            int groupIndex = -1;
            for (int g = 0; g < plan.groups.count(); g++)
            {
                if (clone == false || perTempo == false || plan.groups.at(g).bpm == itemBpm)
                {
                    groupIndex = g;
                    break;
                }
            }

            if (groupIndex == -1)
            {
                TempoConversionGroup group;
                group.bpm = itemBpm;
                plan.groups.append(group);
                groupIndex = plan.groups.count() - 1;
            }
            plan.groups[groupIndex].items.append(sf);
        }

        // a Chaser with no item to follow (e.g. from its editor)
        if (plan.groups.isEmpty())
        {
            TempoConversionGroup group;
            group.bpm = fixedBpm ? bpm : (globalBpm > 0 ? globalBpm : 120);
            if (fixedBpm == false && m_currentShow != nullptr)
                group.bpm = qRound(60000.0 / m_currentShow->tempoMap().beatDurationAt(m_currentTime, group.bpm) * 100) / 100.0;
            plan.groups.append(group);
        }

        plans.append(plan);
    }

    return plans;
}

QPair<quint32, quint32> ShowManager::convertedItemTimes(const ShowFunction *sf, bool toBeats) const
{
    int bpm = m_doc->inputOutputMap()->bpmNumber();
    double beatMs = 60000.0 / (bpm > 0 ? bpm : 120);

    if (toBeats)
        return qMakePair(quint32(qRound64((sf->startTime() / beatMs) * 1000.0)),
                         quint32(qRound64((sf->duration() / beatMs) * 1000.0)));

    return qMakePair(quint32(qRound64((sf->startTime() / 1000.0) * beatMs)),
                     quint32(qRound64((sf->duration() / 1000.0) * beatMs)));
}

static QString speedToString(uint value, bool beats)
{
    if (value == Function::infiniteSpeed())
        return QString(QChar(0x221E));
    if (value == Function::defaultSpeed())
        return ShowManager::tr("default");
    if (beats)
        return value == 1000 ? ShowManager::tr("1 beat") : ShowManager::tr("%1 beats").arg(value / 1000.0);
    return ShowManager::tr("%1 ms").arg(value);
}

QVariantMap ShowManager::tempoConversionPreview(QVariantMap options)
{
    QVariantMap result;
    QVariantList chaserIds;
    QStringList lines;
    QString error;

    QList<TempoConversionPlan> plans = tempoConversionPlans(options, error);
    bool toBeats = options.value("toBeats", true).toBool();
    bool clone = options.value("clone", false).toBool();
    double resolution = options.value("resolution", 0.25).toDouble();

    for (const TempoConversionPlan &plan : plans)
    {
        chaserIds.append(plan.chaser->id());

        for (const TempoConversionGroup &group : plan.groups)
        {
            QString target = clone ? tr("new copy \"%1\"").arg(plan.chaser->name() +
                                     (toBeats ? tr(" (beats") : tr(" (time")) +
                                     (plan.groups.count() > 1 ? QString(", %1)").arg(group.bpm) : QString(")")))
                                   : tr("the Chaser itself");
            if (group.items.isEmpty())
            {
                lines.append(tr("%1: at %2 BPM → %3").arg(plan.chaser->name()).arg(group.bpm).arg(target));
            }
            else
            {
                QString items = group.items.count() == 1 ? tr("1 item") : tr("%1 items").arg(group.items.count());
                lines.append(tr("%1: %2 at %3 BPM → %4")
                             .arg(plan.chaser->name()).arg(items).arg(group.bpm).arg(target));
            }

            // preview the step timings at this tempo
            auto convert = [&](uint value)
            {
                return toBeats ? Chaser::timeToBeats(value, group.bpm, resolution)
                               : Chaser::beatsToTime(value, group.bpm);
            };

            if (plan.chaser->durationMode() == Chaser::Common)
                lines.append(tr("    Steps: %1 → %2").arg(speedToString(plan.chaser->duration(), !toBeats))
                             .arg(speedToString(convert(plan.chaser->duration()), toBeats)));
            if (plan.chaser->fadeInMode() == Chaser::Common)
                lines.append(tr("    Fade in: %1 → %2").arg(speedToString(plan.chaser->fadeInSpeed(), !toBeats))
                             .arg(speedToString(convert(plan.chaser->fadeInSpeed()), toBeats)));
            if (plan.chaser->fadeOutMode() == Chaser::Common)
                lines.append(tr("    Fade out: %1 → %2").arg(speedToString(plan.chaser->fadeOutSpeed(), !toBeats))
                             .arg(speedToString(convert(plan.chaser->fadeOutSpeed()), toBeats)));

            QList<ChaserStep> steps = plan.chaser->steps();
            for (int i = 0; i < steps.count() && i < 8; i++)
            {
                const ChaserStep &step = steps.at(i);
                if (plan.chaser->durationMode() != Chaser::PerStep &&
                    plan.chaser->fadeInMode() != Chaser::PerStep && plan.chaser->fadeOutMode() != Chaser::PerStep)
                    break;

                lines.append(tr("    Step %1: hold %2 → %3, fade in %4 → %5")
                             .arg(i + 1)
                             .arg(speedToString(step.hold, !toBeats)).arg(speedToString(convert(step.hold), toBeats))
                             .arg(speedToString(step.fadeIn, !toBeats)).arg(speedToString(convert(step.fadeIn), toBeats)));
            }
            if (steps.count() > 8 && plan.chaser->durationMode() == Chaser::PerStep)
                lines.append(tr("    … and %1 more steps").arg(steps.count() - 8));
        }

        if (clone == false)
        {
            if (plan.itemBpms.count() > 1)
            {
                QStringList tempos;
                for (double itemBpm : plan.itemBpms)
                    tempos.append(QString::number(itemBpm));
                lines.append(tr("    Its items are at %1 BPM, but a Chaser converted in place has a single tempo: "
                                "%2 BPM. Convert to copies for one per tempo.")
                             .arg(tempos.join(", ")).arg(plan.groups.first().bpm));
            }

            // every use of the Chaser changes with it
            QList<quint32> usage = m_doc->getUsage(plan.chaser->id());
            QSet<quint32> users;
            bool fixUps = false;
            for (int i = 0; i < usage.count(); i += 2)
            {
                users.insert(usage.at(i));
                Show *show = qobject_cast<Show *>(m_doc->function(usage.at(i)));
                if (show != nullptr && show->itemsInMs() == false)
                    fixUps = true;
            }
            if (users.isEmpty() == false)
                lines.append(tr("    Used in %1 Shows or Functions, which all change with it.").arg(users.count()));
            if (fixUps)
                lines.append(tr("    Its items in Shows without tempo sections are adjusted to stay in place."));
        }
    }

    result.insert("valid", error.isEmpty());
    result.insert("message", error);
    result.insert("lines", lines);
    result.insert("chaserIds", chaserIds);
    return result;
}

bool ShowManager::applyTempoConversion(QVariantMap options)
{
    QString error;
    QList<TempoConversionPlan> plans = tempoConversionPlans(options, error);
    if (plans.isEmpty())
        return false;

    bool toBeats = options.value("toBeats", true).toBool();
    bool clone = options.value("clone", false).toBool();
    double resolution = options.value("resolution", 0.25).toDouble();
    Function::TempoType newType = toBeats ? Function::Beats : Function::Time;

    for (const TempoConversionPlan &plan : plans)
    {
        Chaser *chaser = plan.chaser;

        if (clone == false)
        {
            // Shows that position Beats tempo items in beats would read the
            // items of this Chaser in the wrong unit once it changes type
            for (Function *f : m_doc->functionsByType(Function::ShowType))
            {
                Show *show = qobject_cast<Show *>(f);
                if (show == nullptr || show->itemsInMs())
                    continue;

                for (Track *track : show->tracks())
                {
                    for (ShowFunction *sf : track->showFunctions())
                    {
                        if (sf->functionID() != chaser->id())
                            continue;

                        QPair<quint32, quint32> times = convertedItemTimes(sf, toBeats);
                        QVariantList oldTimes = { sf->id(), sf->startTime(), sf->duration() };
                        QVariantList newTimes = { sf->id(), times.first, times.second };
                        sf->setStartTime(times.first);
                        sf->setDuration(times.second);
                        Tardis::instance()->enqueueAction(Tardis::ShowManagerShowItemSetTimes, show->id(),
                                                          oldTimes, newTimes);
                    }
                }
            }

            QByteArray oldState = Tardis::instance()->actionToByteArray(Tardis::FunctionCreate, chaser->id());
            chaser->convertTempoType(newType, plan.groups.first().bpm, resolution);
            Tardis::instance()->enqueueAction(Tardis::ChaserSetState, chaser->id(), oldState,
                                              Tardis::instance()->actionToByteArray(Tardis::FunctionCreate, chaser->id()));
            continue;
        }

        for (const TempoConversionGroup &group : plan.groups)
        {
            Function *copy = chaser->createCopy(m_doc);
            Chaser *copyChaser = qobject_cast<Chaser *>(copy);
            if (copyChaser == nullptr)
                continue;

            copyChaser->setName(chaser->name() + (toBeats ? tr(" (beats") : tr(" (time")) +
                                (plan.groups.count() > 1 ? QString(", %1)").arg(group.bpm) : QString(")")));
            copyChaser->convertTempoType(newType, group.bpm, resolution);
            Tardis::instance()->enqueueAction(Tardis::FunctionCreate, copy->id(), QVariant(),
                                              Tardis::instance()->actionToByteArray(Tardis::FunctionCreate, copy->id()));

            for (ShowFunction *sf : group.items)
            {
                if (m_currentShow->itemsInMs() == false)
                {
                    QPair<quint32, quint32> times = convertedItemTimes(sf, toBeats);
                    QVariantList oldTimes = { sf->id(), sf->startTime(), sf->duration() };
                    QVariantList newTimes = { sf->id(), times.first, times.second };
                    sf->setStartTime(times.first);
                    sf->setDuration(times.second);
                    Tardis::instance()->enqueueAction(Tardis::ShowManagerShowItemSetTimes, m_currentShow->id(),
                                                      oldTimes, newTimes);
                }

                quint32 oldId = sf->functionID();
                setShowItemFunction(sf->id(), copy->id());
                Tardis::instance()->enqueueAction(Tardis::ShowManagerItemSetFunction, sf->id(), oldId, copy->id());
            }
        }
    }

    emit showDurationChanged(m_currentShow != nullptr ? m_currentShow->totalDuration() : 0);
    return true;
}

void ShowManager::slotTimeChanged(quint32 msec_time)
{
    m_currentTime = (int)msec_time;
    emit currentTimeChanged(m_currentTime);
}

void ShowManager::slotShowFinished()
{
    stopShow();
}

void ShowManager::slotShowStopped()
{
    setPlaybackState(false, false);
}

void ShowManager::setPlaybackState(bool playing, bool paused)
{
    if (playing == false)
        paused = false;

    if (m_isPlaying != playing)
    {
        m_isPlaying = playing;
        emit isPlayingChanged(m_isPlaying);
    }

    if (m_isPaused != paused)
    {
        m_isPaused = paused;
        emit isPausedChanged(m_isPaused);
    }
}

bool ShowManager::checkOverlapping(Track *track, ShowFunction *sourceFunc,
                                   quint32 startTime, quint32 duration) const
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
            // items are half-open intervals [start, start + duration), so an
            // item starting exactly where another one ends is not overlapping
            quint32 fst = sf->startTime();
            if (startTime == fst ||
                (startTime < fst + sf->duration() && fst < startTime + duration))
            {
                return true;
            }
        }
    }

    return false;
}

bool ShowManager::checkOverlapping(Track *track, const QList<ShowFunction *> &exclude,
                                   quint32 startTime, quint32 duration) const
{
    if (track == nullptr)
        return false;

    foreach (ShowFunction *sf, track->showFunctions())
    {
        if (exclude.contains(sf))
            continue;

        Function *func = m_doc->function(sf->functionID());
        if (func != nullptr)
        {
            // items are half-open intervals [start, start + duration), so an
            // item starting exactly where another one ends is not overlapping
            quint32 fst = sf->startTime();
            if (startTime == fst ||
                (startTime < fst + sf->duration() && fst < startTime + duration))
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
    // copying nothing keeps what is already in the clipboard
    if (m_selectedItems.isEmpty())
        return;

    m_clipboard.clear();

    for (SelectedShowItem item : m_selectedItems)
        m_clipboard.append(item);

    clearCutState();
    emit clipboardItemsCountChanged(m_clipboard.count());
}

bool ShowManager::cutToClipboard()
{
    if (m_selectedItems.isEmpty())
        return false;

    // pasting cut items moves them, which a lock forbids
    if (selectedItemsLocked())
    {
        emit clipboardActionFailed(tr("Cut error"),
                                   tr("Locked items cannot be cut. Unlock them first, or copy them instead."));
        return false;
    }

    m_clipboard.clear();

    for (SelectedShowItem item : m_selectedItems)
        m_clipboard.append(item);

    m_clipboardIsCut = true;
    emit clipboardItemsCountChanged(m_clipboard.count());
    emit cutItemIdsChanged();

    return true;
}

QVariantList ShowManager::cutItemIds() const
{
    QVariantList list;

    if (m_clipboardIsCut == false)
        return list;

    for (const SelectedShowItem &item : m_clipboard)
    {
        if (item.m_showFunc != nullptr)
            list.append(item.m_showFunc->id());
    }

    return list;
}

void ShowManager::clearCutState()
{
    if (m_clipboardIsCut == false)
        return;

    m_clipboardIsCut = false;
    emit cutItemIdsChanged();
}

quint32 ShowManager::showToFunctionTime(const Function *func, quint32 value) const
{
    bool showIsBeats = timeDivision() != Show::Time;
    bool funcIsBeats = itemInBeats(func);

    if (showIsBeats == funcIsBeats)
        return value;

    int bpm = m_doc->inputOutputMap()->bpmNumber();
    int beatDuration = bpm > 0 ? (60000 / bpm) : 500;

    return funcIsBeats ? Function::timeToBeats(value, beatDuration)
                       : Function::beatsToTime(value, beatDuration);
}

quint32 ShowManager::functionToShowTime(const Function *func, quint32 value) const
{
    bool showIsBeats = timeDivision() != Show::Time;
    bool funcIsBeats = itemInBeats(func);

    if (showIsBeats == funcIsBeats)
        return value;

    int bpm = m_doc->inputOutputMap()->bpmNumber();
    int beatDuration = bpm > 0 ? (60000 / bpm) : 500;

    return showIsBeats ? Function::timeToBeats(value, beatDuration)
                       : Function::beatsToTime(value, beatDuration);
}

bool ShowManager::pasteFromClipboard()
{
    if (m_currentShow == nullptr)
        return false;

    struct PasteItem
    {
        ShowFunction *sf;
        Function *func;
        int srcTrackIdx;
        int dstTrackIdx;
        quint32 showTime;   // source start time, in the Show timeline unit
        quint32 dstTime;    // destination start time, in the unit of func
    };

    QList<Track *> tracks = m_currentShow->tracks();
    QList<PasteItem> pasteList;
    QList<ShowFunction *> cutFuncs;
    quint32 lowerTime = UINT_MAX;
    int topTrack = INT_MAX;
    int bottomTrack = -1;

    // pre-parse the clipboard items to find the one with the
    // lowest start time, and the topmost and bottommost tracks
    for (const SelectedShowItem &item : std::as_const(m_clipboard))
    {
        if (item.m_showFunc == nullptr)
            continue;

        PasteItem pi;
        pi.sf = item.m_showFunc.data();
        pi.func = m_doc->function(pi.sf->functionID());
        if (pi.func == nullptr)
            continue;

        // a Sequence can't be pasted without its bound Scene
        if (pi.func->type() == Function::SequenceType)
        {
            Sequence *sequence = qobject_cast<Sequence*>(pi.func);
            if (m_doc->function(sequence->boundSceneID()) == nullptr)
                continue;
        }

        // the items may have been moved since they were copied,
        // so use the Track they are on now
        Track *srcTrack = m_currentShow->getTrackFromShowFunctionID(pi.sf->id());
        pi.srcTrackIdx = srcTrack != nullptr ? tracks.indexOf(srcTrack) : int(item.m_trackIndex);
        pi.dstTrackIdx = pi.srcTrackIdx;

        // items can mix time and beat based Functions, so compare
        // their start times on the Show timeline
        pi.showTime = functionToShowTime(pi.func, pi.sf->startTime());
        pi.dstTime = 0;

        lowerTime = qMin(lowerTime, pi.showTime);
        topTrack = qMin(topTrack, pi.srcTrackIdx);
        bottomTrack = qMax(bottomTrack, pi.srcTrackIdx);

        pasteList.append(pi);
        if (m_clipboardIsCut)
            cutFuncs.append(pi.sf);
    }

    if (pasteList.isEmpty())
    {
        emit clipboardActionFailed(tr("Paste error"), tr("There are no items to paste."));
        return false;
    }

    // the topmost Track of the items lands on the selected Track, and the
    // others follow below, gap Tracks included. With no Track selected,
    // the items land on the Tracks they come from
    int selectedIdx = tracks.indexOf(m_currentShow->track(m_selectedTrackId));
    int trackOffset = selectedIdx >= 0 ? selectedIdx - topTrack : 0;

    if (bottomTrack + trackOffset >= tracks.count())
    {
        emit clipboardActionFailed(tr("Paste error"),
            tr("The items span %1 tracks, but only %2 tracks are available from the selected track downwards.")
                .arg(bottomTrack - topTrack + 1).arg(tracks.count() - (topTrack + trackOffset)));
        return false;
    }

    quint32 cursorTime = m_currentTime > 0 ? quint32(m_currentTime) : 0;

    // check every destination before pasting anything
    for (PasteItem &pi : pasteList)
    {
        if (m_clipboardIsCut && pi.sf->isLocked())
        {
            emit clipboardActionFailed(tr("Paste error"),
                tr("\"%1\" has been locked since it was cut, so it cannot be moved.").arg(pi.func->name()));
            return false;
        }

        pi.dstTrackIdx = pi.srcTrackIdx + trackOffset;
        pi.dstTime = showToFunctionTime(pi.func, cursorTime + (pi.showTime - lowerTime));

        // cut items leave their place, so they don't prevent pasting over it
        Track *dstTrack = tracks.at(pi.dstTrackIdx);
        if (checkOverlapping(dstTrack, cutFuncs, pi.dstTime, pi.sf->duration()))
        {
            emit clipboardActionFailed(tr("Paste error"),
                tr("\"%1\" would overlap an existing item on track \"%2\".")
                    .arg(pi.func->name(), dstTrack->name()));
            return false;
        }
    }

    QList<ShowFunction *> pastedFuncs;

    if (m_clipboardIsCut)
    {
        QVariantList views, trackIndexes, startTimes;

        for (const PasteItem &pi : std::as_const(pasteList))
        {
            views.append(QVariant::fromValue(m_itemsMap.value(pi.sf->id(), nullptr)));
            trackIndexes.append(pi.dstTrackIdx);
            startTimes.append(pi.dstTime);
            pastedFuncs.append(pi.sf);
        }

        if (moveShowItems(views, trackIndexes, startTimes) == false)
        {
            emit clipboardActionFailed(tr("Paste error"), tr("The cut items could not be moved."));
            return false;
        }

        // the moved items stay in the clipboard, now as a copy
        clearCutState();
    }
    else
    {
        for (const PasteItem &pi : std::as_const(pasteList))
        {
            addItems(contextItem(), pi.dstTrackIdx, cursorTime + (pi.showTime - lowerTime),
                     QVariantList() << pi.func->id(), pi.sf);

            Track *dstTrack = tracks.at(pi.dstTrackIdx);
            if (dstTrack->showFunctions().isEmpty() == false)
                pastedFuncs.append(dstTrack->showFunctions().last());
        }
    }

    // the pasted items replace the selection
    for (const SelectedShowItem &ssi : std::as_const(m_selectedItems))
    {
        if (ssi.m_item != nullptr)
            ssi.m_item->setProperty("isSelected", false);
    }
    m_selectedItems.clear();

    for (ShowFunction *sf : std::as_const(pastedFuncs))
    {
        QQuickItem *view = m_itemsMap.value(sf->id(), nullptr);
        if (view == nullptr)
            continue;

        view->setProperty("isSelected", true);

        SelectedShowItem selection;
        selection.m_trackIndex = tracks.indexOf(m_currentShow->getTrackFromShowFunctionID(sf->id()));
        selection.m_showFunc = sf;
        selection.m_item = view;
        m_selectedItems.append(selection);
    }

    selectTrackOfSelectedItems();
    emit selectedItemsCountChanged(m_selectedItems.count());
    emit showDurationChanged(m_currentShow->totalDuration());

    return true;
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool ShowManager::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != nullptr);

    /* Nothing to remember if no Show is being edited */
    if (m_currentShow == nullptr)
        return true;

    doc->writeStartElement(KXMLQLCShowManager);
    doc->writeAttribute(KXMLQLCShowManagerCurrentShow, QString::number(m_currentShow->id()));
    doc->writeAttribute(KXMLQLCShowManagerTimeScale, QString::number(m_timeScale));
    doc->writeEndElement();

    return true;
}

bool ShowManager::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCShowManager)
    {
        qWarning() << Q_FUNC_INFO << "Show Manager node not found";
        return false;
    }

    QXmlStreamAttributes attrs = root.attributes();
    root.skipCurrentElement();

    /* Ignore a reference to a missing Function or to one that is not a Show */
    bool ok = false;
    quint32 showID = attrs.value(KXMLQLCShowManagerCurrentShow).toUInt(&ok);
    if (ok == false || qobject_cast<Show *>(m_doc->function(showID)) == nullptr)
        return true;

    /* This also applies the default time scale of the Show time division */
    setCurrentShowID(showID);

    float timeScale = attrs.value(KXMLQLCShowManagerTimeScale).toFloat(&ok);
    if (ok && timeScale > 0)
        setTimeScale(timeScale);

    return true;
}
