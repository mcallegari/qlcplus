/*
  Q Light Controller Plus
  vcspeeddial.cpp

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
#include <QQmlEngine>

#include "doc.h"
#include "qlcmacros.h"
#include "vcspeeddial.h"

#define INPUT_DIAL_ID       0
#define INPUT_TAP_ID        1
#define INPUT_MULT_ID       2
#define INPUT_DIV_ID        3
#define INPUT_RESET_ID      4
#define INPUT_APPLY_ID      5
#define INPUT_1_16X_ID      6
#define INPUT_1_8X_ID       7
#define INPUT_1_4X_ID       8
#define INPUT_1_2X_ID       9
#define INPUT_2X_ID         10
#define INPUT_4X_ID         11
#define INPUT_8X_ID         12
#define INPUT_16X_ID        13

VCSpeedDial::VCSpeedDial(Doc *doc, QObject *parent)
    : VCWidget(doc, parent)
    , m_visibilityMask(Tap | Beats | Multipliers)
    , m_timeMinimumValue(0)
    , m_timeMaximumValue(1000 * 10)
    , m_currentTime(0)
    , m_resetOnDialChange(false)
    , m_currentFactor(One)
{
    setType(VCWidget::SpeedWidget);

    cacheMultipliers();

    registerExternalControl(INPUT_DIAL_ID, tr("Time wheel"), false);
    registerExternalControl(INPUT_TAP_ID, tr("Tap Button"), true);
    registerExternalControl(INPUT_MULT_ID, tr("Multiply Button"), true);
    registerExternalControl(INPUT_DIV_ID, tr("Divide Button"), true);
    registerExternalControl(INPUT_RESET_ID, tr("Reset Button"), true);
    registerExternalControl(INPUT_APPLY_ID, tr("Apply Button"), true);
    registerExternalControl(INPUT_1_16X_ID, tr("1/16x Speed Button"), true);
    registerExternalControl(INPUT_1_8X_ID, tr("1/8x Speed Button"), true);
    registerExternalControl(INPUT_1_4X_ID, tr("1/4x Speed Button"), true);
    registerExternalControl(INPUT_1_2X_ID, tr("1/2x Speed Button"), true);
    registerExternalControl(INPUT_2X_ID, tr("2x Speed Button"), true);
    registerExternalControl(INPUT_4X_ID, tr("4x Speed Button"), true);
    registerExternalControl(INPUT_8X_ID, tr("8x Speed Button"), true);
    registerExternalControl(INPUT_16X_ID, tr("16x Speed Button"), true);
}

VCSpeedDial::~VCSpeedDial()
{
    if (m_item)
        delete m_item;
}

QString VCSpeedDial::defaultCaption()
{
    return tr("Speed %1").arg(id() + 1);
}

void VCSpeedDial::setupLookAndFeel(qreal pixelDensity, int page)
{
    setPage(page);
    QFont wFont = font();
    wFont.setBold(true);
    wFont.setPointSize(pixelDensity * 5.0);
    setFont(wFont);
}

void VCSpeedDial::render(QQuickView *view, QQuickItem *parent)
{
    if (view == nullptr || parent == nullptr)
        return;

    QQmlComponent *component = new QQmlComponent(view->engine(), QUrl("qrc:/VCSpeedDialItem.qml"));

    if (component->isError())
    {
        qDebug() << component->errors();
        return;
    }

    m_item = qobject_cast<QQuickItem*>(component->create());

    m_item->setParentItem(parent);
    m_item->setProperty("speedObj", QVariant::fromValue(this));
}

QString VCSpeedDial::propertiesResource() const
{
    return QString("qrc:/VCSpeedDialProperties.qml");
}

VCWidget *VCSpeedDial::createCopy(VCWidget *parent)
{
    Q_ASSERT(parent != nullptr);

    VCSpeedDial *speedDial = new VCSpeedDial(m_doc, parent);
    if (speedDial->copyFrom(this) == false)
    {
        delete speedDial;
        speedDial = nullptr;
    }

    return speedDial;
}

bool VCSpeedDial::copyFrom(const VCWidget *widget)
{
    const VCSpeedDial *speedDial = qobject_cast<const VCSpeedDial*> (widget);
    if (speedDial == nullptr)
        return false;

    /* Copy and set properties */
    setVisibilityMask(speedDial->visibilityMask());
    setCurrentTime(speedDial->currentTime());
    setTimeMinimumValue(speedDial->timeMinimumValue());
    setTimeMaximumValue(speedDial->timeMaximumValue());

    setFunctions(speedDial->functions());

    /* Common stuff */
    return VCWidget::copyFrom(widget);
}

void VCSpeedDial::cacheMultipliers()
{
    m_multiplierCache << 0; // None
    m_multiplierCache << 0; // Zero
    m_multiplierCache << 1000 / 16;
    m_multiplierCache << 1000 / 8;
    m_multiplierCache << 1000 / 4;
    m_multiplierCache << 1000 / 2;
    m_multiplierCache << 1000;
    m_multiplierCache << 1000 * 2;
    m_multiplierCache << 1000 * 4;
    m_multiplierCache << 1000 * 8;
    m_multiplierCache << 1000 * 16;
}

/*********************************************************************
 * UI elements visibility
 *********************************************************************/

quint32 VCSpeedDial::visibilityMask() const
{
    return m_visibilityMask;
}

void VCSpeedDial::setVisibilityMask(quint32 mask)
{
    if (mask == m_visibilityMask)
        return;

    m_visibilityMask = mask;
    emit visibilityMaskChanged();
}

/*********************************************************************
 * Dial absolute time
 *********************************************************************/

uint VCSpeedDial::timeMinimumValue() const
{
    return m_timeMinimumValue;
}

void VCSpeedDial::setTimeMinimumValue(uint newTimeMinimumValue)
{
    if (m_timeMinimumValue == newTimeMinimumValue)
        return;

    m_timeMinimumValue = newTimeMinimumValue;
    emit timeMinimumValueChanged();
}

uint VCSpeedDial::timeMaximumValue() const
{
    return m_timeMaximumValue;
}

void VCSpeedDial::setTimeMaximumValue(uint newTimeMaximumValue)
{
    if (m_timeMaximumValue == newTimeMaximumValue)
        return;

    m_timeMaximumValue = newTimeMaximumValue;
    emit timeMaximumValueChanged();
}

uint VCSpeedDial::currentTime() const
{
    return m_currentTime;
}

void VCSpeedDial::setCurrentTime(uint newCurrentTime)
{
    if (m_currentTime == newCurrentTime)
        return;

    m_currentTime = newCurrentTime;

    if (m_currentTime != 0)
        applyFunctionsTime();

    emit currentTimeChanged();
}

bool VCSpeedDial::resetOnDialChange() const
{
    return m_resetOnDialChange;
}

void VCSpeedDial::setResetOnDialChange(bool newResetOnDialChange)
{
    if (m_resetOnDialChange == newResetOnDialChange)
        return;

    m_resetOnDialChange = newResetOnDialChange;
    emit resetOnDialChangeChanged();
}

/*********************************************************************
 * Speed factor
 *********************************************************************/

VCSpeedDial::SpeedMultiplier VCSpeedDial::currentFactor()
{
    return m_currentFactor;
}

void VCSpeedDial::setCurrentFactor(SpeedMultiplier factor)
{
    if (factor == m_currentFactor)
        return;

    m_currentFactor = factor;

    applyFunctionsTime();

    emit currentFactorChanged();
}

void VCSpeedDial::increaseSpeedFactor()
{
    if (m_currentFactor < Sixteen)
    {
        int factor = m_currentFactor;
        factor++;
        setCurrentFactor(SpeedMultiplier(factor));
    }
}

void VCSpeedDial::decreaseSpeedFactor()
{
    if (m_currentFactor > OneSixteenth)
    {
        int factor = m_currentFactor;
        factor--;
        setCurrentFactor(SpeedMultiplier(factor));
    }
}

/*********************************************************************
 * Functions
 *********************************************************************/

QMap<quint32, VCSpeedDial::VCSpeedDialFunction> VCSpeedDial::functions() const
{
    return m_functions;
}

void VCSpeedDial::setFunctions(const QMap<quint32, VCSpeedDialFunction> &functions)
{
    m_functions = functions;
}

void VCSpeedDial::addFunction(VCSpeedDialFunction function)
{
    m_functions[function.m_fId] = function;

    emit functionsListChanged();
}

void VCSpeedDial::addFunction(quint32 functionID)
{
    if (m_functions.contains(functionID))
        return;

    VCSpeedDialFunction func;
    func.m_fId = functionID;
    func.m_fadeInFactor = None;
    func.m_fadeOutFactor = None;
    func.m_durationFactor = One;

    m_functions[functionID] = func;

    emit functionsListChanged();
}

void VCSpeedDial::removeFunction(quint32 functionID)
{
    m_functions.remove(functionID);

    emit functionsListChanged();
}

QVariant VCSpeedDial::functionsList()
{
    QVariantList fList;

    for (VCSpeedDialFunction &func : m_functions)
    {
        QVariantMap fMap;
        fMap.insert("funcID", func.m_fId);
        fMap.insert("fadeIn", func.m_fadeInFactor);
        fMap.insert("fadeOut", func.m_fadeOutFactor);
        fMap.insert("duration", func.m_durationFactor);
        fList.append(fMap);
    }

    return QVariant::fromValue(fList);
}

void VCSpeedDial::setFunctionSpeed(quint32 fid, int speedType, SpeedMultiplier amount)
{
    VCSpeedDialFunction func = m_functions[fid];

    switch (speedType)
    {
        case Function::FadeIn: func.m_fadeInFactor = amount; break;
        case Function::FadeOut: func.m_fadeOutFactor = amount; break;
        case Function::Duration: func.m_durationFactor = amount; break;
    }

    m_functions[fid] = func;
}

void VCSpeedDial::applyFunctionsTime()
{
    float factoredTime = m_currentTime * (m_multiplierCache[m_currentFactor] / 1000.0);

    for (const VCSpeedDialFunction &func : m_functions)
    {
        Function *function = m_doc->function(func.m_fId);
        if (function != NULL)
        {
            if (func.m_fadeInFactor != VCSpeedDial::None)
                function->setFadeInSpeed(factoredTime * (m_multiplierCache[func.m_fadeInFactor] / 1000.0));

            if (func.m_fadeOutFactor != VCSpeedDial::None)
                function->setFadeOutSpeed(factoredTime * (m_multiplierCache[func.m_fadeOutFactor] / 1000.0));

            if (func.m_durationFactor != VCSpeedDial::None)
                function->setDuration(factoredTime * (m_multiplierCache[func.m_durationFactor] / 1000.0));
        }
    }
}

/*********************************************************************
 * External input
 *********************************************************************/

void VCSpeedDial::updateFeedback()
{
    // TODO: DIAL_ID, TAP_ID(?)
    sendFeedback(m_currentFactor == OneSixteenth ? UCHAR_MAX : 0, INPUT_1_16X_ID, VCWidget::ExactValue);
    sendFeedback(m_currentFactor == OneEighth ? UCHAR_MAX : 0, INPUT_1_8X_ID, VCWidget::ExactValue);
    sendFeedback(m_currentFactor == OneFourth ? UCHAR_MAX : 0, INPUT_1_4X_ID, VCWidget::ExactValue);
    sendFeedback(m_currentFactor == Half ? UCHAR_MAX : 0, INPUT_1_2X_ID, VCWidget::ExactValue);
    sendFeedback(m_currentFactor == Two ? UCHAR_MAX : 0, INPUT_2X_ID, VCWidget::ExactValue);
    sendFeedback(m_currentFactor == Four ? UCHAR_MAX : 0, INPUT_4X_ID, VCWidget::ExactValue);
    sendFeedback(m_currentFactor == Eight ? UCHAR_MAX : 0, INPUT_8X_ID, VCWidget::ExactValue);
    sendFeedback(m_currentFactor == Sixteen ? UCHAR_MAX : 0, INPUT_16X_ID, VCWidget::ExactValue);
}

void VCSpeedDial::slotInputValueChanged(quint8 id, uchar value)
{
    // filter unwanted values
    if (id != INPUT_DIAL_ID && value != UCHAR_MAX)
        return;

    switch (id)
    {
        case INPUT_DIAL_ID:
        {
            int ms = static_cast<int> (SCALE(qreal(value), qreal(0), qreal(255),
                                        qreal(timeMinimumValue()),
                                        qreal(timeMaximumValue())));
            setCurrentTime(ms);
        }
        break;
        case INPUT_TAP_ID:
            if (m_item)
                QMetaObject::invokeMethod(m_item, "tap");
        break;
        case INPUT_MULT_ID:
            increaseSpeedFactor();
        break;
        case INPUT_DIV_ID:
            decreaseSpeedFactor();
        break;
        case INPUT_RESET_ID:
            setCurrentFactor(One);
        break;
        case INPUT_APPLY_ID:
            applyFunctionsTime();
        break;
        case INPUT_1_16X_ID:
            setCurrentFactor(OneSixteenth);
        break;
        case INPUT_1_8X_ID:
            setCurrentFactor(OneEighth);
        break;
        case INPUT_1_4X_ID:
            setCurrentFactor(OneFourth);
        break;
        case INPUT_1_2X_ID:
            setCurrentFactor(Half);
        break;
        case INPUT_2X_ID:
            setCurrentFactor(Two);
        break;
        case INPUT_4X_ID:
            setCurrentFactor(Four);
        break;
        case INPUT_8X_ID:
            setCurrentFactor(Eight);
        break;
        case INPUT_16X_ID:
            setCurrentFactor(Sixteen);
        break;
    }
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool VCSpeedDial::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCSpeedDial)
    {
        qWarning() << Q_FUNC_INFO << "Speed dial node not found";
        return false;
    }

    QXmlStreamAttributes attrs = root.attributes();

    /* Widget commons */
    loadXMLCommon(root);

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCWindowState)
        {
            bool visible = false;
            int x = 0, y = 0, w = 0, h = 0;
            loadXMLWindowState(root, &x, &y, &w, &h, &visible);
            setGeometry(QRect(x, y, w, h));
        }
        else if (root.name() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(root);
        }
        else if (root.name() == KXMLQLCVCSpeedDialVisibilityMask)
        {
            setVisibilityMask(root.readElementText().toUInt());
        }
        else if (root.name() == KXMLQLCVCSpeedDialAbsoluteValue)
        {
            // Value range
            QXmlStreamAttributes vAttrs = root.attributes();
            if (vAttrs.hasAttribute(KXMLQLCVCSpeedDialAbsoluteValueMin) &&
                vAttrs.hasAttribute(KXMLQLCVCSpeedDialAbsoluteValueMax))
            {
                setTimeMinimumValue(vAttrs.value(KXMLQLCVCSpeedDialAbsoluteValueMin).toString().toUInt());
                setTimeMaximumValue(vAttrs.value(KXMLQLCVCSpeedDialAbsoluteValueMax).toString().toUInt());
            }
            loadXMLSources(root, INPUT_DIAL_ID);
        }
        else if (root.name() == KXMLQLCVCSpeedDialTime)
        {
            setCurrentTime(root.readElementText().toUInt());
        }
        else if (root.name() == KXMLQLCVCSpeedDialResetFactorOnDialChange)
        {
            setResetOnDialChange(root.readElementText() == KXMLQLCTrue);
        }
        else if (root.name() == KXMLQLCVCSpeedDialFunction)
        {
            QXmlStreamAttributes attrs = root.attributes();
            QString text = root.readElementText();
            if (text.isEmpty())
            {
                qWarning() << Q_FUNC_INFO << "Function ID not found";
                root.skipCurrentElement();
            }
            VCSpeedDialFunction func;
            func.m_fId = text.toUInt();
            func.m_fadeInFactor = None;
            func.m_fadeOutFactor = None;
            func.m_durationFactor = One;

            // check for attribute presence
            if (attrs.hasAttribute(KXMLQLCFunctionSpeedFadeIn) == true)
                func.m_fadeInFactor = SpeedMultiplier(attrs.value(KXMLQLCFunctionSpeedFadeIn).toString().toUInt());
            if (attrs.hasAttribute(KXMLQLCFunctionSpeedFadeOut) == true)
                func.m_fadeOutFactor = SpeedMultiplier(attrs.value(KXMLQLCFunctionSpeedFadeOut).toString().toUInt());
            if (attrs.hasAttribute(KXMLQLCFunctionSpeedDuration) == true)
                func.m_durationFactor = SpeedMultiplier(attrs.value(KXMLQLCFunctionSpeedDuration).toString().toUInt());

            addFunction(func);
        }
        else if (root.name() == KXMLQLCVCWidgetInput)
        {
            loadXMLInputSource(root, VCWIDGET_AUTODETECT_INPUT_ID);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown speed dial tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool VCSpeedDial::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != nullptr);

    /* VC object entry */
    doc->writeStartElement(KXMLQLCVCSpeedDial);

    saveXMLCommon(doc);

    /* Window state */
    saveXMLWindowState(doc);

    /* Appearance */
    saveXMLAppearance(doc);

    /* Visibility bitmask */
    doc->writeTextElement(KXMLQLCVCSpeedDialVisibilityMask, QString::number(m_visibilityMask));

    /* Save time */
    doc->writeTextElement(KXMLQLCVCSpeedDialTime, QString::number(currentTime()));

    /* Reset factor on dial change */
    if (resetOnDialChange())
        doc->writeTextElement(KXMLQLCVCSpeedDialResetFactorOnDialChange, KXMLQLCTrue);

    /* Absolute input */
    doc->writeStartElement(KXMLQLCVCSpeedDialAbsoluteValue);
    doc->writeAttribute(KXMLQLCVCSpeedDialAbsoluteValueMin, QString::number(timeMinimumValue()));
    doc->writeAttribute(KXMLQLCVCSpeedDialAbsoluteValueMax, QString::number(timeMaximumValue()));
    saveXMLInputControl(doc, INPUT_DIAL_ID);
    doc->writeEndElement();

    for (VCSpeedDialFunction &func : m_functions)
    {
        /* Function tag */
        doc->writeStartElement(KXMLQLCVCSpeedDialFunction);

        /* Multipliers */
        doc->writeAttribute(KXMLQLCFunctionSpeedFadeIn, QString::number(func.m_fadeInFactor));
        doc->writeAttribute(KXMLQLCFunctionSpeedFadeOut, QString::number(func.m_fadeOutFactor));
        doc->writeAttribute(KXMLQLCFunctionSpeedDuration, QString::number(func.m_durationFactor));

        /* Function ID */
        doc->writeCharacters(QString::number(func.m_fId));

        /* Close the <Function> tag */
        doc->writeEndElement();
    }

    for (quint8 iId = INPUT_TAP_ID; iId <= INPUT_16X_ID; iId++)
        saveXMLInputControl(doc, iId);

    /* Write the <end> tag */
    doc->writeEndElement();

    return true;
}
