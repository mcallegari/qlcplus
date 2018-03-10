/*
  Q Light Controller Plus
  vccuelist.cpp

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

#include "chasereditor.h"
#include "vccuelist.h"
#include "listmodel.h"
#include "chaser.h"
#include "tardis.h"

#define INPUT_NEXT_STEP_ID          0
#define INPUT_PREVIOUS_STEP_ID      1
#define INPUT_PLAY_PAUSE_ID         2
#define INPUT_CROSSFADE_L_ID        3
#define INPUT_CROSSFADE_R_ID        4
#define INPUT_STOP_PAUSE_ID         5

VCCueList::VCCueList(Doc *doc, QObject *parent)
    : VCWidget(doc, parent)
    , m_chaserID(Function::invalidId())
{
    setType(VCWidget::CueListWidget);

    registerExternalControl(INPUT_NEXT_STEP_ID, tr("Next Cue"), true);
    registerExternalControl(INPUT_PREVIOUS_STEP_ID, tr("Previous Cue"), true);
    registerExternalControl(INPUT_PLAY_PAUSE_ID, tr("Play/Stop/Pause"), true);
    registerExternalControl(INPUT_CROSSFADE_L_ID, tr("Left Crossfade"), false);
    registerExternalControl(INPUT_CROSSFADE_R_ID, tr("Right Crossfade"), false);
    registerExternalControl(INPUT_STOP_PAUSE_ID, tr("Stop/Pause"), true);

    m_stepsList = new ListModel(this);
    QStringList listRoles;
    listRoles << "funcID" << "isSelected" << "fadeIn" << "hold" << "fadeOut" << "duration" << "note";
    m_stepsList->setRoleNames(listRoles);
}

VCCueList::~VCCueList()
{
    if (m_item)
        delete m_item;
}

QString VCCueList::defaultCaption()
{
    return tr("Cue List %1").arg(id() + 1);
}

void VCCueList::setupLookAndFeel(qreal pixelDensity, int page)
{
    setPage(page);
    setDefaultFontSize(pixelDensity * 3.5);
}

void VCCueList::render(QQuickView *view, QQuickItem *parent)
{
    if (view == NULL || parent == NULL)
        return;

    QQmlComponent *component = new QQmlComponent(view->engine(), QUrl("qrc:/VCCueListItem.qml"));

    if (component->isError())
    {
        qDebug() << component->errors();
        return;
    }

    m_item = qobject_cast<QQuickItem*>(component->create());

    m_item->setParentItem(parent);
    m_item->setProperty("cueListObj", QVariant::fromValue(this));
}

QString VCCueList::propertiesResource() const
{
    return QString("qrc:/VCCueListProperties.qml");
}

/*********************************************************************
 * Chaser attachment
 *********************************************************************/

FunctionParent VCCueList::functionParent() const
{
    return FunctionParent(FunctionParent::ManualVCWidget, id());
}

Chaser *VCCueList::chaser()
{
    if (m_chaserID == Function::invalidId())
        return NULL;
    Chaser *chaser = qobject_cast<Chaser*>(m_doc->function(m_chaserID));
    return chaser;
}

QVariant VCCueList::stepsList() const
{
    return QVariant::fromValue(m_stepsList);
}

quint32 VCCueList::chaserID() const
{
    return m_chaserID;
}

void VCCueList::setChaserID(quint32 fid)
{
    bool running = false;

    if (m_chaserID == fid)
        return;

    Function *current = m_doc->function(m_chaserID);
    Function *function = m_doc->function(fid);

    if (current != NULL)
    {
        if(current->isRunning())
        {
            running = true;
            current->stop(functionParent());
        }
    }

    if (function != NULL)
    {
        m_chaserID = fid;
        if ((isEditing() && caption().isEmpty()) || caption() == defaultCaption())
            setCaption(function->name());

        ChaserEditor::updateStepsList(m_doc, chaser(), m_stepsList);
        emit stepsListChanged();

        if(running)
        {
            function->start(m_doc->masterTimer(), functionParent());
        }
        emit chaserIDChanged(fid);
    }
    else
    {
        /* No function attachment */
        m_chaserID = Function::invalidId();
        emit chaserIDChanged(-1);
    }

    Tardis::instance()->enqueueAction(VCCueListSetChaserID, id(),
                                      current ? current->id() : Function::invalidId(),
                                      function ? function->id() : Function::invalidId());
}


/*********************************************************************
 * Load & Save
 *********************************************************************/

bool VCCueList::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCCueList)
    {
        qWarning() << Q_FUNC_INFO << "Cue List node not found";
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
        else if (root.name() == KXMLQLCVCCueListChaser)
        {
            setChaserID(root.readElementText().toUInt());
        }
        else if (root.name() == KXMLQLCVCCueListNext)
        {
            loadXMLSources(root, INPUT_NEXT_STEP_ID);
        }
        else if (root.name() == KXMLQLCVCCueListPrevious)
        {
            loadXMLSources(root, INPUT_PREVIOUS_STEP_ID);
        }
        else if (root.name() == KXMLQLCVCCueListPlayback)
        {
            loadXMLSources(root, INPUT_PLAY_PAUSE_ID);
        }
        else if (root.name() == KXMLQLCVCCueListStop)
        {
            loadXMLSources(root, INPUT_STOP_PAUSE_ID);
        }
        else if (root.name() == KXMLQLCVCCueListCrossfadeLeft)
        {
            loadXMLSources(root, INPUT_CROSSFADE_L_ID);
        }
        else if (root.name() == KXMLQLCVCCueListCrossfadeRight)
        {
            loadXMLSources(root, INPUT_CROSSFADE_R_ID);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown label tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool VCCueList::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* VC Cue List entry */
    doc->writeStartElement(KXMLQLCVCCueList);

    saveXMLCommon(doc);

    /* Window state */
    saveXMLWindowState(doc);

    /* Appearance */
    saveXMLAppearance(doc);

    /* End the <CueList> tag */
    doc->writeEndElement();

    return true;
}
