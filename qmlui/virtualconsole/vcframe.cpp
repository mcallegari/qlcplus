/*
  Q Light Controller Plus
  vcframe.cpp

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

#include <QDebug>
#include <QtXml>

#include "vcframe.h"
#include "vcbutton.h"

VCFrame::VCFrame(Doc *doc, QObject *parent)
    : VCWidget(doc, parent)
    , m_showHeader(false)
    , m_showEnable(false)
    , m_multipageMode(false)
{
    setType(VCWidget::FrameWidget);
}

VCFrame::~VCFrame()
{
}

void VCFrame::render(QQuickView *view, QQuickItem *parent)
{
    if (view == NULL || parent == NULL)
        return;

    QQmlComponent *component = new QQmlComponent(view->engine(), QUrl("qrc:/VCFrameItem.qml"));

    if (component->isError())
    {
        qDebug() << component->errors();
        return;
    }

    QQuickItem *item = qobject_cast<QQuickItem*>(component->create());

    item->setParentItem(parent);
    item->setProperty("frameObj", QVariant::fromValue(this));

    foreach(VCWidget *child, m_childrenList)
        child->render(view, item);
}

bool VCFrame::hasChildren()
{
    return !m_childrenList.isEmpty();
}

QList<VCWidget *> VCFrame::children()
{
    return m_childrenList;
}

/*********************************************************************
 * Header
 *********************************************************************/

bool VCFrame::showHeader() const
{
    return m_showHeader;
}

void VCFrame::setShowHeader(bool showHeader)
{
    if (m_showHeader == showHeader)
        return;

    m_showHeader = showHeader;
    emit showHeaderChanged(showHeader);
}

/*********************************************************************
 * Enable button
 *********************************************************************/

bool VCFrame::showEnable() const
{
    return m_showEnable;
}

void VCFrame::setShowEnable(bool showEnable)
{
    if (m_showEnable == showEnable)
        return;

    m_showEnable = showEnable;
    emit showEnableChanged(showEnable);
}

/*********************************************************************
 * Multi page mode
 *********************************************************************/

bool VCFrame::multipageMode() const
{
    return m_multipageMode;
}

void VCFrame::setMultipageMode(bool multipageMode)
{
    if (m_multipageMode == multipageMode)
        return;

    m_multipageMode = multipageMode;
    emit multipageModeChanged(multipageMode);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

QString VCFrame::xmlTagName() const
{
    return KXMLQLCVCFrame;
}

bool VCFrame::loadXML(const QDomElement* root)
{
    Q_ASSERT(root != NULL);

    if (root->tagName() != xmlTagName())
    {
        qWarning() << Q_FUNC_INFO << "Frame node not found";
        return false;
    }

    /* Widget commons */
    loadXMLCommon(root);

    /* Children */
    QDomNode node = root->firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCWindowState)
        {
            /* Frame geometry (visibility is ignored) */
            int x = 0, y = 0, w = 0, h = 0;
            bool visible = false;
            loadXMLWindowState(&tag, &x, &y, &w, &h, &visible);
            setGeometry(QRect(x, y, w, h));
        }
        else if (tag.tagName() == KXMLQLCVCWidgetAppearance)
        {
            /* Frame appearance */
            loadXMLAppearance(&tag);
        }
        else if (tag.tagName() == KXMLQLCVCFrameShowHeader)
        {
            if (tag.text() == KXMLQLCTrue)
                setShowHeader(true);
            else
                setShowHeader(false);
        }
        else if (tag.tagName() == KXMLQLCVCFrameShowEnableButton)
        {
            if (tag.text() == KXMLQLCTrue)
                setShowEnable(true);
            else
                setShowEnable(false);
        }

        /** ***************** children widgets *************************** */

        else if (tag.tagName() == KXMLQLCVCFrame)
        {
            /* Create a new frame into its parent */
            VCFrame* frame = new VCFrame(m_doc, this);
            if (frame->loadXML(&tag) == false)
                delete frame;
            else
            {
                //addWidgetToPageMap(frame);
                m_childrenList.append(frame);
            }
        }
        else if (tag.tagName() == KXMLQLCVCButton)
        {
            /* Create a new button into its parent */
            VCButton* button = new VCButton(m_doc, this);
            if (button->loadXML(&tag) == false)
                delete button;
            else
            {
                //addWidgetToPageMap(button);
                m_childrenList.append(button);
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown frame tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}


