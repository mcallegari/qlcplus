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
#include "doc.h"

VCButton::VCButton(Doc *doc, QObject *parent)
    : VCWidget(doc, parent)
    , m_functionID(Function::invalidId())
    , m_isOn(false)
    , m_actionType(Toggle)
{
    setType(VCWidget::ButtonWidget);
    setBackgroundColor(QColor("#444"));
}

VCButton::~VCButton()
{
}

void VCButton::setID(quint32 id)
{
    VCWidget::setID(id);

    if (caption().isEmpty())
        setCaption(tr("Button %1").arg(id));
}

void VCButton::render(QQuickView *view, QQuickItem *parent)
{
    if (view == NULL || parent == NULL)
        return;

    QQmlComponent *component = new QQmlComponent(view->engine(), QUrl("qrc:/VCButtonItem.qml"));

    if (component->isError())
    {
        qDebug() << component->errors();
        return;
    }

    QQuickItem *item = qobject_cast<QQuickItem*>(component->create());

    item->setParentItem(parent);
    item->setProperty("buttonObj", QVariant::fromValue(this));
}

QString VCButton::propertiesResource() const
{
    return QString("qrc:/VCButtonProperties.qml");
}

/*********************************************************************
 * Function attachment
 *********************************************************************/

void VCButton::setFunctionID(quint32 fid)
{
    bool running = false;

    if (m_functionID == fid)
        return;

    Function* current = m_doc->function(m_functionID);
    if (current != NULL)
    {
        /* Get rid of old function connections */
        disconnect(current, SIGNAL(running(quint32)),
                this, SLOT(slotFunctionRunning(quint32)));
        disconnect(current, SIGNAL(stopped(quint32)),
                this, SLOT(slotFunctionStopped(quint32)));
        disconnect(current, SIGNAL(flashing(quint32,bool)),
                this, SLOT(slotFunctionFlashing(quint32,bool)));

        if(current->isRunning())
        {
            running = true;
            current->stop(functionParent());
        }
    }

    Function* function = m_doc->function(fid);
    if (function != NULL)
    {
        /* Connect to the new function */
        connect(function, SIGNAL(running(quint32)),
                this, SLOT(slotFunctionRunning(quint32)));
        connect(function, SIGNAL(stopped(quint32)),
                this, SLOT(slotFunctionStopped(quint32)));
        connect(function, SIGNAL(flashing(quint32,bool)),
                this, SLOT(slotFunctionFlashing(quint32,bool)));

        m_functionID = fid;
        if (caption().isEmpty())
            setCaption(function->name());
        if(running)
            function->start(m_doc->masterTimer(), functionParent());
        emit functionIDChanged(fid);
    }
    else
    {
        /* No function attachment */
        m_functionID = Function::invalidId();
        emit functionIDChanged(-1);
    }
    setDocModified();
}

quint32 VCButton::functionID() const
{
    return m_functionID;
}

void VCButton::requestStateChange(bool pressed)
{
    switch(actionType())
    {
        case Toggle:
        {
            Function *f = m_doc->function(m_functionID);
            if (f == NULL)
                return;

            if (m_isOn == false && pressed == true)
            {
                static const QMetaMethod funcSignal = QMetaMethod::fromSignal(&VCButton::functionStarting);
                if (isSignalConnected(funcSignal))
                    emit functionStarting(this, m_functionID);
                else
                    notifyFunctionStarting(this, m_functionID, 1.0);
            }
            else if (m_isOn == true && pressed == false)
            {
                if (f->isRunning())
                    f->stop(functionParent());
            }
        }
        break;
        case Flash:
        {
            Function *f = m_doc->function(m_functionID);
            if (f != NULL)
            {
                if (m_isOn == false && pressed == true)
                {
                    f->flash(m_doc->masterTimer());
                    setOn(true);
                }
                else if (m_isOn == true && pressed == false)
                {
                    f->unFlash(m_doc->masterTimer());
                    setOn(false);
                }
            }
        }
        break;
        default:
        break;
    }
}

void VCButton::notifyFunctionStarting(VCWidget *widget, quint32 fid, qreal fIntensity)
{
    Q_UNUSED(widget)
    Q_UNUSED(fIntensity)

    if (m_functionID == Function::invalidId() || actionType() != VCButton::Toggle)
        return;

    Function *f = m_doc->function(m_functionID);
    if (f == NULL)
        return;

    if (m_functionID != fid)
    {
        if (f->isRunning())
            f->stop(functionParent());
    }
    else
    {
        if (isStartupIntensityEnabled() == true)
            f->adjustAttribute(startupIntensity() * intensity(), Function::Intensity);
        else
            f->adjustAttribute(intensity(), Function::Intensity);
        f->start(m_doc->masterTimer(), functionParent());
    }
}

void VCButton::slotFunctionRunning(quint32 fid)
{
    if (fid == m_functionID && actionType() == Toggle)
        setOn(true);
}

void VCButton::slotFunctionStopped(quint32 fid)
{
    if (fid == m_functionID && actionType() == Toggle)
    {
        setOn(false);
        //blink(250);
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
    if (state == false && actionType() == Toggle && f != NULL && f->isRunning())
    {
        return;
    }

    setOn(state);
}

FunctionParent VCButton::functionParent() const
{
    return FunctionParent(FunctionParent::ManualVCWidget, id());
}

/*********************************************************************
 * Button state
 *********************************************************************/

bool VCButton::isOn() const
{
    return m_isOn;
}

void VCButton::setOn(bool isOn)
{
    if (m_isOn == isOn)
        return;

    if (m_functionID == Function::invalidId())
        return;

    m_isOn = isOn;
    emit isOnChanged(isOn);
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
    m_blackoutFadeOutTime = ms;
}

int VCButton::stopAllFadeTime()
{
    return m_blackoutFadeOutTime;
}


/*****************************************************************************
 * Intensity adjustment
 *****************************************************************************/

void VCButton::enableStartupIntensity(bool enable)
{
    m_startupIntensityEnabled = enable;
}

bool VCButton::isStartupIntensityEnabled() const
{
    return m_startupIntensityEnabled;
}

void VCButton::setStartupIntensity(qreal fraction)
{
    m_startupIntensity = CLAMP(fraction, qreal(0), qreal(1));
}

qreal VCButton::startupIntensity() const
{
    return m_startupIntensity;
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
            //QXmlStreamAttributes attrs = root.attributes();
            setActionType(stringToAction(root.readElementText()));
            /*
            if (tag.hasAttribute(KXMLQLCVCButtonStopAllFadeTime))
                setStopAllFadeOutTime(tag.attribute(KXMLQLCVCButtonStopAllFadeTime).toInt());
            */
        }
        else if (root.name() == KXMLQLCVCButtonIntensity)
        {
            bool adjust;
            if (root.attributes().value(KXMLQLCVCButtonIntensityAdjust).toString() == KXMLQLCTrue)
                adjust = true;
            else
                adjust = false;
            setStartupIntensity(qreal(root.readElementText().toInt()) / qreal(100));
            enableStartupIntensity(adjust);
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
    Q_ASSERT(doc != NULL);

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

    if (actionType() == StopAll && stopAllFadeTime() != 0)
        doc->writeAttribute(KXMLQLCVCButtonStopAllFadeTime, QString::number(stopAllFadeTime()));

    doc->writeCharacters(actionToString(actionType()));
    doc->writeEndElement();

#if 0 // TODO
    /* Key sequence */
    if (m_keySequence.isEmpty() == false)
        doc->writeTextElement(KXMLQLCVCButtonKey, m_keySequence.toString());
#endif
    /* Intensity adjustment */
    doc->writeStartElement(KXMLQLCVCButtonIntensity);
    doc->writeAttribute(KXMLQLCVCButtonIntensityAdjust,
                     isStartupIntensityEnabled() ? KXMLQLCTrue : KXMLQLCFalse);
    doc->writeCharacters(QString::number(int(startupIntensity() * 100)));
    doc->writeEndElement();

#if 0 // TODO
    /* External input */
    saveXMLInput(doc);
#endif

    /* End the <Button> tag */
    doc->writeEndElement();

    return true;
}
