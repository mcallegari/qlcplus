/*
  Q Light Controller
  efx.cpp

  Copyright (C) Heikki Junnila

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
#include <QtXml>

#include <math.h>

#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "genericfader.h"
#include "qlcchannel.h"
#include "qlcmacros.h"
#include "qlcfile.h"

#include "universearray.h"
#include "mastertimer.h"
#include "fixture.h"
#include "scene.h"
#include "doc.h"
#include "efx.h"
#include "bus.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

EFX::EFX(Doc* doc) : Function(doc, Function::EFX)
{
    m_width = 127;
    m_height = 127;
    m_xOffset = 127;
    m_yOffset = 127;
    m_rotation = 0;
    m_startOffset = 0;
    m_isRelative = false;

    updateRotationCache();

    m_xFrequency = 2;
    m_yFrequency = 3;
    m_xPhase = M_PI / 2.0;
    m_yPhase = 0;

    m_propagationMode = Parallel;

    m_algorithm = EFX::Circle;

    setName(tr("New EFX"));

    m_fader = NULL;

    setDuration(20000); // 20s

    m_legacyHoldBus = Bus::invalid();
    m_legacyFadeBus = Bus::invalid();

    registerAttribute(tr("Width"));
    registerAttribute(tr("Height"));
    registerAttribute(tr("Rotation"));
    registerAttribute(tr("X Offset"));
    registerAttribute(tr("Y Offset"));
}

EFX::~EFX()
{
    while (m_fixtures.isEmpty() == false)
        delete m_fixtures.takeFirst();
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

    m_width = efx->m_width;
    m_height = efx->m_height;
    m_xOffset = efx->m_xOffset;
    m_yOffset = efx->m_yOffset;
    m_rotation = efx->m_rotation;
    m_startOffset = efx->m_startOffset;
    m_isRelative = efx->m_isRelative;

    updateRotationCache();

    m_xFrequency = efx->m_xFrequency;
    m_yFrequency = efx->m_yFrequency;
    m_xPhase = efx->m_xPhase;
    m_yPhase = efx->m_yPhase;

    m_algorithm = efx->m_algorithm;

    return Function::copyFrom(function);
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
    else if (str == QString(KXMLQLCEFXLissajousAlgorithmName))
        return EFX::Lissajous;
    else
        return EFX::Circle;
}

void EFX::preview(QVector <QPoint>& polygon) const
{
    preview(polygon, Function::Forward, 0);
}

void EFX::previewFixtures(QVector <QVector <QPoint> >& polygons) const
{
    polygons.resize(m_fixtures.size());
    for (int i = 0; i < m_fixtures.size(); ++i)
    { 
        preview(polygons[i], m_fixtures[i]->m_direction, m_fixtures[i]->m_startOffset);
    }
}

void EFX::preview(QVector <QPoint>& polygon, Function::Direction direction, int startOffset) const
{
    int stepCount = 128;
    int step = 0;
    qreal stepSize = (qreal)(1) / ((qreal)(stepCount) / (M_PI * 2.0));

    qreal i = 0;
    qreal x = 0;
    qreal y = 0;

    /* Resize the array to contain stepCount points */
    polygon.resize(stepCount);

    /* Draw a preview of the effect */
    for (step = 0; step < stepCount; step++)
    {
        calculatePoint(direction, startOffset, i, &x, &y);
        polygon[step] = QPoint(int(x), int(y));
        i += stepSize;
    }
}

void EFX::calculatePoint(Function::Direction direction, int startOffset, qreal iterator, qreal* x, qreal* y) const
{
    iterator = calculateDirection(direction, iterator);
    iterator += convertOffset(startOffset + m_startOffset);

    if (iterator >= M_PI * 2.0)
        iterator -= M_PI * 2.0;

    calculatePoint(iterator, x, y);
}

void EFX::rotateAndScale(qreal* x, qreal* y) const
{
    qreal xx = *x;
    qreal yy = *y;
    qreal w = m_width * getAttributeValue(Width);
    qreal h = m_height * getAttributeValue(Height);

    *x = (m_xOffset * getAttributeValue(XOffset)) + xx * m_cosR * w + yy * m_sinR * h;
    *y = (m_yOffset * getAttributeValue(YOffset)) + -xx * m_sinR * w + yy * m_cosR * h;
}

qreal EFX::calculateDirection(Function::Direction direction, qreal iterator) const
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
    case Lissajous:
        return (M_PI * 2.0) - iterator;
    case Line:
        return (iterator > M_PI) ? (iterator - M_PI) : (iterator + M_PI);
    }
}

// this function should map from 0..M_PI * 2 -> -1..1
void EFX::calculatePoint(qreal iterator, qreal* x, qreal* y) const
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

    case Lissajous:
        *x = cos((m_xFrequency * iterator) - m_xPhase);
        *y = cos((m_yFrequency * iterator) - m_yPhase);
        break;
    }

    rotateAndScale(x, y);
}

/*****************************************************************************
 * Width
 *****************************************************************************/

void EFX::setWidth(int width)
{
    m_width = static_cast<double> (CLAMP(width, 0, 127));
    emit changed(this->id());
}

int EFX::width() const
{
    return static_cast<int> (m_width);
}

/*****************************************************************************
 * Height
 *****************************************************************************/

void EFX::setHeight(int height)
{
    m_height = static_cast<double> (CLAMP(height, 0, 127));
    emit changed(this->id());
}

int EFX::height() const
{
    return static_cast<int> (m_height);
}

/*****************************************************************************
 * Rotation
 *****************************************************************************/

void EFX::setRotation(int rot)
{
    m_rotation = static_cast<int> (CLAMP(rot, 0, 359));
    updateRotationCache();
    emit changed(this->id());
}

int EFX::rotation() const
{
    return static_cast<int> (m_rotation);
}

void EFX::updateRotationCache()
{
    qreal r = M_PI/180 * m_rotation * getAttributeValue(Rotation);
    m_cosR = cos(r);
    m_sinR = sin(r);
}

/*****************************************************************************
 * Start Offset
 *****************************************************************************/

void EFX::setStartOffset(int startOffset)
{
    m_startOffset = CLAMP(startOffset, 0, 359);
    emit changed(this->id());
}

int EFX::startOffset() const
{
    return m_startOffset;
}

qreal EFX::convertOffset(int offset) const
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
    m_xOffset = static_cast<double> (CLAMP(offset, 0, UCHAR_MAX));
    emit changed(this->id());
}

int EFX::xOffset() const
{
    return static_cast<int> (m_xOffset);
}

void EFX::setYOffset(int offset)
{
    m_yOffset = static_cast<double> (CLAMP(offset, 0, UCHAR_MAX));
    emit changed(this->id());
}

int EFX::yOffset() const
{
    return static_cast<int> (m_yOffset);
}

/*****************************************************************************
 * Frequency
 *****************************************************************************/

void EFX::setXFrequency(int freq)
{
    m_xFrequency = static_cast<qreal> (CLAMP(freq, 0, 5));
    emit changed(this->id());
}

int EFX::xFrequency() const
{
    return static_cast<int> (m_xFrequency);
}

void EFX::setYFrequency(int freq)
{
    m_yFrequency = static_cast<qreal> (CLAMP(freq, 0, 5));
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
    m_xPhase = static_cast<qreal> (CLAMP(phase, 0, 359)) * M_PI / 180.0;
    emit changed(this->id());
}

int EFX::xPhase() const
{
    return static_cast<int> (floor((m_xPhase * 180.0 / M_PI) + 0.5));
}

void EFX::setYPhase(int phase)
{
    m_yPhase = static_cast<qreal> (CLAMP(phase, 0, 359)) * M_PI / 180.0;
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

    /* Search for an existing fixture with the same ID to prevent multiple
       entries of the same fixture. */
    QListIterator <EFXFixture*> it(m_fixtures);
    while (it.hasNext() == true)
    {
        /* Found the same fixture. Don't add the new one. */
        if (it.next()->head() == ef->head())
            return false;
    }

    /* Put the EFXFixture object into our list */
    m_fixtures.append(ef);

    emit changed(this->id());

    return true;
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

bool EFX::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement root;
    QDomElement tag;
    QDomElement subtag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    /* Function tag */
    root = doc->createElement(KXMLQLCFunction);
    wksp_root->appendChild(root);

    /* Common attributes */
    saveXMLCommon(&root);

    /* Fixtures */
    QListIterator <EFXFixture*> it(m_fixtures);
    while (it.hasNext() == true)
        it.next()->saveXML(doc, &root);

    /* Propagation mode */
    tag = doc->createElement(KXMLQLCEFXPropagationMode);
    root.appendChild(tag);
    text = doc->createTextNode(propagationModeToString(m_propagationMode));
    tag.appendChild(text);

    /* Speeds */
    saveXMLSpeed(doc, &root);

    /* Direction */
    saveXMLDirection(doc, &root);

    /* Run order */
    saveXMLRunOrder(doc, &root);

    /* Algorithm */
    tag = doc->createElement(KXMLQLCEFXAlgorithm);
    root.appendChild(tag);
    text = doc->createTextNode(algorithmToString(algorithm()));
    tag.appendChild(text);

    /* Width */
    tag = doc->createElement(KXMLQLCEFXWidth);
    root.appendChild(tag);
    str.setNum(width());
    text = doc->createTextNode(str);
    tag.appendChild(text);

    /* Height */
    tag = doc->createElement(KXMLQLCEFXHeight);
    root.appendChild(tag);
    str.setNum(height());
    text = doc->createTextNode(str);
    tag.appendChild(text);

    /* Rotation */
    tag = doc->createElement(KXMLQLCEFXRotation);
    root.appendChild(tag);
    str.setNum(rotation());
    text = doc->createTextNode(str);
    tag.appendChild(text);

    /* StartOffset */
    tag = doc->createElement(KXMLQLCEFXStartOffset);
    root.appendChild(tag);
    str.setNum(startOffset());
    text = doc->createTextNode(str);
    tag.appendChild(text);

    /* IsRelative */
    tag = doc->createElement(KXMLQLCEFXIsRelative);
    root.appendChild(tag);
    str.setNum(isRelative() ? 1 : 0);
    text = doc->createTextNode(str);
    tag.appendChild(text);

    /********************************************
     * X-Axis
     ********************************************/
    tag = doc->createElement(KXMLQLCEFXAxis);
    root.appendChild(tag);
    tag.setAttribute(KXMLQLCFunctionName, KXMLQLCEFXX);

    /* Offset */
    subtag = doc->createElement(KXMLQLCEFXOffset);
    tag.appendChild(subtag);
    str.setNum(xOffset());
    text = doc->createTextNode(str);
    subtag.appendChild(text);

    /* Frequency */
    subtag = doc->createElement(KXMLQLCEFXFrequency);
    tag.appendChild(subtag);
    str.setNum(xFrequency());
    text = doc->createTextNode(str);
    subtag.appendChild(text);

    /* Phase */
    subtag = doc->createElement(KXMLQLCEFXPhase);
    tag.appendChild(subtag);
    str.setNum(xPhase());
    text = doc->createTextNode(str);
    subtag.appendChild(text);

    /********************************************
     * Y-Axis
     ********************************************/
    tag = doc->createElement(KXMLQLCEFXAxis);
    root.appendChild(tag);
    tag.setAttribute(KXMLQLCFunctionName, KXMLQLCEFXY);

    /* Offset */
    subtag = doc->createElement(KXMLQLCEFXOffset);
    tag.appendChild(subtag);
    str.setNum(yOffset());
    text = doc->createTextNode(str);
    subtag.appendChild(text);

    /* Frequency */
    subtag = doc->createElement(KXMLQLCEFXFrequency);
    tag.appendChild(subtag);
    str.setNum(yFrequency());
    text = doc->createTextNode(str);
    subtag.appendChild(text);

    /* Phase */
    subtag = doc->createElement(KXMLQLCEFXPhase);
    tag.appendChild(subtag);
    str.setNum(yPhase());
    text = doc->createTextNode(str);
    subtag.appendChild(text);

    return true;
}

bool EFX::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCFunction)
    {
        qWarning() << "Function node not found!";
        return false;
    }

    if (root.attribute(KXMLQLCFunctionType) != typeToString(Function::EFX))
    {
        qWarning("Function is not an EFX!");
        return false;
    }

    /* Load EFX contents */
    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();

        if (tag.tagName() == KXMLQLCBus)
        {
            /* Bus */
            QString str = tag.attribute(KXMLQLCBusRole);
            if (str == KXMLQLCBusFade)
                m_legacyFadeBus = tag.text().toUInt();
            else if (str == KXMLQLCBusHold)
                m_legacyHoldBus = tag.text().toUInt();
        }
        else if (tag.tagName() == KXMLQLCFunctionSpeed)
        {
            loadXMLSpeed(tag);
        }
        else if (tag.tagName() == KXMLQLCEFXFixture)
        {
            EFXFixture* ef = new EFXFixture(this);
            ef->loadXML(tag);
            if (ef->head().isValid())
            {
                if (addFixture(ef) == false)
                    delete ef;
            }
        }
        else if (tag.tagName() == KXMLQLCEFXPropagationMode)
        {
            /* Propagation mode */
            setPropagationMode(stringToPropagationMode(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCEFXAlgorithm)
        {
            /* Algorithm */
            setAlgorithm(stringToAlgorithm(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCFunctionDirection)
        {
            loadXMLDirection(tag);
        }
        else if (tag.tagName() == KXMLQLCFunctionRunOrder)
        {
            loadXMLRunOrder(tag);
        }
        else if (tag.tagName() == KXMLQLCEFXWidth)
        {
            /* Width */
            setWidth(tag.text().toInt());
        }
        else if (tag.tagName() == KXMLQLCEFXHeight)
        {
            /* Height */
            setHeight(tag.text().toInt());
        }
        else if (tag.tagName() == KXMLQLCEFXRotation)
        {
            /* Rotation */
            setRotation(tag.text().toInt());
        }
        else if (tag.tagName() == KXMLQLCEFXStartOffset)
        {
            /* StartOffset */
            setStartOffset(tag.text().toInt());
        }
        else if (tag.tagName() == KXMLQLCEFXIsRelative)
        {
            /* IsRelative */
            setIsRelative(tag.text().toInt() != 0);
        }
        else if (tag.tagName() == KXMLQLCEFXAxis)
        {
            /* Axes */
            loadXMLAxis(tag);
        }
        else
        {
            qWarning() << "Unknown EFX tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool EFX::loadXMLAxis(const QDomElement& root)
{
    int frequency = 0;
    int offset = 0;
    int phase = 0;
    QString axis;

    if (root.tagName() != KXMLQLCEFXAxis)
    {
        qWarning() << "EFX axis node not found!";
        return false;
    }

    /* Get the axis name */
    axis = root.attribute(KXMLQLCFunctionName);

    /* Load axis contents */
    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCEFXOffset)
            offset = tag.text().toInt();
        else if (tag.tagName() == KXMLQLCEFXFrequency)
            frequency = tag.text().toInt();
        else if (tag.tagName() == KXMLQLCEFXPhase)
            phase = tag.text().toInt();
        else
            qWarning() << "Unknown EFX axis tag:" << tag.tagName();
        node = node.nextSibling();
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

void EFX::preRun(MasterTimer* timer)
{
    int serialNumber = 0;

    QListIterator <EFXFixture*> it(m_fixtures);
    while (it.hasNext() == true)
    {
        EFXFixture* ef = it.next();
        Q_ASSERT(ef != NULL);
        ef->setSerialNumber(serialNumber++);
    }

    Q_ASSERT(m_fader == NULL);
    m_fader = new GenericFader(doc());

    Function::preRun(timer);
}

void EFX::write(MasterTimer* timer, UniverseArray* universes)
{
    int ready = 0;

    Q_UNUSED(timer);

    QListIterator <EFXFixture*> it(m_fixtures);
    while (it.hasNext() == true)
    {
        EFXFixture* ef = it.next();
        if (ef->isReady() == false)
            ef->nextStep(timer, universes);
        else
            ready++;
    }

    incrementElapsed();

    /* Check for stop condition */
    if (ready == m_fixtures.count())
        stop();
    m_fader->write(universes);
}

void EFX::postRun(MasterTimer* timer, UniverseArray* universes)
{
    /* Reset all fixtures */
    QListIterator <EFXFixture*> it(m_fixtures);
    while (it.hasNext() == true)
    {
        EFXFixture* ef(it.next());

        /* Run the EFX's stop scene for Loop & PingPong modes */
        if (runOrder() != SingleShot)
            ef->stop(timer, universes);
        ef->reset();
    }

    Q_ASSERT(m_fader != NULL);
    m_fader->removeAll();
    delete m_fader;
    m_fader = NULL;

    Function::postRun(timer, universes);
}

/*****************************************************************************
 * Intensity
 *****************************************************************************/

void EFX::adjustAttribute(qreal fraction, int attributeIndex)
{
    switch (attributeIndex)
    {
        case Intensity:
        {
            if (m_fader != NULL)
                m_fader->adjustIntensity(fraction);

            QListIterator <EFXFixture*> it(m_fixtures);
            while (it.hasNext() == true)
            {
                EFXFixture* ef = it.next();
                ef->adjustIntensity(fraction);
            }
        }
        break;

        case Height:
        case Width:
        case XOffset:
        case YOffset:
        case Rotation:
        break;
    }

    Function::adjustAttribute(fraction, attributeIndex);

    if (attributeIndex == Rotation)
        updateRotationCache();
}
