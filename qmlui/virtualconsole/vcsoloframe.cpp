/*
  Q Light Controller Plus
  vcsoloframe.cpp

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

#include "vcsoloframe.h"

VCSoloFrame::VCSoloFrame(Doc *doc, VirtualConsole *vc, QObject *parent)
    : VCFrame(doc, vc, parent)
{
    setType(VCWidget::SoloFrameWidget);
}

VCSoloFrame::~VCSoloFrame()
{
}

QString VCSoloFrame::defaultCaption()
{
    return tr("Solo Frame %1").arg(id() + 1);
}

void VCSoloFrame::render(QQuickView *view, QQuickItem *parent)
{
    if (view == nullptr || parent == nullptr)
        return;

    QQmlComponent *component = new QQmlComponent(view->engine(), QUrl("qrc:/VCFrameItem.qml"));

    if (component->isError())
    {
        qDebug() << component->errors();
        return;
    }

    QQuickItem *item = qobject_cast<QQuickItem*>(component->create());

    item->setParentItem(parent);
    item->setProperty("isSolo", true);
    item->setProperty("frameObj", QVariant::fromValue(this));

    if (m_pagesMap.count() > 0)
    {
        QString chName = QString("frameDropArea%1").arg(id());
        QQuickItem *childrenArea = qobject_cast<QQuickItem*>(item->findChild<QObject *>(chName));

        foreach (VCWidget *child, m_pagesMap.keys())
            child->render(view, childrenArea);
    }
}

VCWidget *VCSoloFrame::createCopy(VCWidget *parent)
{
    Q_ASSERT(parent != nullptr);

    VCSoloFrame *frame = new VCSoloFrame(m_doc, m_vc, parent);
    if (frame->copyFrom(this) == false)
    {
        delete frame;
        frame = nullptr;
    }

    return frame;
}

bool VCSoloFrame::copyFrom(const VCWidget *widget)
{
    const VCSoloFrame *frame = qobject_cast<const VCSoloFrame*> (widget);
    if (frame == nullptr)
        return false;

    // setSoloframeMixing(frame->soloframeMixing()); // TODO

    return VCFrame::copyFrom(widget);
}

/*********************************************************************
 * Widget Function
 *********************************************************************/

void VCSoloFrame::slotFunctionStarting(VCWidget *widget, quint32 fid, qreal intensity)
{
    qDebug() << "[VCSoloFrame] requested to start a Function with ID:" << fid << intensity << widget->caption();
    foreach (VCWidget *child, children(true))
    {
        if (child != widget)
            child->notifyFunctionStarting(widget, fid, intensity);
    }
    widget->notifyFunctionStarting(widget, fid, intensity);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

QString VCSoloFrame::xmlTagName() const
{
    return KXMLQLCVCSoloFrame;
}
