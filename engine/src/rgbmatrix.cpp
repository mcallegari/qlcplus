/*
  Q Light Controller
  rgbmatrix.cpp

  Copyright (c) Heikki Junnila

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

#include <QCoreApplication>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDomText>
#include <QDebug>
#include <QTime>
#include <cmath>
#include <QDir>

#include "qlcfixturehead.h"
#include "fixturegroup.h"
#include "genericfader.h"
#include "fadechannel.h"
#include "rgbmatrix.h"
#include "qlcmacros.h"
#include "rgbtext.h"
#include "doc.h"

#define KXMLQLCRGBMatrixStartColor "MonoColor"
#define KXMLQLCRGBMatrixEndColor "EndColor"
#define KXMLQLCRGBMatrixFixtureGroup "FixtureGroup"

/****************************************************************************
 * Initialization
 ****************************************************************************/

RGBMatrix::RGBMatrix(Doc* doc)
    : Function(doc, Function::RGBMatrix)
    , m_fixtureGroup(FixtureGroup::invalidId())
    , m_algorithm(NULL)
    , m_startColor(Qt::red)
    , m_endColor(QColor())
    , m_fader(NULL)
    , m_step(0)
    , m_roundTime(new QTime)
    , m_stepColor(QColor())
    , m_crDelta(0)
    , m_cgDelta(0)
    , m_cbDelta(0)
{
    setName(tr("New RGB Matrix"));
    setDuration(500);

    RGBScript scr = RGBScript::script(doc, "Full Columns");
    setAlgorithm(scr.clone());
}

RGBMatrix::~RGBMatrix()
{
    setAlgorithm(NULL);
    delete m_roundTime;
    m_roundTime = NULL;
}

/****************************************************************************
 * Copying
 ****************************************************************************/

Function* RGBMatrix::createCopy(Doc* doc, bool addToDoc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new RGBMatrix(doc);
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

bool RGBMatrix::copyFrom(const Function* function)
{
    const RGBMatrix* mtx = qobject_cast<const RGBMatrix*> (function);
    if (mtx == NULL)
        return false;

    setFixtureGroup(mtx->fixtureGroup());
    if (mtx->algorithm() != NULL)
        setAlgorithm(mtx->algorithm()->clone());
    else
        setAlgorithm(NULL);
    setStartColor(mtx->startColor());
    setEndColor(mtx->endColor());

    return Function::copyFrom(function);
}

/****************************************************************************
 * Fixtures
 ****************************************************************************/

void RGBMatrix::setFixtureGroup(quint32 id)
{
    m_fixtureGroup = id;
}

quint32 RGBMatrix::fixtureGroup() const
{
    return m_fixtureGroup;
}

/****************************************************************************
 * Algorithm
 ****************************************************************************/

void RGBMatrix::setAlgorithm(RGBAlgorithm* algo)
{
    if (m_algorithm != NULL)
        delete m_algorithm;
    m_algorithm = algo;
}

RGBAlgorithm* RGBMatrix::algorithm() const
{
    return m_algorithm;
}

QList <RGBMap> RGBMatrix::previewMaps()
{
    QList <RGBMap> steps;

    if (m_algorithm == NULL)
        return steps;

    FixtureGroup* grp = doc()->fixtureGroup(fixtureGroup());
    if (grp != NULL)
    {
        int stepCount = m_algorithm->rgbMapStepCount(grp->size());
        for (int i = 0; i < stepCount; i++)
            steps << m_algorithm->rgbMap(grp->size(), m_stepColor.rgb(), i);
    }

    return steps;
}

/****************************************************************************
 * Colour
 ****************************************************************************/

void RGBMatrix::setStartColor(const QColor& c)
{
    m_startColor = c;
}

QColor RGBMatrix::startColor() const
{
    return m_startColor;
}

void RGBMatrix::setEndColor(const QColor &c)
{
    m_endColor = c;
}

QColor RGBMatrix::endColor() const
{
    return m_endColor;
}

void RGBMatrix::calculateColorDelta()
{
    m_crDelta = 0;
    m_cgDelta = 0;
    m_cbDelta = 0;

    if (m_endColor.isValid())
    {
        if (doc() == NULL)
            return;

        FixtureGroup* grp = doc()->fixtureGroup(fixtureGroup());
        if (grp != NULL && m_algorithm != NULL)
        {
            if (m_algorithm->rgbMapStepCount(grp->size()) > 1)
            {
                m_crDelta = (m_endColor.red() - m_startColor.red()) / (m_algorithm->rgbMapStepCount(grp->size()) - 1);
                m_cgDelta = (m_endColor.green() - m_startColor.green()) / (m_algorithm->rgbMapStepCount(grp->size()) - 1);
                m_cbDelta = (m_endColor.blue() - m_startColor.blue()) / (m_algorithm->rgbMapStepCount(grp->size()) - 1);
            }
        }
    }
}

void RGBMatrix::setStepColor(QColor color)
{
    m_stepColor = color;
}

QColor RGBMatrix::stepColor()
{
    return m_stepColor;
}

void RGBMatrix::updateStepColor(Function::Direction direction)
{
    if (direction == Forward)
    {
        m_stepColor = QColor(m_stepColor.red() + m_crDelta,
                             m_stepColor.green() + m_cgDelta,
                             m_stepColor.blue() + m_cbDelta);
    }
    else
        m_stepColor = QColor(m_stepColor.red() - m_crDelta,
                             m_stepColor.green() - m_cgDelta,
                             m_stepColor.blue() - m_cbDelta);
}


/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool RGBMatrix::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root.attribute(KXMLQLCFunctionType) != typeToString(Function::RGBMatrix))
    {
        qWarning() << Q_FUNC_INFO << "Function is not an RGB matrix";
        return false;
    }

    /* Load matrix contents */
    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();

        if (tag.tagName() == KXMLQLCFunctionSpeed)
        {
            loadXMLSpeed(tag);
        }
        else if (tag.tagName() == KXMLQLCRGBAlgorithm)
        {
            setAlgorithm(RGBAlgorithm::loader(doc(), tag));
        }
        else if (tag.tagName() == KXMLQLCRGBMatrixFixtureGroup)
        {
            setFixtureGroup(tag.text().toUInt());
        }
        else if (tag.tagName() == KXMLQLCFunctionDirection)
        {
            loadXMLDirection(tag);
        }
        else if (tag.tagName() == KXMLQLCFunctionRunOrder)
        {
            loadXMLRunOrder(tag);
        }
        else if (tag.tagName() == KXMLQLCRGBMatrixStartColor)
        {
            setStartColor(QColor::fromRgb(QRgb(tag.text().toUInt())));
        }
        else if (tag.tagName() == KXMLQLCRGBMatrixEndColor)
        {
            setEndColor(QColor::fromRgb(QRgb(tag.text().toUInt())));
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown RGB matrix tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool RGBMatrix::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement root;
    QDomElement tag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    /* Function tag */
    root = doc->createElement(KXMLQLCFunction);
    wksp_root->appendChild(root);

    /* Common attributes */
    saveXMLCommon(&root);

    /* Speeds */
    saveXMLSpeed(doc, &root);

    /* Direction */
    saveXMLDirection(doc, &root);

    /* Run order */
    saveXMLRunOrder(doc, &root);

    /* Algorithm */
    if (m_algorithm != NULL)
        m_algorithm->saveXML(doc, &root);

    /* Start Color */
    tag = doc->createElement(KXMLQLCRGBMatrixStartColor);
    root.appendChild(tag);
    text = doc->createTextNode(QString::number(startColor().rgb()));
    tag.appendChild(text);

    /* End Color */
    if (endColor().isValid())
    {
        tag = doc->createElement(KXMLQLCRGBMatrixEndColor);
        root.appendChild(tag);
        text = doc->createTextNode(QString::number(endColor().rgb()));
        tag.appendChild(text);
    }

    /* Fixture Group */
    tag = doc->createElement(KXMLQLCRGBMatrixFixtureGroup);
    root.appendChild(tag);
    text = doc->createTextNode(QString::number(fixtureGroup()));
    tag.appendChild(text);

    return true;
}

/****************************************************************************
 * Running
 ****************************************************************************/

void RGBMatrix::tap()
{
    if (stopped() == false)
    {
        FixtureGroup* grp = doc()->fixtureGroup(fixtureGroup());
        // Filter out taps that are too close to each other
        if (grp != NULL && uint(m_roundTime->elapsed()) >= (duration() / 4))
            roundCheck(grp->size());
    }
}

void RGBMatrix::preRun(MasterTimer* timer)
{
    Q_UNUSED(timer);

    FixtureGroup* grp = doc()->fixtureGroup(fixtureGroup());
    if (grp != NULL && m_algorithm != NULL)
    {
        m_direction = direction();

        Q_ASSERT(m_fader == NULL);
        m_fader = new GenericFader(doc());

        if (m_direction == Forward)
        {
            m_step = 0;
            m_stepColor = m_startColor.rgb();
        }
        else
        {
            m_step = m_algorithm->rgbMapStepCount(grp->size());
            m_stepColor = m_endColor.rgb();
        }

        calculateColorDelta();
    }

    m_roundTime->start();

    Function::preRun(timer);
}

void RGBMatrix::write(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(timer);
    Q_UNUSED(universes);

    FixtureGroup* grp = doc()->fixtureGroup(fixtureGroup());
    if (grp == NULL)
    {
        // No fixture group to control
        stop();
        return;
    }

    // No time to do anything.
    if (duration() == 0)
        return;

    // Invalid/nonexistent script
    if (m_algorithm == NULL || m_algorithm->apiVersion() == 0)
        return;

    // Get new map every time when elapsed is reset to zero
    if (elapsed() == 0)
    {
        qDebug() << "RGBMatrix stepColor:" << QString::number(m_stepColor.rgb(), 16);
        RGBMap map = m_algorithm->rgbMap(grp->size(), m_stepColor.rgb(), m_step);
        updateMapChannels(map, grp);
    }

    // Run the generic fader that takes care of fading in/out individual channels
    m_fader->write(universes);

    // Increment elapsed time
    incrementElapsed();

    // Check if we need to change direction, stop completely or go to next step
    if (elapsed() >= duration())
        roundCheck(grp->size());
}

void RGBMatrix::postRun(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(timer);
    Q_UNUSED(universes);

    if (m_fader != NULL)
        delete m_fader; // Might be NULL if there's no fixture group
    m_fader = NULL;

    Function::postRun(timer, universes);
}

void RGBMatrix::roundCheck(const QSize& size)
{
    if (m_algorithm == NULL)
        return;

    if (runOrder() == PingPong)
    {
        if (m_direction == Forward && (m_step + 1) == m_algorithm->rgbMapStepCount(size))
        {
            m_direction = Backward;
            m_step = m_algorithm->rgbMapStepCount(size) - 2;
            if (m_endColor.isValid())
                m_stepColor = m_endColor;

            updateStepColor(m_direction);
        }
        else if (m_direction == Backward && (m_step - 1) < 0)
        {
            m_direction = Forward;
            m_step = 1;
            m_stepColor = m_startColor;
            updateStepColor(m_direction);
        }
        else
        {
            if (m_direction == Forward)
                m_step++;
            else
                m_step--;
            updateStepColor(m_direction);
        }
    }
    else if (runOrder() == SingleShot)
    {
        if (m_direction == Forward)
        {
            if (m_step >= m_algorithm->rgbMapStepCount(size) - 1)
                stop();
            else
                m_step++;
            updateStepColor(m_direction);
        }
    }
    else
    {
        if (m_direction == Forward)
        {
            if (m_step >= m_algorithm->rgbMapStepCount(size) - 1)
            {
                m_step = 0;
                m_stepColor = m_startColor;
            }
            else
            {
                m_step++;
                updateStepColor(m_direction);
            }
        }
        else
        {
            if (m_step <= 0)
            {
                m_step = m_algorithm->rgbMapStepCount(size) - 1;
                if (m_endColor.isValid())
                    m_stepColor = m_endColor;
            }
            else
            {
                m_step--;
                updateStepColor(m_direction);
            }
        }
    }

    m_roundTime->restart();
    resetElapsed();
}

void RGBMatrix::updateMapChannels(const RGBMap& map, const FixtureGroup* grp)
{
    quint32 mdAssigned = QLCChannel::invalid();
    quint32 mdFxi = Fixture::invalidId();

    // Create/modify fade channels for ALL pixels in the color map.
    for (int y = 0; y < map.size(); y++)
    {
        for (int x = 0; x < map[y].size(); x++)
        {
            QLCPoint pt(x, y);
            GroupHead grpHead(grp->head(pt));
            Fixture* fxi = doc()->fixture(grpHead.fxi);
            if (fxi == NULL)
                continue;

            if (grpHead.fxi != mdFxi)
            {
                mdAssigned = QLCChannel::invalid();
                mdFxi = grpHead.fxi;
            }

            QLCFixtureHead head = fxi->head(grpHead.head);

            QList <quint32> rgb = head.rgbChannels();
            QList <quint32> cmy = head.cmyChannels();
            if (rgb.size() == 3)
            {
                // RGB color mixing
                FadeChannel fc;
                fc.setFixture(grpHead.fxi);

                fc.setChannel(rgb.takeFirst());
                fc.setTarget(qRed(map[y][x]));
                insertStartValues(fc);
                m_fader->add(fc);

                fc.setChannel(rgb.takeFirst());
                fc.setTarget(qGreen(map[y][x]));
                insertStartValues(fc);
                m_fader->add(fc);

                fc.setChannel(rgb.takeFirst());
                fc.setTarget(qBlue(map[y][x]));
                insertStartValues(fc);
                m_fader->add(fc);
            }
            else if (cmy.size() == 3)
            {
                // CMY color mixing
                QColor col(map[y][x]);

                FadeChannel fc;
                fc.setFixture(grpHead.fxi);

                fc.setChannel(cmy.takeFirst());
                fc.setTarget(col.cyan());
                insertStartValues(fc);
                m_fader->add(fc);

                fc.setChannel(cmy.takeFirst());
                fc.setTarget(col.magenta());
                insertStartValues(fc);
                m_fader->add(fc);

                fc.setChannel(cmy.takeFirst());
                fc.setTarget(col.yellow());
                insertStartValues(fc);
                m_fader->add(fc);
            }

            if (head.masterIntensityChannel() != QLCChannel::invalid())
            {
                qDebug() << "RGBMatrix: found dimmer at" << head.masterIntensityChannel();
                // Simple intensity (dimmer) channel
                QColor col(map[y][x]);
                FadeChannel fc;
                fc.setFixture(grpHead.fxi);
                fc.setChannel(head.masterIntensityChannel());
                if (col.value() == 0 && mdAssigned != head.masterIntensityChannel())
                    fc.setTarget(0);
                else
                {
                    fc.setTarget(255);
                    if (mdAssigned == QLCChannel::invalid())
                        mdAssigned = head.masterIntensityChannel();
                }
                insertStartValues(fc);
                m_fader->add(fc);
            }
        }
    }
}

void RGBMatrix::insertStartValues(FadeChannel& fc) const
{
    Q_ASSERT(m_fader != NULL);

    // To create a nice and smooth fade, get the starting value from
    // m_fader's existing FadeChannel (if any). Otherwise just assume
    // we're starting from zero.
    if (m_fader->channels().contains(fc) == true)
    {
        FadeChannel old = m_fader->channels()[fc];
        fc.setCurrent(old.current());
        fc.setStart(old.current());
    }
    else
    {
        fc.setCurrent(0);
        fc.setStart(0);
    }

    // The channel is not ready yet
    fc.setReady(false);

    // Fade in speed is used for all non-zero targets
    if (fc.target() == 0)
        fc.setFadeTime(fadeOutSpeed());
    else
        fc.setFadeTime(fadeInSpeed());
}

/*********************************************************************
 * Attributes
 *********************************************************************/

void RGBMatrix::adjustAttribute(qreal fraction, int attributeIndex)
{
    if (m_fader != NULL && attributeIndex == Function::Intensity)
        m_fader->adjustIntensity(fraction);
    Function::adjustAttribute(fraction, attributeIndex);
}
