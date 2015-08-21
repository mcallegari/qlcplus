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

#include <QtXml>

#include "vcbutton.h"
#include "doc.h"

VCButton::VCButton(Doc *doc, QObject *parent)
    : VCWidget(doc, parent)
    , m_function(Function::invalidId())
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

/*********************************************************************
 * Function attachment
 *********************************************************************/

void VCButton::setFunction(quint32 fid)
{
    bool running = false;

    Function* current = m_doc->function(m_function);
    if (current != NULL)
    {
        if(current->isRunning())
        {
            running = true;
            current->stop();
        }
    }

    Function* function = m_doc->function(fid);
    if (function != NULL)
    {
        m_function = fid;
        if(running)
            function->start(m_doc->masterTimer());
    }
    else
    {
        /* No function attachment */
        m_function = Function::invalidId();
    }
}

quint32 VCButton::function() const
{
    return m_function;
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

    if (m_function == Function::invalidId())
        return;

    m_isOn = isOn;
    emit isOnChanged(isOn);
}

/*********************************************************************
 * Button action
 *********************************************************************/

VCButton::Action VCButton::actionType() const
{
    return m_actionType;
}

void VCButton::setActionType(VCButton::Action actionType)
{
    if (m_actionType == actionType)
        return;

    m_actionType = actionType;
    emit actionTypeChanged(actionType);
}

QString VCButton::actionToString(VCButton::Action action)
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

VCButton::Action VCButton::stringToAction(const QString& str)
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

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool VCButton::loadXML(const QDomElement* root)
{
    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCVCButton)
    {
        qWarning() << Q_FUNC_INFO << "Button node not found";
        return false;
    }

    /* Widget commons */
    loadXMLCommon(root);

    QString str;
    QDomNode node = root->firstChild();

    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCWindowState)
        {
            bool visible = false;
            int x = 0, y = 0, w = 0, h = 0;
            loadXMLWindowState(&tag, &x, &y, &w, &h, &visible);
            setGeometry(QRect(x, y, w, h));
        }
        else if (tag.tagName() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(&tag);
        }
        else if (tag.tagName() == KXMLQLCVCButtonFunction)
        {
            str = tag.attribute(KXMLQLCVCButtonFunctionID);
            setFunction(str.toUInt());
        }
        else if (tag.tagName() == KXMLQLCVCButtonAction)
        {
            setActionType(stringToAction(tag.text()));
            /*
            if (tag.hasAttribute(KXMLQLCVCButtonStopAllFadeTime))
                setStopAllFadeOutTime(tag.attribute(KXMLQLCVCButtonStopAllFadeTime).toInt());
            */
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown button tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    /* All buttons start raised... */
    //setOn(false);

    return true;
}
