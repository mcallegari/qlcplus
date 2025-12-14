/*
  Q Light Controller Plus
  vcbutton.cpp

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

#include "qlcmacros.h"
#include "vcbutton.h"
#include "tardis.h"
#include "doc.h"

#define INPUT_PRESSURE_ID   0

VCButton::VCButton(Doc *doc, QObject *parent)
    : VCWidget(doc, parent)
    , m_functionID(Function::invalidId())
    , m_flashOverrides(false)
    , m_flashForceLTP(false)
    , m_state(Inactive)
    , m_actionType(Toggle)
    , m_stopAllFadeOutTime(0)
    , m_startupIntensityEnabled(false)
    , m_startupIntensity(1.0)
{
    setType(VCWidget::ButtonWidget);

    registerExternalControl(INPUT_PRESSURE_ID, tr("Pressure"), true);
}

VCButton::~VCButton()
{
    if (m_item)
        delete m_item;
}

QString VCButton::defaultCaption()
{
    return tr("Button %1").arg(id() + 1);
}

void VCButton::setupLookAndFeel(qreal pixelDensity, int page)
{
    VCWidget::setupLookAndFeel(pixelDensity, page);

    setBackgroundColor(QColor("#444"));
}

void VCButton::render(QQuickView *view, QQuickItem *parent)
{
    if (view == nullptr || parent == nullptr)
        return;

    QQmlComponent *component = new QQmlComponent(view->engine(), QUrl("qrc:/VCButtonItem.qml"));

    if (component->isError())
    {
        qDebug() << component->errors();
        return;
    }

    m_item = qobject_cast<QQuickItem*>(component->create());

    m_item->setParentItem(parent);
    m_item->setProperty("buttonObj", QVariant::fromValue(this));
}

QString VCButton::propertiesResource() const
{
    return QString("qrc:/VCButtonProperties.qml");
}

VCWidget *VCButton::createCopy(VCWidget *parent)
{
    Q_ASSERT(parent != nullptr);

    VCButton *button = new VCButton(m_doc, parent);
    if (button->copyFrom(this) == false)
    {
        delete button;
        button = nullptr;
    }

    return button;
}

bool VCButton::copyFrom(const VCWidget* widget)
{
    const VCButton *button = qobject_cast <const VCButton*> (widget);
    if (button == nullptr)
        return false;

    /* Copy button-specific stuff */
    //setIconPath(button->iconPath()); // TODO ?
    setFunctionID(button->functionID());
    setStartupIntensityEnabled(button->startupIntensityEnabled());
    setStartupIntensity(button->startupIntensity());
    setStopAllFadeOutTime(button->stopAllFadeOutTime());
    setActionType(button->actionType());
    setState(button->state());

    setFlashForceLTP(button->flashForceLTP());
    setFlashOverride(button->flashOverrides());

    /* Copy common stuff */
    return VCWidget::copyFrom(widget);
}

/*********************************************************************
 * Function attachment
 *********************************************************************/

void VCButton::setFunctionID(quint32 fid)
{
    bool running = false;

    if (m_functionID == fid)
        return;

    Function *current = m_doc->function(m_functionID);
    Function *function = m_doc->function(fid);

    if (current != nullptr)
    {
        /* Get rid of old function connections */
        disconnect(current, SIGNAL(running(quint32)),
                this, SLOT(slotFunctionRunning(quint32)));
        disconnect(current, SIGNAL(stopped(quint32)),
                this, SLOT(slotFunctionStopped(quint32)));
        disconnect(current, SIGNAL(flashing(quint32,bool)),
                this, SLOT(slotFunctionFlashing(quint32,bool)));

        if (current->isRunning())
        {
            running = true;
            current->stop(functionParent());
            resetIntensityOverrideAttribute();
        }

    }

    if (function != nullptr)
    {
        /* Connect to the new function */
        connect(function, SIGNAL(running(quint32)),
                this, SLOT(slotFunctionRunning(quint32)));
        connect(function, SIGNAL(stopped(quint32)),
                this, SLOT(slotFunctionStopped(quint32)));
        connect(function, SIGNAL(flashing(quint32,bool)),
                this, SLOT(slotFunctionFlashing(quint32,bool)));

        m_functionID = fid;
        if ((isEditing() && caption().isEmpty()) || caption() == defaultCaption())
            setCaption(function->name());

        if (running)
        {
            function->start(m_doc->masterTimer(), functionParent());
            setState(Active);
        }
        emit functionIDChanged(fid);
    }
    else
    {
        /* No function attachment */
        m_functionID = Function::invalidId();
        emit functionIDChanged(-1);
    }

    Tardis::instance()->enqueueAction(Tardis::VCButtonSetFunctionID, id(),
                                      current ? current->id() : Function::invalidId(),
                                      function ? function->id() : Function::invalidId());
}

quint32 VCButton::functionID() const
{
    return m_functionID;
}

void VCButton::adjustFunctionIntensity(Function *f, qreal value)
{
    qreal finalValue = startupIntensityEnabled() ? startupIntensity() * value : value;

    VCWidget::adjustFunctionIntensity(f, finalValue);
}

void VCButton::adjustIntensity(qreal val)
{
    VCWidget::adjustIntensity(val);

    if (state() == Active)
    {
        Function *f = m_doc->function(m_functionID);
        if (f == nullptr)
            return;

        adjustFunctionIntensity(f, val);
    }
}

void VCButton::notifyFunctionStarting(VCWidget *widget, quint32 fid, qreal fIntensity)
{
    Q_UNUSED(widget)
    Q_UNUSED(fIntensity)

    qDebug() << "notifyFunctionStarting" << widget->caption() << fid << fIntensity;

    if (m_functionID == Function::invalidId() || actionType() != VCButton::Toggle)
        return;

    Function *f = m_doc->function(m_functionID);
    if (f == nullptr)
        return;

    if (m_functionID != fid)
    {
        if (f->isRunning())
        {
            f->stop(functionParent());
            resetIntensityOverrideAttribute();
        }
    }
    else
    {
        adjustFunctionIntensity(f, intensity());
        f->start(m_doc->masterTimer(), functionParent());
        setState(Active);
    }
}

void VCButton::slotFunctionRunning(quint32 fid)
{
    if (fid == m_functionID && actionType() == Toggle)
    {
        if (state() == Inactive)
            setState(Monitoring);
        //emit functionStarting(this, m_functionID);
    }
}

void VCButton::slotFunctionStopped(quint32 fid)
{
    if (fid == m_functionID && actionType() == Toggle)
    {
        resetIntensityOverrideAttribute();
        setState(Inactive);
    }
}

void VCButton::slotFunctionFlashing(quint32 fid, bool state)
{
    // Do not change the state of the button for Blackout or Stop All Functions buttons
    if (actionType() != Toggle && actionType() != Flash)
        return;

    if (fid != m_functionID)
        return;

    // if the function was flashed by another button, and the function is still running, keep the button pushed
    Function* f = m_doc->function(m_functionID);
    if (state == false && actionType() == Toggle && f != nullptr && f->isRunning())
    {
        return;
    }

    setState(state ? Active : Inactive);
}

FunctionParent VCButton::functionParent() const
{
    return FunctionParent(FunctionParent::ManualVCWidget, id());
}

/*****************************************************************************
 * Flash Properties
 *****************************************************************************/

bool VCButton::flashOverrides() const
{
    return m_flashOverrides;
}

void VCButton::setFlashOverride(bool shouldOverride)
{
    if (m_flashOverrides == shouldOverride)
        return;

    m_flashOverrides = shouldOverride;
    emit flashOverrideChanged(shouldOverride);
}

bool VCButton::flashForceLTP() const
{
    return m_flashForceLTP;
}

void VCButton::setFlashForceLTP(bool forceLTP)
{
    if (m_flashForceLTP == forceLTP)
        return;

    m_flashForceLTP = forceLTP;
    emit flashForceLTPChanged(forceLTP);
}

/*********************************************************************
 * Button state
 *********************************************************************/

VCButton::ButtonState VCButton::state() const
{
    return m_state;
}

void VCButton::setState(ButtonState state)
{
    if (state == m_state)
        return;

    m_state = state;

    emit stateChanged(m_state);

    updateFeedback();
}

void VCButton::requestStateChange(bool pressed)
{
    qDebug() << "Requested button state" << pressed;

    switch(actionType())
    {
        case Toggle:
        {
            Function *f = m_doc->function(m_functionID);
            if (f == nullptr)
                return;

            if (state() != Active && pressed == true)
            {
                if (hasSoloParent())
                    emit functionStarting(this, m_functionID);
                else
                    notifyFunctionStarting(this, m_functionID, 1.0);
            }
            else if (state() == Active && pressed == false)
            {
                if (f->isRunning())
                {
                    f->stop(functionParent());
                    resetIntensityOverrideAttribute();
                    setState(Inactive);
                }
            }
        }
        break;
        case Flash:
        {
            Function *f = m_doc->function(m_functionID);
            if (f != nullptr)
            {
                if (state() == Inactive && pressed == true)
                {
                    f->flash(m_doc->masterTimer(), flashOverrides(), flashForceLTP());
                    setState(Active);
                }
                else if (state() == Active && pressed == false)
                {
                    f->unFlash(m_doc->masterTimer());
                    setState(Inactive);
                }
            }
        }
        break;
        case Blackout:
        {
            m_doc->inputOutputMap()->toggleBlackout();
            setState(pressed ? Active : Inactive);
        }
        break;
        case StopAll:
        {
            if (stopAllFadeOutTime() == 0)
                m_doc->masterTimer()->stopAllFunctions();
            else
                m_doc->masterTimer()->fadeAndStopAll(stopAllFadeOutTime());
        }
        break;
        default:
        break;
    }

    Tardis::instance()->enqueueAction(Tardis::VCButtonSetPressed, id(), false, pressed);
}


/*********************************************************************
 * Button action
 *********************************************************************/

VCButton::ButtonAction VCButton::actionType() const
{
    return m_actionType;
}

void VCButton::setActionType(ButtonAction actionType)
{
    if (m_actionType == actionType)
        return;

    Tardis::instance()->enqueueAction(Tardis::VCButtonSetActionType, id(), m_actionType, actionType);

    m_actionType = actionType;
    emit actionTypeChanged(actionType);
}

QString VCButton::actionToString(VCButton::ButtonAction action)
{
    if (action == Flash)
        return QString(KXMLQLCVCButtonActionFlash);
    else if (action == Blackout)
        return QString(KXMLQLCVCButtonActionBlackout);
    else if (action == StopAll)
        return QString(KXMLQLCVCButtonActionStopAll);
    else
        return QString(KXMLQLCVCButtonActionToggle);
}

VCButton::ButtonAction VCButton::stringToAction(const QString& str)
{
    if (str == KXMLQLCVCButtonActionFlash)
        return Flash;
    else if (str == KXMLQLCVCButtonActionBlackout)
        return Blackout;
    else if (str == KXMLQLCVCButtonActionStopAll)
        return StopAll;
    else
        return Toggle;
}

void VCButton::setStopAllFadeOutTime(int ms)
{
    if (ms == m_stopAllFadeOutTime)
        return;

    m_stopAllFadeOutTime = ms;
    emit stopAllFadeOutTimeChanged();
}

int VCButton::stopAllFadeOutTime() const
{
    return m_stopAllFadeOutTime;
}


/*****************************************************************************
 * Function startup intensity adjustment
 *****************************************************************************/

bool VCButton::startupIntensityEnabled() const
{
    return m_startupIntensityEnabled;
}

void VCButton::setStartupIntensityEnabled(bool enable)
{
    if (enable == m_startupIntensityEnabled)
        return;

    Tardis::instance()->enqueueAction(Tardis::VCButtonEnableStartupIntensity, id(), m_startupIntensityEnabled, enable);

    m_startupIntensityEnabled = enable;
    emit startupIntensityEnabledChanged();
}

qreal VCButton::startupIntensity() const
{
    return m_startupIntensity;
}

void VCButton::setStartupIntensity(qreal fraction)
{
    if (fraction == m_startupIntensity)
        return;

    qreal newVal = CLAMP(fraction, qreal(0), qreal(1));
    Tardis::instance()->enqueueAction(Tardis::VCButtonSetStartupIntensity, id(), m_startupIntensity, newVal);

    m_startupIntensity = newVal;
    emit startupIntensityChanged();
}

/*********************************************************************
 * External input
 *********************************************************************/

void VCButton::updateFeedback()
{
    if (m_state == Inactive)
        sendFeedback(0, INPUT_PRESSURE_ID, VCWidget::LowerValue);
    else if (m_state == Monitoring)
        sendFeedback(0, INPUT_PRESSURE_ID, VCWidget::MonitorValue);
    else
        sendFeedback(UCHAR_MAX, INPUT_PRESSURE_ID, VCWidget::UpperValue);
}

void VCButton::slotInputValueChanged(quint8 id, uchar value)
{
    if (id != INPUT_PRESSURE_ID)
        return;

    if (actionType() == Flash)
    {
        if (state() == Inactive && value > 0)
            requestStateChange(true);
        else if (state() == Active && value == 0)
            requestStateChange(false);
    }
    else
    {
        if (value > 0 && state() == Inactive)
            requestStateChange(true);
        else if (value > 0 && state() == Active)
            requestStateChange(false);
    }
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool VCButton::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCButton)
    {
        qWarning() << Q_FUNC_INFO << "Button node not found";
        return false;
    }

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
        else if (root.name() == KXMLQLCVCButtonFunction)
        {
            QString str = root.attributes().value(KXMLQLCVCButtonFunctionID).toString();
            setFunctionID(str.toUInt());
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLQLCVCButtonAction)
        {
            QXmlStreamAttributes attrs = root.attributes();
            if (attrs.hasAttribute(KXMLQLCVCButtonStopAllFadeTime))
                setStopAllFadeOutTime(attrs.value(KXMLQLCVCButtonStopAllFadeTime).toInt());

            if (attrs.hasAttribute(KXMLQLCVCButtonFlashOverride))
                    setFlashOverride(attrs.value(KXMLQLCVCButtonFlashOverride).toInt());

            if (attrs.hasAttribute(KXMLQLCVCButtonFlashForceLTP))
                    setFlashForceLTP(attrs.value(KXMLQLCVCButtonFlashForceLTP).toInt());

            setActionType(stringToAction(root.readElementText()));
        }
        else if (root.name() == KXMLQLCVCButtonIntensity)
        {
            bool adjust;
            if (root.attributes().value(KXMLQLCVCButtonIntensityAdjust).toString() == KXMLQLCTrue)
                adjust = true;
            else
                adjust = false;
            setStartupIntensity(qreal(root.readElementText().toInt()) / qreal(100));
            setStartupIntensityEnabled(adjust);
        }
        else if (root.name() == KXMLQLCVCWidgetInput)
        {
            loadXMLInputSource(root, INPUT_PRESSURE_ID);
        }
        else if (root.name() == KXMLQLCVCWidgetKey)
        {
            loadXMLInputKey(root, INPUT_PRESSURE_ID);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown button tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool VCButton::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != nullptr);

    /* VC button entry */
    doc->writeStartElement(KXMLQLCVCButton);

    saveXMLCommon(doc);

    /* Window state */
    saveXMLWindowState(doc);

    /* Appearance */
    saveXMLAppearance(doc);

    /* Function */
    doc->writeStartElement(KXMLQLCVCButtonFunction);
    doc->writeAttribute(KXMLQLCVCButtonFunctionID, QString::number(functionID()));
    doc->writeEndElement();

    /* Action */
    doc->writeStartElement(KXMLQLCVCButtonAction);

    if (actionType() == StopAll && stopAllFadeOutTime() != 0)
    {
        doc->writeAttribute(KXMLQLCVCButtonStopAllFadeTime, QString::number(stopAllFadeOutTime()));
    }
    else if (actionType() == Flash)
    {
        doc->writeAttribute(KXMLQLCVCButtonFlashOverride, QString::number(flashOverrides()));
        doc->writeAttribute(KXMLQLCVCButtonFlashForceLTP, QString::number(flashForceLTP()));
    }

    doc->writeCharacters(actionToString(actionType()));
    doc->writeEndElement();

    /* External control */
    saveXMLInputControl(doc, INPUT_PRESSURE_ID, false);

    /* Intensity adjustment */
    if (startupIntensityEnabled())
    {
        doc->writeStartElement(KXMLQLCVCButtonIntensity);
        doc->writeCharacters(QString::number(int(startupIntensity() * 100)));
        doc->writeEndElement();
    }

    /* End the <Button> tag */
    doc->writeEndElement();

    return true;
}
