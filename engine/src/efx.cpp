/*
  Q Light Controller Plus
  efx.cpp

  Copyright (C) Heikki Junnila
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

#include <QVector>
#include <QDebug>
#include <QList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <math.h>

#include "genericfader.h"
#include "mastertimer.h"
#include "qlcmacros.h"
#include "doc.h"
#include "efx.h"
#include "bus.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

EFX::EFX(Doc* doc)
    : Function(doc, Function::EFXType)
    , m_algorithm(EFX::Circle)
    , m_isRelative(false)
    , m_xFrequency(2)
    , m_yFrequency(3)
    , m_xPhase(M_PI / 2.0)
    , m_yPhase(0)
    , m_propagationMode(Parallel)
    , m_legacyFadeBus(Bus::invalid())
    , m_legacyHoldBus(Bus::invalid())
{
    updateRotationCache();
    setName(tr("New EFX"));
    setDuration(20000); // 20s

    registerAttribute(tr("Width"), Function::LastWins, 0.0, 127.0, 127.0);
    registerAttribute(tr("Height"), Function::LastWins, 0.0, 127.0, 127.0);
    registerAttribute(tr("Rotation"), Function::LastWins, 0.0, 359.0, 0.0);
    registerAttribute(tr("X Offset"), Function::LastWins, 0.0, 255.0, 127.0);
    registerAttribute(tr("Y Offset"), Function::LastWins, 0.0, 255.0, 127.0);
    registerAttribute(tr("Start Offset"), Function::LastWins, 0.0, 359.0, 0.0);
}

EFX::~EFX()
{
    while (m_fixtures.isEmpty() == false)
        delete m_fixtures.takeFirst();
}

QIcon EFX::getIcon() const
{
    return QIcon(":/efx.png");
}

/*****************************************************************************
 * Copying
 *****************************************************************************/

Function* EFX::createCopy(Doc* doc, bool addToDoc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new EFX(doc);
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

bool EFX::copyFrom(const Function* function)
{
    const EFX* efx = qobject_cast<const EFX*> (function);
    if (efx == NULL)
        return false;

    while (m_fixtures.isEmpty() == false)
        delete m_fixtures.takeFirst();

    QListIterator <EFXFixture*> it(efx->m_fixtures);
    while (it.hasNext() == true)
    {
        EFXFixture* ef = new EFXFixture(this);
        ef->copyFrom(it.next());
        m_fixtures.append(ef);
    }

    m_propagationMode = efx->m_propagationMode;

    for (int i = 0; i < efx->attributes().count(); i++)
        adjustAttribute(efx->attributes().at(i).m_value, i);

    m_isRelative = efx->m_isRelative;

    updateRotationCache();

    m_xFrequency = efx->m_xFrequency;
    m_yFrequency = efx->m_yFrequency;
    m_xPhase = efx->m_xPhase;
    m_yPhase = efx->m_yPhase;

    m_algorithm = efx->m_algorithm;

    return Function::copyFrom(function);
}

void EFX::setDuration(uint ms)
{
    Function::setDuration(ms);

    for (int i = 0; i < m_fixtures.size(); ++i)
        m_fixtures[i]->durationChanged();

    emit durationChanged(ms);
}

uint EFX::loopDuration() const
{
    uint fadeIn = overrideFadeInSpeed() == defaultSpeed() ? fadeInSpeed() : overrideFadeInSpeed();

    return duration() - fadeIn;
}

/*****************************************************************************
 * Algorithm
 *****************************************************************************/

EFX::Algorithm EFX::algorithm() const
{
    return m_algorithm;
}

void EFX::setAlgorithm(EFX::Algorithm algo)
{
    if (algo == m_algorithm)
        return;

    if (algo >= EFX::Circle && algo <= EFX::Lissajous)
        m_algorithm = algo;
    else
        m_algorithm = EFX::Circle;

    emit changed(this->id());
}

QStringList EFX::algorithmList()
{
    QStringList list;
    list << algorithmToString(EFX::Circle);
    list << algorithmToString(EFX::Eight);
    list << algorithmToString(EFX::Line);
    list << algorithmToString(EFX::Line2);
    list << algorithmToString(EFX::Diamond);
    list << algorithmToString(EFX::Square);
    list << algorithmToString(EFX::SquareChoppy);
	list << algorithmToString(EFX::SquareTrue);
    list << algorithmToString(EFX::Leaf);
    list << algorithmToString(EFX::Lissajous);
    return list;
}

QString EFX::algorithmToString(EFX::Algorithm algo)
{
    switch (algo)
    {
        default:
        case EFX::Circle:
            return QString(KXMLQLCEFXCircleAlgorithmName);
        case EFX::Eight:
            return QString(KXMLQLCEFXEightAlgorithmName);
        case EFX::Line:
            return QString(KXMLQLCEFXLineAlgorithmName);
        case EFX::Line2:
            return QString(KXMLQLCEFXLine2AlgorithmName);
        case EFX::Diamond:
            return QString(KXMLQLCEFXDiamondAlgorithmName);
        case EFX::Square:
            return QString(KXMLQLCEFXSquareAlgorithmName);
        case EFX::SquareChoppy:
            return QString(KXMLQLCEFXSquareChoppyAlgorithmName);
		 case EFX::SquareTrue:
            return QString(KXMLQLCEFXSquareTrueAlgorithmName);
        case EFX::Leaf:
            return QString(KXMLQLCEFXLeafAlgorithmName);
        case EFX::Lissajous:
            return QString(KXMLQLCEFXLissajousAlgorithmName);
    }
}

EFX::Algorithm EFX::stringToAlgorithm(const QString& str)
{
    if (str == QString(KXMLQLCEFXEightAlgorithmName))
        return EFX::Eight;
    else if (str == QString(KXMLQLCEFXLineAlgorithmName))
        return EFX::Line;
    else if (str == QString(KXMLQLCEFXLine2AlgorithmName))
        return EFX::Line2;
    else if (str == QString(KXMLQLCEFXDiamondAlgorithmName))
        return EFX::Diamond;
    else if (str == QString(KXMLQLCEFXSquareAlgorithmName))
        return EFX::Square;
    else if (str == QString(KXMLQLCEFXSquareChoppyAlgorithmName))
        return EFX::SquareChoppy;
	 else if (str == QString(KXMLQLCEFXSquareTrueAlgorithmName))
        return EFX::SquareTrue;
    else if (str == QString(KXMLQLCEFXLeafAlgorithmName))
        return EFX::Leaf;
    else if (str == QString(KXMLQLCEFXLissajousAlgorithmName))
        return EFX::Lissajous;
    else
        return EFX::Circle;
}

void EFX::preview(QPolygonF &polygon) const
{
    preview(polygon, Function::Forward, 0);
}

void EFX::previewFixtures(QVector <QPolygonF>& polygons) const
{
    polygons.resize(m_fixtures.size());
    for (int i = 0; i < m_fixtures.size(); ++i)
        preview(polygons[i], m_fixtures[i]->m_direction, m_fixtures[i]->m_startOffset);
}

void EFX::preview(QPolygonF &polygon, Function::Direction direction, int startOffset) const
{
    float stepCount = 128.0;
    int step = 0;
    float stepSize = 1.0 / (stepCount / (M_PI * 2.0));

    float i = 0;
    float x = 0;
    float y = 0;

    /* Reset the polygon to fill it with new values */
    polygon.clear();

    /* Draw a preview of the effect */
    for (step = 0; step < stepCount; step++)
    {
        calculatePoint(direction, startOffset, i, &x, &y);
        polygon << QPointF(x, y);
        i += stepSize;
    }
}

void EFX::calculatePoint(Function::Direction direction, int startOffset, float iterator, float *x, float *y) const
{
    iterator = calculateDirection(direction, iterator);
    iterator += convertOffset(startOffset + getAttributeValue(StartOffset));

    if (iterator >= M_PI * 2.0)
        iterator -= M_PI * 2.0;

    calculatePoint(iterator, x, y);
}

void EFX::rotateAndScale(float *x, float *y) const
{
    float xx = *x;
    float yy = *y;
    float w = getAttributeValue(Width);
    float h = getAttributeValue(Height);
    float fadeScale = 1.0;

    if (isRunning())
    {
        uint fadeIn = overrideFadeInSpeed() == defaultSpeed() ? fadeInSpeed() : overrideFadeInSpeed();
        if (fadeIn > 0 && elapsed() <= fadeIn)
        {
            fadeScale = SCALE(float(elapsed()),
                              float(0), float(fadeIn),
                              float(0), float(1.0));
        }
    }

    *x = (getAttributeValue(XOffset)) + xx * m_cosR * (w * fadeScale) + yy * m_sinR * (h * fadeScale);
    *y = (getAttributeValue(YOffset)) + -xx * m_sinR * (w * fadeScale) + yy * m_cosR * (h * fadeScale);
}

float EFX::calculateDirection(Function::Direction direction, float iterator) const
{
    if (direction == this->direction())
        return iterator;

    switch (algorithm())
    {
    default:
    case Circle:
    case Eight:
    case Line2:
    case Diamond:
    case Square:
    case SquareChoppy:
	case SquareTrue:
    case Leaf:
    case Lissajous:
        return (M_PI * 2.0) - iterator;
    case Line:
        return (iterator > M_PI) ? (iterator - M_PI) : (iterator + M_PI);
    }
}

// this function should map from 0..M_PI * 2 -> -1..1
void EFX::calculatePoint(float iterator, float *x, float *y) const
{
    switch (algorithm())
    {
    default:
    case Circle:
        *x = cos(iterator + M_PI_2);
        *y = cos(iterator);
        break;

    case Eight:
        *x = cos((iterator * 2) + M_PI_2);
        *y = cos(iterator);
        break;

    case Line:
        *x = cos(iterator);
        *y = cos(iterator);
        break;

    case Line2:
        *x = iterator / M_PI - 1;
        *y = iterator / M_PI - 1;
        break;

    case Diamond:
        *x = pow(cos(iterator - M_PI_2), 3);
        *y = pow(cos(iterator), 3);
        break;

    case Square:
        if (iterator < M_PI / 2)
        {
            *x = (iterator * 2 / M_PI) * 2 - 1;
            *y = 1;
        }
        else if (M_PI / 2 <= iterator && iterator < M_PI)
        {
            *x = 1;
            *y = (1 - (iterator - M_PI / 2) * 2 / M_PI) * 2 - 1;
        }
        else if (M_PI <= iterator && iterator < M_PI * 3 / 2)
        {
            *x = (1 - (iterator - M_PI) * 2 / M_PI) * 2 - 1;
            *y = -1;
        }
        else // M_PI * 3 / 2 <= iterator
        {
            *x = -1;
            *y = ((iterator - M_PI * 3 / 2) * 2 / M_PI) * 2 - 1;
        }
        break;

    case SquareChoppy:
        *x = round(cos(iterator));
        *y = round(sin(iterator));
        break;
		
	case SquareTrue:
        if (iterator < M_PI / 2)
        {
            *x = 1;
            *y = 1;
        }
        else if (M_PI / 2 <= iterator && iterator < M_PI)
        {
            *x = 1;
            *y = -1;
        }
        else if (M_PI <= iterator && iterator < M_PI * 3 / 2)
        {
            *x = -1;
            *y = -1;
        }
        else // M_PI * 3 / 2 <= iterator
        {
            *x = -1;
            *y = 1;
        }
		break;

    case Leaf:
        *x = pow(cos(iterator + M_PI_2), 5);
        *y = cos(iterator);
        break;

    case Lissajous:
        {
            if (m_xFrequency > 0)
                *x = cos((m_xFrequency * iterator) - m_xPhase);
            else
            {
                float iterator0 = ((iterator + m_xPhase) / M_PI);
                int fff = iterator0;
                iterator0 -= (fff - fff % 2);
                float forward = 1 - floor(iterator0); // 1 when forward
                float backward = 1 - forward; // 1 when backward
                iterator0 = iterator0 - floor(iterator0);
                *x = (forward * iterator0 + backward * (1 - iterator0)) * 2 - 1;
            }
            if (m_yFrequency > 0)
                *y = cos((m_yFrequency * iterator) - m_yPhase);
            else
            {
                float iterator0 = ((iterator + m_yPhase) / M_PI);
                int fff = iterator0;
                iterator0 -= (fff - fff % 2);
                float forward = 1 - floor(iterator0); // 1 when forward
                float backward = 1 - forward; // 1 when backward
                iterator0 = iterator0 - floor(iterator0);
                *y = (forward * iterator0 + backward * (1 - iterator0)) * 2 - 1;
            }
        }
        break;
    }

    rotateAndScale(x, y);
}

/*****************************************************************************
 * Width
 *****************************************************************************/

void EFX::setWidth(int width)
{
    adjustAttribute(static_cast<double> (CLAMP(width, 0, 127)), Width);
    emit changed(this->id());
}

int EFX::width() const
{
    return static_cast<int> (attributes().at(Width).m_value);
}

/*****************************************************************************
 * Height
 *****************************************************************************/

void EFX::setHeight(int height)
{
    adjustAttribute(static_cast<double> (CLAMP(height, 0, 127)), Height);
    emit changed(this->id());
}

int EFX::height() const
{
    return static_cast<int> (attributes().at(Height).m_value);
}

/*****************************************************************************
 * Rotation
 *****************************************************************************/

void EFX::setRotation(int rot)
{
    adjustAttribute(CLAMP(rot, 0, 359), Rotation);
    updateRotationCache();
    emit changed(this->id());
}

int EFX::rotation() const
{
    return static_cast<int> (attributes().at(Rotation).m_value);
}

void EFX::updateRotationCache()
{
    double r = M_PI/180 * getAttributeValue(Rotation);
    m_cosR = cos(r);
    m_sinR = sin(r);
}

/*****************************************************************************
 * Start Offset
 *****************************************************************************/

void EFX::setStartOffset(int startOffset)
{
    adjustAttribute(CLAMP(startOffset, 0, 359), StartOffset);
    emit changed(this->id());
}

int EFX::startOffset() const
{
    return static_cast<int> (attributes().at(StartOffset).m_value);
}

float EFX::convertOffset(int offset) const
{
    return M_PI/180 * (offset % 360);
}

/*****************************************************************************
 * Is Relative
 *****************************************************************************/

void EFX::setIsRelative(bool isRelative)
{
    m_isRelative = isRelative;
    emit changed(this->id());
}

bool EFX::isRelative() const
{
    return m_isRelative;
}

/*****************************************************************************
 * Offset
 *****************************************************************************/

void EFX::setXOffset(int offset)
{
    adjustAttribute(static_cast<double> (CLAMP(offset, 0, (int)UCHAR_MAX)), XOffset);
    emit changed(this->id());
}

int EFX::xOffset() const
{
    return static_cast<int> (attributes().at(XOffset).m_value);
}

void EFX::setYOffset(int offset)
{
    adjustAttribute(static_cast<double> (CLAMP(offset, 0, (int)UCHAR_MAX)), YOffset);
    emit changed(this->id());
}

int EFX::yOffset() const
{
    return static_cast<int> (attributes().at(YOffset).m_value);
}

/*****************************************************************************
 * Frequency
 *****************************************************************************/

void EFX::setXFrequency(int freq)
{
    m_xFrequency = static_cast<float> (CLAMP(freq, 0, 32));
    emit changed(this->id());
}

int EFX::xFrequency() const
{
    return static_cast<int> (m_xFrequency);
}

void EFX::setYFrequency(int freq)
{
    m_yFrequency = static_cast<float> (CLAMP(freq, 0, 32));
    emit changed(this->id());
}

int EFX::yFrequency() const
{
    return static_cast<int> (m_yFrequency);
}

bool EFX::isFrequencyEnabled()
{
    if (m_algorithm == EFX::Lissajous)
        return true;
    else
        return false;
}

/*****************************************************************************
 * Phase
 *****************************************************************************/

void EFX::setXPhase(int phase)
{
    m_xPhase = static_cast<float> (CLAMP(phase, 0, 359)) * M_PI / 180.0;
    emit changed(this->id());
}

int EFX::xPhase() const
{
    return static_cast<int> (floor((m_xPhase * 180.0 / M_PI) + 0.5));
}

void EFX::setYPhase(int phase)
{
    m_yPhase = static_cast<float> (CLAMP(phase, 0, 359)) * M_PI / 180.0;
    emit changed(this->id());
}

int EFX::yPhase() const
{
    return static_cast<int> (floor((m_yPhase * 180.0 / M_PI) + 0.5));
}

bool EFX::isPhaseEnabled() const
{
    if (m_algorithm == EFX::Lissajous)
        return true;
    else
        return false;
}

/*****************************************************************************
 * Fixtures
 *****************************************************************************/

bool EFX::addFixture(EFXFixture* ef)
{
    Q_ASSERT(ef != NULL);

    /* Search for an existing fixture with the same ID and append at last but do
     * not prevent multiple entries because a fixture can have multiple efx. */
    //! @todo Prevent multiple entries using head & mode
    int i;
    for (i = 0; i < m_fixtures.size (); i++)
    {
        if (m_fixtures[i]->head() == ef->head())
        {
            m_fixtures.insert(i, ef);
            break;
        }
    }

    /* If not inserted, put the EFXFixture object into our list */
    if (i >= m_fixtures.size())
        m_fixtures.append(ef);

    emit changed(this->id());

    return true;
}

bool EFX::addFixture(quint32 fxi, int head)
{
    EFXFixture *ef = new EFXFixture(this);
    GroupHead gHead(fxi, head);
    ef->setHead(gHead);

    return addFixture(ef);
}

bool EFX::removeFixture(EFXFixture* ef)
{
    Q_ASSERT(ef != NULL);

    if (m_fixtures.removeAll(ef) > 0)
    {
        emit changed(this->id());
        return true;
    }
    else
    {
        return false;
    }
}

bool EFX::removeFixture(quint32 fxi, int head)
{
    for (int i = 0; i < m_fixtures.count(); i++)
    {
        EFXFixture *ef = m_fixtures.at(i);
        if (ef->head().fxi == fxi && ef->head().head == head)
        {
            m_fixtures.removeAt(i);
            return true;
        }
    }

    return false;
}

void EFX::removeAllFixtures()
{
    m_fixtures.clear();
    emit changed(this->id());
}

bool EFX::raiseFixture(EFXFixture* ef)
{
    Q_ASSERT(ef != NULL);

    int index = m_fixtures.indexOf(ef);
    if (index > 0)
    {
        m_fixtures.move(index, index - 1);
        emit changed(this->id());
        return true;
    }
    else
    {
        return false;
    }
}

bool EFX::lowerFixture(EFXFixture* ef)
{
    int index = m_fixtures.indexOf(ef);
    if (index < (m_fixtures.count() - 1))
    {
        m_fixtures.move(index, index + 1);
        emit changed(this->id());
        return true;
    }
    else
    {
        return false;
    }
}

const QList <EFXFixture*> EFX::fixtures() const
{
    return m_fixtures;
}

EFXFixture *EFX::fixture(quint32 id, int headIndex)
{
    foreach (EFXFixture *ef, m_fixtures)
    {
        if (ef->head().fxi == id && ef->head().head == headIndex)
            return ef;
    }

    return NULL;
}

QList<quint32> EFX::components()
{
    QList<quint32> ids;

    foreach (EFXFixture *ef, m_fixtures)
    {
        if (ids.contains(ef->head().fxi) == false)
            ids.append(ef->head().fxi);
    }

    return ids;
}

void EFX::slotFixtureRemoved(quint32 fxi_id)
{
    /* Remove the destroyed fixture from our list */
    QMutableListIterator <EFXFixture*> it(m_fixtures);
    while (it.hasNext() == true)
    {
        it.next();

        if (it.value()->head().fxi == fxi_id)
        {
            delete it.value();
            it.remove();
            break;
        }
    }
}

/*****************************************************************************
 * Fixture propagation mode
 *****************************************************************************/

void EFX::setPropagationMode(PropagationMode mode)
{
    m_propagationMode = mode;
    emit changed(this->id());
}

EFX::PropagationMode EFX::propagationMode() const
{
    return m_propagationMode;
}

QString EFX::propagationModeToString(PropagationMode mode)
{
    if (mode == Serial)
        return QString(KXMLQLCEFXPropagationModeSerial);
    else if (mode == Asymmetric)
        return QString(KXMLQLCEFXPropagationModeAsymmetric);
    else
        return QString(KXMLQLCEFXPropagationModeParallel);
}

EFX::PropagationMode EFX::stringToPropagationMode(QString str)
{
    if (str == QString(KXMLQLCEFXPropagationModeSerial))
        return Serial;
    else if (str == QString(KXMLQLCEFXPropagationModeAsymmetric))
        return Asymmetric;
    else
        return Parallel;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool EFX::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* Function tag */
    doc->writeStartElement(KXMLQLCFunction);

    /* Common attributes */
    saveXMLCommon(doc);

    /* Fixtures */
    QListIterator <EFXFixture*> it(m_fixtures);
    while (it.hasNext() == true)
        it.next()->saveXML(doc);

    /* Propagation mode */
    doc->writeTextElement(KXMLQLCEFXPropagationMode, propagationModeToString(m_propagationMode));

    /* Speeds */
    saveXMLSpeed(doc);
    /* Direction */
    saveXMLDirection(doc);
    /* Run order */
    saveXMLRunOrder(doc);

    /* Algorithm */
    doc->writeTextElement(KXMLQLCEFXAlgorithm, algorithmToString(algorithm()));
    /* Width */
    doc->writeTextElement(KXMLQLCEFXWidth, QString::number(width()));
    /* Height */
    doc->writeTextElement(KXMLQLCEFXHeight, QString::number(height()));
    /* Rotation */
    doc->writeTextElement(KXMLQLCEFXRotation, QString::number(rotation()));
    /* StartOffset */
    doc->writeTextElement(KXMLQLCEFXStartOffset, QString::number(startOffset()));
    /* IsRelative */
    doc->writeTextElement(KXMLQLCEFXIsRelative, QString::number(isRelative() ? 1 : 0));

    /********************************************
     * X-Axis
     ********************************************/
    doc->writeStartElement(KXMLQLCEFXAxis);
    doc->writeAttribute(KXMLQLCFunctionName, KXMLQLCEFXX);

    /* Offset */
    doc->writeTextElement(KXMLQLCEFXOffset, QString::number(xOffset()));
    /* Frequency */
    doc->writeTextElement(KXMLQLCEFXFrequency, QString::number(xFrequency()));
    /* Phase */
    doc->writeTextElement(KXMLQLCEFXPhase, QString::number(xPhase()));

    /* End the (X) <Axis> tag */
    doc->writeEndElement();

    /********************************************
     * Y-Axis
     ********************************************/
    doc->writeStartElement(KXMLQLCEFXAxis);
    doc->writeAttribute(KXMLQLCFunctionName, KXMLQLCEFXY);

    /* Offset */
    doc->writeTextElement(KXMLQLCEFXOffset, QString::number(yOffset()));
    /* Frequency */
    doc->writeTextElement(KXMLQLCEFXFrequency, QString::number(yFrequency()));
    /* Phase */
    doc->writeTextElement(KXMLQLCEFXPhase, QString::number(yPhase()));

    /* End the (Y) <Axis> tag */
    doc->writeEndElement();

    /* End the <Function> tag */
    doc->writeEndElement();

    return true;
}

bool EFX::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCFunction)
    {
        qWarning() << "Function node not found!";
        return false;
    }

    if (root.attributes().value(KXMLQLCFunctionType).toString() != typeToString(Function::EFXType))
    {
        qWarning("Function is not an EFX!");
        return false;
    }

    /* Load EFX contents */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCBus)
        {
            /* Bus */
            QString str = root.attributes().value(KXMLQLCBusRole).toString();
            if (str == KXMLQLCBusFade)
                m_legacyFadeBus = root.readElementText().toUInt();
            else if (str == KXMLQLCBusHold)
                m_legacyHoldBus = root.readElementText().toUInt();
        }
        else if (root.name() == KXMLQLCFunctionSpeed)
        {
            loadXMLSpeed(root);
        }
        else if (root.name() == KXMLQLCEFXFixture)
        {
            EFXFixture* ef = new EFXFixture(this);
            ef->loadXML(root);
            if (ef->head().isValid())
            {
                if (addFixture(ef) == false)
                    delete ef;
            }
        }
        else if (root.name() == KXMLQLCEFXPropagationMode)
        {
            /* Propagation mode */
            setPropagationMode(stringToPropagationMode(root.readElementText()));
        }
        else if (root.name() == KXMLQLCEFXAlgorithm)
        {
            /* Algorithm */
            setAlgorithm(stringToAlgorithm(root.readElementText()));
        }
        else if (root.name() == KXMLQLCFunctionDirection)
        {
            loadXMLDirection(root);
        }
        else if (root.name() == KXMLQLCFunctionRunOrder)
        {
            loadXMLRunOrder(root);
        }
        else if (root.name() == KXMLQLCEFXWidth)
        {
            /* Width */
            setWidth(root.readElementText().toInt());
        }
        else if (root.name() == KXMLQLCEFXHeight)
        {
            /* Height */
            setHeight(root.readElementText().toInt());
        }
        else if (root.name() == KXMLQLCEFXRotation)
        {
            /* Rotation */
            setRotation(root.readElementText().toInt());
        }
        else if (root.name() == KXMLQLCEFXStartOffset)
        {
            /* StartOffset */
            setStartOffset(root.readElementText().toInt());
        }
        else if (root.name() == KXMLQLCEFXIsRelative)
        {
            /* IsRelative */
            setIsRelative(root.readElementText().toInt() != 0);
        }
        else if (root.name() == KXMLQLCEFXAxis)
        {
            /* Axes */
            loadXMLAxis(root);
        }
        else
        {
            qWarning() << "Unknown EFX tag:" << root.name();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool EFX::loadXMLAxis(QXmlStreamReader &root)
{
    int frequency = 0;
    int offset = 0;
    int phase = 0;
    QString axis;

    if (root.name() != KXMLQLCEFXAxis)
    {
        qWarning() << "EFX axis node not found!";
        return false;
    }

    /* Get the axis name */
    axis = root.attributes().value(KXMLQLCFunctionName).toString();

    /* Load axis contents */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCEFXOffset)
            offset = root.readElementText().toInt();
        else if (root.name() == KXMLQLCEFXFrequency)
            frequency = root.readElementText().toInt();
        else if (root.name() == KXMLQLCEFXPhase)
            phase = root.readElementText().toInt();
        else
        {
            qWarning() << "Unknown EFX axis tag:" << root.name();
            root.skipCurrentElement();
        }
    }

    if (axis == KXMLQLCEFXY)
    {
        setYOffset(offset);
        setYFrequency(frequency);
        setYPhase(phase);
        return true;
    }
    else if (axis == KXMLQLCEFXX)
    {
        setXOffset(offset);
        setXFrequency(frequency);
        setXPhase(phase);
        return true;
    }
    else
    {
        qWarning() << "Unknown EFX axis:" << axis;
        return false;
    }
}

void EFX::postLoad()
{
    // Map legacy bus speeds to fixed speed values
    if (m_legacyFadeBus != Bus::invalid())
    {
        quint32 value = Bus::instance()->value(m_legacyFadeBus);
        setFadeInSpeed((value / MasterTimer::frequency()) * 1000);
        setFadeOutSpeed((value / MasterTimer::frequency()) * 1000);
    }

    if (m_legacyHoldBus != Bus::invalid())
    {
        quint32 value = Bus::instance()->value(m_legacyHoldBus);
        setDuration((value / MasterTimer::frequency()) * 1000);
    }
}

/*****************************************************************************
 * Running
 *****************************************************************************/
QSharedPointer<GenericFader> EFX::getFader(QList<Universe *> universes, quint32 universeID)
{
    // get the universe Fader first. If doesn't exist, create it
    QSharedPointer<GenericFader> fader = m_fadersMap.value(universeID, QSharedPointer<GenericFader>());
    if (fader.isNull())
    {
        fader = universes[universeID]->requestFader();
        fader->adjustIntensity(getAttributeValue(Intensity));
        fader->setBlendMode(blendMode());
        fader->setName(name());
        fader->setParentFunctionID(id());
        fader->setHandleSecondary(true);
        m_fadersMap[universeID] = fader;
    }

    return fader;
}

void EFX::preRun(MasterTimer* timer)
{
    int serialNumber = 0;

    QListIterator <EFXFixture*> it(m_fixtures);
    while (it.hasNext() == true)
    {
        EFXFixture *ef = it.next();
        Q_ASSERT(ef != NULL);
        ef->setSerialNumber(serialNumber++);
    }

    Function::preRun(timer);
}

void EFX::write(MasterTimer *timer, QList<Universe*> universes)
{
    Q_UNUSED(timer);

    if (isPaused())
        return;

    int done = 0;

    QListIterator <EFXFixture*> it(m_fixtures);
    while (it.hasNext() == true)
    {
        EFXFixture *ef = it.next();
        if (ef->isDone() == false)
        {
            QSharedPointer<GenericFader> fader = getFader(universes, ef->universe());
            ef->nextStep(universes, fader);
        }
        else
        {
            done++;
        }
    }

    incrementElapsed();

    /* Check for stop condition */
    if (done == m_fixtures.count())
        stop(FunctionParent::master());
}

void EFX::postRun(MasterTimer *timer, QList<Universe *> universes)
{
    /* Reset all fixtures */
    QListIterator <EFXFixture*> it(m_fixtures);
    while (it.hasNext() == true)
    {
        EFXFixture* ef(it.next());

        /* Run the EFX's stop scene for Loop & PingPong modes */
        if (runOrder() != SingleShot)
            ef->stop();
        ef->reset();
    }

    dismissAllFaders();

    Function::postRun(timer, universes);
}

/*****************************************************************************
 * Intensity
 *****************************************************************************/

int EFX::adjustAttribute(qreal fraction, int attributeId)
{
    int attrIndex = Function::adjustAttribute(fraction, attributeId);

    switch (attrIndex)
    {
        case Intensity:
        {
            foreach (QSharedPointer<GenericFader> fader, m_fadersMap.values())
            {
                if (!fader.isNull())
                    fader->adjustIntensity(getAttributeValue(Function::Intensity));
            }
        }
        break;

        case Height:
        case Width:
        case XOffset:
        case YOffset:
        case Rotation:
            updateRotationCache();
        break;
    }

    return attrIndex;
}

/*************************************************************************
 * Blend mode
 *************************************************************************/

void EFX::setBlendMode(Universe::BlendMode mode)
{
    if (mode == blendMode())
        return;

    foreach (QSharedPointer<GenericFader> fader, m_fadersMap.values())
    {
        if (!fader.isNull())
            fader->setBlendMode(mode);
    }

    Function::setBlendMode(mode);
}
