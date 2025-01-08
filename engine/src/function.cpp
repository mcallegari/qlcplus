/*
  Q Light Controller Plus
  function.cpp

  Copyright (c) Heikki Junnila
                Massimo Callegari

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
#include <QElapsedTimer>
#include <QString>
#include <QDebug>
#include <math.h>

#include "qlcmacros.h"
#include "qlcfile.h"

#include "scriptwrapper.h"
#include "mastertimer.h"
#include "collection.h"
#include "rgbmatrix.h"
#include "function.h"
#include "sequence.h"
#include "chaser.h"
#include "audio.h"
#include "video.h"
#include "scene.h"
#include "show.h"
#include "efx.h"
#include "doc.h"

#define OVERRIDE_ATTRIBUTE_START_ID     128

const QString KSceneString      (      "Scene" );
const QString KChaserString     (     "Chaser" );
const QString KEFXString        (        "EFX" );
const QString KCollectionString ( "Collection" );
const QString KScriptString     (     "Script" );
const QString KRGBMatrixString  (  "RGBMatrix" );
const QString KShowString       (       "Show" );
const QString KSequenceString   (   "Sequence" );
const QString KAudioString      (      "Audio" );
const QString KVideoString      (      "Video" );
const QString KUndefinedString  (  "Undefined" );

const QString KLoopString       (       "Loop" );
const QString KPingPongString   (   "PingPong" );
const QString KSingleShotString ( "SingleShot" );
const QString KRandomString     (     "Random" );

const QString KBackwardString   (   "Backward" );
const QString KForwardString    (    "Forward" );

const QString KTimeTypeString   (       "Time" );
const QString KBeatsTypeString  (      "Beats" );

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Function::Function(QObject *parent)
    : QObject(parent)
    , m_id(Function::invalidId())
    , m_type(Undefined)
    , m_path(QString())
    , m_visible(true)
    , m_runOrder(Loop)
    , m_direction(Forward)
    , m_tempoType(Time)
    , m_overrideTempoType(Original)
    , m_beatResyncNeeded(false)
    , m_fadeInSpeed(0)
    , m_fadeOutSpeed(0)
    , m_duration(0)
    , m_overrideFadeInSpeed(defaultSpeed())
    , m_overrideFadeOutSpeed(defaultSpeed())
    , m_overrideDuration(defaultSpeed())
    , m_flashing(false)
    , m_elapsed(0)
    , m_elapsedBeats(0)
    , m_stop(true)
    , m_running(false)
    , m_paused(false)
    , m_lastOverrideAttributeId(OVERRIDE_ATTRIBUTE_START_ID)
    , m_preserveAttributes(false)
    , m_blendMode(Universe::NormalBlend)
{

}

Function::Function(Doc* doc, Type t)
    : QObject(doc)
    , m_id(Function::invalidId())
    , m_type(t)
    , m_path(QString())
    , m_visible(true)
    , m_runOrder(Loop)
    , m_direction(Forward)
    , m_tempoType(Time)
    , m_overrideTempoType(Original)
    , m_beatResyncNeeded(false)
    , m_fadeInSpeed(0)
    , m_fadeOutSpeed(0)
    , m_duration(0)
    , m_overrideFadeInSpeed(defaultSpeed())
    , m_overrideFadeOutSpeed(defaultSpeed())
    , m_overrideDuration(defaultSpeed())
    , m_flashing(false)
    , m_elapsed(0)
    , m_elapsedBeats(0)
    , m_stop(true)
    , m_running(false)
    , m_paused(false)
    , m_lastOverrideAttributeId(OVERRIDE_ATTRIBUTE_START_ID)
    , m_preserveAttributes(false)
    , m_blendMode(Universe::NormalBlend)
{
    Q_ASSERT(doc != NULL);
    registerAttribute(tr("Intensity"), Multiply | Single);
}

Function::~Function()
{
}

Doc* Function::doc() const
{
    return qobject_cast<Doc*>(parent());
}

/*****************************************************************************
 * Copying
 *****************************************************************************/
Function *Function::createCopy(Doc *doc, bool addToDoc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new Function(doc, type());
    if (copy->copyFrom(this) == false)
    {
        delete copy;
        copy = NULL;
    }
    if (addToDoc == true && doc->addFunction(copy) == false)
    {
        delete copy;
        copy = NULL;
    }

    return copy;
}

bool Function::copyFrom(const Function* function)
{
    if (function == NULL)
        return false;

    m_name = function->name();
    m_runOrder = function->runOrder();
    m_direction = function->direction();
    m_tempoType = function->tempoType();
    m_fadeInSpeed = function->fadeInSpeed();
    m_fadeOutSpeed = function->fadeOutSpeed();
    m_duration = function->duration();
    m_path = function->path(true);
    m_visible = function->isVisible();
    m_blendMode = function->blendMode();
    m_uiState = function->uiStateMap();

    emit changed(m_id);

    return true;
}

/*****************************************************************************
 * ID
 *****************************************************************************/

void Function::setID(quint32 id)
{
    /* Don't set doc modified status or emit changed signal, because this
       function is called only once during function creation. */
    m_id = id;
}

quint32 Function::id() const
{
    return m_id;
}

quint32 Function::invalidId()
{
    return UINT_MAX;
}

/*****************************************************************************
 * Name
 *****************************************************************************/

void Function::setName(const QString& name)
{
    if (m_name == name)
        return;

    m_name = QString(name);

    emit nameChanged(m_id);
}

QString Function::name() const
{
    return m_name;
}

/*****************************************************************************
 * Type
 *****************************************************************************/

Function::Type Function::type() const
{
    return m_type;
}

QString Function::typeString() const
{
    return Function::typeToString(type());
}

QString Function::typeToString(Type type)
{
    switch (type)
    {
        case SceneType:      return KSceneString;
        case ChaserType:     return KChaserString;
        case EFXType:        return KEFXString;
        case CollectionType: return KCollectionString;
        case ScriptType:     return KScriptString;
        case RGBMatrixType:  return KRGBMatrixString;
        case ShowType:       return KShowString;
        case SequenceType:   return KSequenceString;
        case AudioType:      return KAudioString;
        case VideoType:      return KVideoString;
        case Undefined:
        default:
            return KUndefinedString;
    }
}

Function::Type Function::stringToType(const QString& string)
{
    if (string == KSceneString)
        return SceneType;
    else if (string == KChaserString)
        return ChaserType;
    else if (string == KEFXString)
        return EFXType;
    else if (string == KCollectionString)
        return CollectionType;
    else if (string == KScriptString)
        return ScriptType;
    else if (string == KRGBMatrixString)
        return RGBMatrixType;
    else if (string == KShowString)
        return ShowType;
    else if (string == KSequenceString)
        return SequenceType;
    else if (string == KAudioString)
        return AudioType;
    else if (string == KVideoString)
        return VideoType;
    else
        return Undefined;
}

QIcon Function::getIcon() const
{
    return QIcon(":/function.png");
}

/*********************************************************************
 * Path
 *********************************************************************/
void Function::setPath(QString path)
{
    if (path.contains(typeToString(type())))
        path.remove(typeToString(type()) + "/");
    //qDebug() << "Function " << name() << "path set to:" << path;
    m_path = path;
}

QString Function::path(bool simplified) const
{
    if (simplified == true)
        return m_path;
    else
        return QString("%1/%2").arg(typeToString(type())).arg(m_path);
}

/*********************************************************************
 * Visibility
 *********************************************************************/

void Function::setVisible(bool visible)
{
    m_visible = visible;
}

bool Function::isVisible() const
{
    return m_visible;
}

/*********************************************************************
 * Common
 *********************************************************************/

bool Function::saveXMLCommon(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    doc->writeAttribute(KXMLQLCFunctionID, QString::number(id()));
    doc->writeAttribute(KXMLQLCFunctionType, Function::typeToString(type()));
    doc->writeAttribute(KXMLQLCFunctionName, name());
    if (isVisible() == false)
        doc->writeAttribute(KXMLQLCFunctionHidden, KXMLQLCTrue);
    if (path(true).isEmpty() == false)
        doc->writeAttribute(KXMLQLCFunctionPath, path(true));
    if (blendMode() != Universe::NormalBlend)
        doc->writeAttribute(KXMLQLCFunctionBlendMode, Universe::blendModeToString(blendMode()));

    return true;
}

/*****************************************************************************
 * Running order
 *****************************************************************************/

void Function::setRunOrder(const Function::RunOrder& order)
{
    if (order == Loop || order == SingleShot || order == PingPong || order == Random)
        m_runOrder = order;
    else
        m_runOrder = Loop;
    emit runOrderChanged();
    emit changed(m_id);
}

Function::RunOrder Function::runOrder() const
{
    return m_runOrder;
}

QString Function::runOrderToString(const RunOrder& order)
{
    switch (order)
    {
    default:
    case Loop:
        return KLoopString;
    case PingPong:
        return KPingPongString;
    case SingleShot:
        return KSingleShotString;
    case Random:
        return KRandomString;
    }
}

Function::RunOrder Function::stringToRunOrder(const QString& str)
{
    if (str == KPingPongString)
        return PingPong;
    else if (str == KSingleShotString)
        return SingleShot;
    else if (str == KRandomString)
        return Random;
    else
        return Loop;
}

bool Function::saveXMLRunOrder(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    doc->writeTextElement(KXMLQLCFunctionRunOrder, runOrderToString(runOrder()));

    return true;
}

bool Function::loadXMLRunOrder(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCFunctionRunOrder)
    {
        qWarning() << Q_FUNC_INFO << "RunOrder node not found";
        return false;
    }
    QString str = root.readElementText();
    if (str.isEmpty())
        return false;

    setRunOrder(stringToRunOrder(str));

    return true;
}

/*****************************************************************************
 * Direction
 *****************************************************************************/

void Function::setDirection(const Function::Direction& dir)
{
    if (dir == Forward || dir == Backward)
        m_direction = dir;
    else
        m_direction = Forward;
    emit changed(m_id);
}

Function::Direction Function::direction() const
{
    return m_direction;
}

QString Function::directionToString(const Direction& dir)
{
    switch (dir)
    {
        default:
        case Forward:
            return KForwardString;
        case Backward:
            return KBackwardString;
    }
}

Function::Direction Function::stringToDirection(const QString& str)
{
    if (str == KBackwardString)
        return Backward;
    else
        return Forward;
}

bool Function::saveXMLDirection(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    doc->writeTextElement(KXMLQLCFunctionDirection, directionToString(direction()));

    return true;
}

bool Function::loadXMLDirection(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCFunctionDirection)
    {
        qWarning() << Q_FUNC_INFO << "Direction node not found";
        return false;
    }

    QString str = root.readElementText();
    if (str.isEmpty())
        return false;

    setDirection(stringToDirection(str));

    return true;
}

/*********************************************************************
 * Speed type
 *********************************************************************/

void Function::setTempoType(const Function::TempoType &type)
{
    if (type == m_tempoType)
        return;

    m_tempoType = type;

    /* Retrieve the current BPM value known by the Master Timer */
    float bpmNum = doc()->masterTimer()->bpmNumber();
    /* Calculate the duration in ms of a single beat */
    float beatTime = 60000.0 / bpmNum;

    switch (type)
    {
        /* Beats -> Time */
        case Time:
            setFadeInSpeed(beatsToTime(fadeInSpeed(), beatTime));
            setDuration(beatsToTime(duration(), beatTime));
            setFadeOutSpeed(beatsToTime(fadeOutSpeed(), beatTime));
            disconnect(doc()->masterTimer(), SIGNAL(bpmNumberChanged(int)),
                       this, SLOT(slotBPMChanged(int)));
        break;

        /* Time -> Beats */
        case Beats:
            setFadeInSpeed(timeToBeats(fadeInSpeed(), beatTime));
            setDuration(timeToBeats(duration(), beatTime));
            setFadeOutSpeed(timeToBeats(fadeOutSpeed(), beatTime));
            connect(doc()->masterTimer(), SIGNAL(bpmNumberChanged(int)),
                    this, SLOT(slotBPMChanged(int)));
        break;
        default:
            qDebug() << "Error. Unhandled tempo type" << type;
        break;
    }

    emit changed(m_id);
    emit tempoTypeChanged();
}

Function::TempoType Function::tempoType() const
{
    return m_tempoType;
}

QString Function::tempoTypeToString(const Function::TempoType &type)
{
    switch (type)
    {
        default:
        case Time:
            return KTimeTypeString;
        case Beats:
            return KBeatsTypeString;
    }
}

Function::TempoType Function::stringToTempoType(const QString &str)
{
    if (str == KTimeTypeString)
        return Time;
    else
        return Beats;
}

uint Function::timeToBeats(uint time, int beatDuration)
{
    if (time == 0 || time == infiniteSpeed())
        return time;

    uint value = 0;

    float beats = (float)time / (float)beatDuration;
    value = floor(beats) * 1000;

    beats -= floor(beats);
    beats = floor((beats * 1000) / 125) * 125;

    return value + beats;
}

uint Function::beatsToTime(uint beats, int beatDuration)
{
    if (beats == 0 || beats == infiniteSpeed())
        return beats;

    return ((float)beats / 1000.0) * beatDuration;
}

Function::TempoType Function::overrideTempoType() const
{
    return m_overrideTempoType;
}

void Function::setOverrideTempoType(Function::TempoType type)
{
    m_overrideTempoType = type;
}

void Function::slotBPMChanged(int bpmNumber)
{
    Q_UNUSED(bpmNumber)
    m_beatResyncNeeded = true;
}

/****************************************************************************
 * Speed
 ****************************************************************************/

void Function::setFadeInSpeed(uint ms)
{
    m_fadeInSpeed = ms;
    emit changed(m_id);
}

uint Function::fadeInSpeed() const
{
    return m_fadeInSpeed;
}

void Function::setFadeOutSpeed(uint ms)
{
    m_fadeOutSpeed = ms;
    emit changed(m_id);
}

uint Function::fadeOutSpeed() const
{
    return m_fadeOutSpeed;
}

void Function::setDuration(uint ms)
{
    m_duration = ms;
    emit changed(m_id);
}

uint Function::duration() const
{
    return m_duration;
}

quint32 Function::totalDuration()
{
    // fall back to duration in case a
    // subclass doesn't provide this method
    return duration();
}

void Function::setTotalDuration(quint32 msec)
{
    Q_UNUSED(msec)
}

void Function::setOverrideFadeInSpeed(uint ms)
{
    m_overrideFadeInSpeed = ms;
}

uint Function::overrideFadeInSpeed() const
{
    return m_overrideFadeInSpeed;
}

void Function::setOverrideFadeOutSpeed(uint ms)
{
    m_overrideFadeOutSpeed = ms;
}

uint Function::overrideFadeOutSpeed() const
{
    return m_overrideFadeOutSpeed;
}

void Function::setOverrideDuration(uint ms)
{
    m_overrideDuration = ms;
}

uint Function::overrideDuration() const
{
    return m_overrideDuration;
}

QString Function::speedToString(uint ms)
{
    QString str;
    if (ms == infiniteSpeed())
    {
        str = QChar(0x221E); // Infinity symbol
    }
    else
    {
        uint h, m, s;

        h = ms / MS_PER_HOUR;
        ms -= (h * MS_PER_HOUR);

        m = ms / MS_PER_MINUTE;
        ms -= (m * MS_PER_MINUTE);

        s = ms / MS_PER_SECOND;
        ms -= (s * MS_PER_SECOND);

        if (h != 0)
            str += QString("%1h").arg(h, 1, 10, QChar('0'));
        if (m != 0)
            str += QString("%1m").arg(m, str.size() ? 2 : 1, 10, QChar('0'));
        if (s != 0)
            str += QString("%1s").arg(s, str.size() ? 2 : 1, 10, QChar('0'));
        if (ms != 0 || str.size() == 0)
            str += QString("%1ms").arg(ms, str.size() ? 3 : 1, 10, QChar('0'));
    }

    return str;
}

static uint speedSplit(QString& speedString, QString splitNeedle)
{
    QStringList splitResult;
    // Filter out "ms" because "m" and "s" may wrongly use it
    splitResult = speedString.split("ms");
    if (splitResult.count() > 1)
        splitResult = splitResult.at(0).split(splitNeedle);
    else
        splitResult = speedString.split(splitNeedle);

    if (splitResult.count() > 1)
    {
        speedString.remove(0, speedString.indexOf(splitNeedle) + 1);
        return splitResult.at(0).toUInt();
    }
    return 0;
}

uint Function::stringToSpeed(QString speed)
{
    uint value = 0;

    if (speed == QChar(0x221E)) // Infinity symbol
        return infiniteSpeed();

    value += speedSplit(speed, "h") * 1000 * 60 * 60;
    value += speedSplit(speed, "m") * 1000 * 60;
    value += speedSplit(speed, "s") * 1000;

    if (speed.contains("."))
    {
        // lround avoids toDouble precison issues (.03 transforms to .029)
        value += lround(speed.toDouble() * 1000.0);
    }
    else
    {
        if (speed.contains("ms"))
            speed = speed.split("ms").at(0);
        value += speed.toUInt();
    }

    return speedNormalize(value);
}

uint Function::speedNormalize(uint speed)
{
    if ((int)speed < 0)
        return infiniteSpeed();
    return speed;
}

uint Function::speedAdd(uint left, uint right)
{
    if (speedNormalize(left) == infiniteSpeed()
        || speedNormalize(right) == infiniteSpeed())
        return infiniteSpeed();

    return speedNormalize(left + right);
}

uint Function::speedSubtract(uint left, uint right)
{
    if (right >= left)
        return 0;
    if (speedNormalize(right) == infiniteSpeed())
        return 0;
    if (speedNormalize(left) == infiniteSpeed())
        return infiniteSpeed();

    return speedNormalize(left - right);
}

void Function::tap()
{
}

bool Function::loadXMLSpeed(QXmlStreamReader &speedRoot)
{
    if (speedRoot.name() != KXMLQLCFunctionSpeed)
        return false;

    QXmlStreamAttributes attrs = speedRoot.attributes();

    m_fadeInSpeed = attrs.value(KXMLQLCFunctionSpeedFadeIn).toString().toUInt();
    m_fadeOutSpeed = attrs.value(KXMLQLCFunctionSpeedFadeOut).toString().toUInt();
    m_duration = attrs.value(KXMLQLCFunctionSpeedDuration).toString().toUInt();

    speedRoot.skipCurrentElement();

    return true;
}

bool Function::saveXMLSpeed(QXmlStreamWriter *doc) const
{
    doc->writeStartElement(KXMLQLCFunctionSpeed);
    doc->writeAttribute(KXMLQLCFunctionSpeedFadeIn, QString::number(fadeInSpeed()));
    doc->writeAttribute(KXMLQLCFunctionSpeedFadeOut, QString::number(fadeOutSpeed()));
    doc->writeAttribute(KXMLQLCFunctionSpeedDuration, QString::number(duration()));
    doc->writeEndElement();

    return true;
}

uint Function::infiniteSpeed()
{
    return (uint) -2;
}

uint Function::defaultSpeed()
{
    return (uint) -1;
}

/*****************************************************************************
 * UI State
 *****************************************************************************/

QVariant Function::uiStateValue(QString property)
{
    return m_uiState.value(property, QVariant());
}

void Function::setUiStateValue(QString property, QVariant value)
{
    m_uiState[property] = value;
}

QMap<QString, QVariant> Function::uiStateMap() const
{
    return m_uiState;
}

/*****************************************************************************
 * Fixtures
 *****************************************************************************/

void Function::slotFixtureRemoved(quint32 fid)
{
    Q_UNUSED(fid);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/
bool Function::saveXML(QXmlStreamWriter *doc)
{
    Q_UNUSED(doc)
    return false;
}

bool Function::loadXML(QXmlStreamReader &root)
{
    Q_UNUSED(root)
    return false;
}

bool Function::loader(QXmlStreamReader &root, Doc* doc)
{
    if (root.name() != KXMLQLCFunction)
    {
        qWarning("Function node not found!");
        return false;
    }

    QXmlStreamAttributes attrs = root.attributes();

    /* Get common information from the tag's attributes */
    quint32 id = attrs.value(KXMLQLCFunctionID).toString().toUInt();
    QString name = attrs.value(KXMLQLCFunctionName).toString();
    Type type = Function::stringToType(attrs.value(KXMLQLCFunctionType).toString());
    QString path;
    bool visible = true;
    Universe::BlendMode blendMode = Universe::NormalBlend;

    if (attrs.hasAttribute(KXMLQLCFunctionPath))
        path = attrs.value(KXMLQLCFunctionPath).toString();
    if (attrs.hasAttribute(KXMLQLCFunctionHidden))
        visible = false;
    if (attrs.hasAttribute(KXMLQLCFunctionBlendMode))
        blendMode = Universe::stringToBlendMode(attrs.value(KXMLQLCFunctionBlendMode).toString());

    /* Check for ID validity before creating the function */
    if (id == Function::invalidId())
    {
        qWarning() << Q_FUNC_INFO << "Function ID" << id << "is not allowed.";
        return false;
    }

    /* Create a new function according to the type */
    Function* function = NULL;
    if (type == Function::SceneType)
        function = new class Scene(doc);
    else if (type == Function::ChaserType)
        function = new class Chaser(doc);
    else if (type == Function::CollectionType)
        function = new class Collection(doc);
    else if (type == Function::EFXType)
        function = new class EFX(doc);
    else if (type == Function::ScriptType)
        function = new class Script(doc);
    else if (type == Function::RGBMatrixType)
        function = new class RGBMatrix(doc);
    else if (type == Function::ShowType)
        function = new class Show(doc);
    else if (type == Function::SequenceType)
        function = new class Sequence(doc);
    else if (type == Function::AudioType)
        function = new class Audio(doc);
    else if (type == Function::VideoType)
        function = new class Video(doc);
    else
        return false;

    function->setName(name);
    function->setPath(path);
    function->setVisible(visible);
    function->setBlendMode(blendMode);
    if (function->loadXML(root) == true)
    {
        if (doc->addFunction(function, id) == true)
        {
            /* Success */
            return true;
        }
        else
        {
            qWarning() << "Function" << name << "cannot be created.";
            delete function;
            return false;
        }
    }
    else
    {
        qWarning() << "Function" << name << "cannot be loaded.";
        delete function;
        return false;
    }
}

void Function::postLoad()
{
    /* NOP */
}

bool Function::contains(quint32 functionId)
{
    Q_UNUSED(functionId);
    return false;
}

QList<quint32> Function::components()
{
    return QList<quint32>();
}

/*****************************************************************************
 * Flash
 *****************************************************************************/

void Function::flash(MasterTimer *timer, bool shouldOverride, bool forceLTP)
{
    Q_UNUSED(timer);
    Q_UNUSED(shouldOverride);
    Q_UNUSED(forceLTP);

    if (m_flashing == false)
    {
        emit flashing(m_id, true);
    }

    m_flashing = true;
}

void Function::unFlash(MasterTimer *timer)
{
    Q_UNUSED(timer);

    if (m_flashing == true)
        emit flashing(m_id, false);

    m_flashing = false;
}

bool Function::flashing() const
{
    return m_flashing;
}

/*****************************************************************************
 * Running
 *****************************************************************************/

void Function::preRun(MasterTimer *timer)
{
    Q_UNUSED(timer);

    qDebug() << "Function preRun. Name:" << m_name << "ID:" << m_id << "type:" << typeToString(type());
    m_running = true;

    emit running(m_id);
}

void Function::write(MasterTimer *timer, QList<Universe *> universes)
{
    Q_UNUSED(timer);
    Q_UNUSED(universes);
}

void Function::postRun(MasterTimer *timer, QList<Universe *> universes)
{
    Q_UNUSED(timer);
    Q_UNUSED(universes);

    qDebug() << "Function postRun. Name:" << m_name << "ID: " << m_id;

    {
        QMutexLocker locker(&m_stopMutex);

        resetElapsed();
        if (m_preserveAttributes == false)
            resetAttributes();

        m_functionStopped.wakeAll();
    }

    m_paused = false;
    m_running = false;
    emit stopped(m_id);
}

void Function::dismissAllFaders()
{
    QMapIterator <quint32, QSharedPointer<GenericFader> > it(m_fadersMap);
    while (it.hasNext() == true)
    {
        it.next();
        QSharedPointer<GenericFader> fader = it.value();
        if (!fader.isNull())
            fader->requestDelete();
    }

    m_fadersMap.clear();
}

bool Function::isRunning() const
{
    return m_running;
}

bool Function::isPaused() const
{
    return m_paused;
}

/*****************************************************************************
 * Elapsed ticks while running
 *****************************************************************************/

quint32 Function::elapsed() const
{
    return m_elapsed;
}

quint32 Function::elapsedBeats() const
{
    return m_elapsedBeats;
}

void Function::resetElapsed()
{
    qDebug() << Q_FUNC_INFO;
    m_elapsed = 0;
    m_elapsedBeats = 0;
}

void Function::incrementElapsed()
{
    // Don't wrap around. UINT_MAX is the maximum fade/hold time.
    if (m_elapsed < UINT_MAX - MasterTimer::tick())
        m_elapsed += MasterTimer::tick();
    else
        m_elapsed = UINT_MAX;
}

void Function::incrementElapsedBeats()
{
    m_elapsedBeats += 1000;
}

void Function::roundElapsed(quint32 roundTime)
{
    if (roundTime == 0)
        m_elapsed = 0;
    else
        m_elapsed %= roundTime;
}

/*****************************************************************************
 * Start & Stop
 *****************************************************************************/

void Function::start(MasterTimer* timer, FunctionParent source, quint32 startTime,
                     uint overrideFadeIn, uint overrideFadeOut, uint overrideDuration, TempoType overrideTempoType)
{
    qDebug() << "Function start(). Name:" << m_name << "ID: " << m_id << "source:" << source.type() << source.id() << ", startTime:" << startTime;

    Q_ASSERT(timer != NULL);

    {
        QMutexLocker sourcesLocker(&m_sourcesMutex);
        if (m_sources.contains(source))
            return;
        m_sources.append(source);
        if (m_sources.size() > 1)
            return;
    }

    /** If we're in a paused state, then just return to the running state
     *  to let subclasses resuming what they were doing. */
    if (m_paused == true)
    {
        m_paused = false;
        return;
    }

    m_elapsed = startTime;
    m_elapsedBeats = 0;
    m_overrideFadeInSpeed = overrideFadeIn;
    m_overrideFadeOutSpeed = overrideFadeOut;
    m_overrideDuration = overrideDuration;
    m_overrideTempoType = overrideTempoType == Original ? tempoType() : overrideTempoType;

    m_stop = false;
    timer->startFunction(this);
}

void Function::setPause(bool enable)
{
    if (enable && isRunning() == false)
        return;

    m_paused = enable;
}

void Function::stop(FunctionParent source, bool preserveAttributes)
{
    qDebug() << "Function stop(). Name:" << m_name << "ID: " << m_id << "source:" << source.type() << source.id();

    QMutexLocker sourcesLocker(&m_sourcesMutex);

    if ((source.id() == id() && source.type() == FunctionParent::Function) ||
        (source.type() == FunctionParent::Master) ||
        (source.type() == FunctionParent::ManualVCWidget))
    {
        m_sources.clear();
    }
    else
    {
        m_sources.removeAll(source);
    }

    if (m_sources.size() == 0)
    {
        m_stop = true;
        m_preserveAttributes = preserveAttributes;
    }
}

bool Function::stopped() const
{
    return m_stop;
}

bool Function::startedAsChild() const
{
    QMutexLocker sourcesLocker(const_cast<QMutex*>(&m_sourcesMutex));
    foreach (FunctionParent source, m_sources)
    {
        if (source.type() == FunctionParent::Function && source.id() != id())
            return true;
    }
    return false;
}

int Function::invalidAttributeId()
{
    return -1;
}

bool Function::stopAndWait()
{
    QMutexLocker locker(&m_stopMutex);

    stop(FunctionParent::master());

    QElapsedTimer watchdog;
    watchdog.restart();

    // block thread for maximum 2 seconds
    while (m_running == true)
    {
        if (watchdog.elapsed() > 2000)
            return false;

        // wait until the function has stopped
        m_functionStopped.wait(&m_stopMutex, 100);
    }

    return true;
}

/*****************************************************************************
 * Attributes
 *****************************************************************************/

int Function::registerAttribute(QString name, int flags, qreal min, qreal max, qreal value)
{
    for (int i = 0; i < m_attributes.count(); i++)
    {
        if (m_attributes[i].m_name == name)
        {
            m_attributes[i].m_min = min;
            m_attributes[i].m_max = max;
            m_attributes[i].m_value = value;
            m_attributes[i].m_flags = flags;
            m_attributes[i].m_isOverridden = false;
            m_attributes[i].m_overrideValue = 0.0;
            return i;
        }
    }
    Attribute newAttr;
    newAttr.m_name = name;
    newAttr.m_min = min;
    newAttr.m_max = max;
    newAttr.m_value = value;
    newAttr.m_flags = flags;
    newAttr.m_isOverridden = false;
    newAttr.m_overrideValue = 0.0;
    m_attributes.append(newAttr);

    return m_attributes.count() - 1;
}

int Function::requestAttributeOverride(int attributeIndex, qreal value)
{
    if (attributeIndex < 0 || attributeIndex >= m_attributes.count())
        return -1;

    int attributeID = invalidAttributeId();

    if (m_attributes.at(attributeIndex).m_flags & Single)
    {
        foreach (int id, m_overrideMap.keys())
        {
            if (m_overrideMap[id].m_attrIndex == attributeIndex)
            {
                attributeID = id;
                break;
            }
        }
    }

    if (attributeID == invalidAttributeId())
    {
        AttributeOverride override;
        override.m_attrIndex = attributeIndex;
        override.m_value = 0.0;

        attributeID = m_lastOverrideAttributeId;
        m_overrideMap[attributeID] = override;

        qDebug() << name() << "Override requested for new attribute" << attributeIndex << "value" << value << "new ID" << attributeID;

        calculateOverrideValue(attributeIndex);

        m_lastOverrideAttributeId++;
    }
    else
    {
        qDebug() << name() << "Override requested for existing attribute" << attributeIndex << "value" << value << "single ID" << attributeID;
    }

    // actually apply the new override value
    adjustAttribute(value, attributeID);

    return attributeID;
}

void Function::releaseAttributeOverride(int attributeId)
{
    if (m_overrideMap.contains(attributeId) == false)
        return;

    int attributeIndex = m_overrideMap[attributeId].m_attrIndex;

    m_overrideMap.remove(attributeId);

    calculateOverrideValue(attributeIndex);

    qDebug() << name() << "Attribute override released" << attributeId;
}

bool Function::unregisterAttribute(QString name)
{
    for (int i = 0; i < m_attributes.count(); i++)
    {
        if (m_attributes[i].m_name == name)
        {
            m_attributes.removeAt(i);
            return true;
        }
    }
    return false;
}

bool Function::renameAttribute(int idx, QString newName)
{
    if (idx < 0 || idx >= m_attributes.count())
        return false;
    m_attributes[idx].m_name = newName;

    return true;
}

int Function::adjustAttribute(qreal value, int attributeId)
{
    if (attributeId < 0)
        return -1;

    int attrIndex;

    //qDebug() << name() << "Attribute ID:" << attributeId << ", val:" << value;

    if (attributeId < OVERRIDE_ATTRIBUTE_START_ID)
    {
        if (attributeId >= m_attributes.count() || m_attributes[attributeId].m_value == value)
            return -1;

        // Adjust the original value of an attribute. Only Function editors should do this!
        m_attributes[attributeId].m_value = CLAMP(value, m_attributes[attributeId].m_min, m_attributes[attributeId].m_max);
        attrIndex = attributeId;
    }
    else
    {
        if (m_overrideMap.contains(attributeId) == false)
            return -1;

        // Adjust an attribute override value and recalculate the final overridden value
        m_overrideMap[attributeId].m_value = value;
        attrIndex = m_overrideMap[attributeId].m_attrIndex;
        calculateOverrideValue(attrIndex);
    }

    emit attributeChanged(attrIndex, m_attributes[attrIndex].m_isOverridden ?
                                     m_attributes[attrIndex].m_overrideValue :
                                     m_attributes[attrIndex].m_value);

    return attrIndex;
}

void Function::resetAttributes()
{
    for (int i = 0; i < m_attributes.count(); i++)
    {
        m_attributes[i].m_isOverridden = false;
        m_attributes[i].m_overrideValue = 0.0;
    }
    m_overrideMap.clear();
    m_lastOverrideAttributeId = OVERRIDE_ATTRIBUTE_START_ID;
}

qreal Function::getAttributeValue(int attributeIndex) const
{
    if (attributeIndex >= m_attributes.count())
        return 0.0;

    return m_attributes[attributeIndex].m_isOverridden ?
                m_attributes[attributeIndex].m_overrideValue :
                m_attributes[attributeIndex].m_value;
}

int Function::getAttributeIndex(QString name) const
{
    for (int i = 0; i < m_attributes.count(); i++)
    {
        Attribute attr = m_attributes.at(i);
        if (attr.m_name == name)
            return i;
    }
    return -1;
}

QList<Attribute> Function::attributes() const
{
    return m_attributes;
}

void Function::calculateOverrideValue(int attributeIndex)
{
    if (attributeIndex >= m_attributes.count())
        return;

    qreal finalValue = 0.0;
    bool found = false;
    Attribute origAttr = m_attributes.at(attributeIndex);

    if (origAttr.m_flags & Multiply)
        finalValue = origAttr.m_value;

    foreach (AttributeOverride attr, m_overrideMap.values())
    {
        if (attr.m_attrIndex != attributeIndex)
            continue;

        found = true;

        if (origAttr.m_flags & Multiply)
            finalValue = finalValue * attr.m_value;
        else if (origAttr.m_flags & LastWins)
            finalValue = attr.m_value;
    }

    m_attributes[attributeIndex].m_overrideValue = finalValue;
    m_attributes[attributeIndex].m_isOverridden = found;
}

/*************************************************************************
 * Blend mode
 *************************************************************************/

void Function::setBlendMode(Universe::BlendMode mode)
{
    m_blendMode = mode;
}

Universe::BlendMode Function::blendMode() const
{
    return m_blendMode;
}
