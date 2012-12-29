/*
  Q Light Controller
  vccuelist.cpp

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QString>
#include <QDebug>
#include <QtXml>

#include "vccuelistproperties.h"
#include "virtualconsole.h"
#include "chaserrunner.h"
#include "mastertimer.h"
#include "chaserstep.h"
#include "vccuelist.h"
#include "function.h"
#include "inputmap.h"
#include "qlcfile.h"
#include "chaser.h"
#include "doc.h"

#define COL_NUM  0
#define COL_NAME 1
#define PROP_ID  Qt::UserRole
#define HYSTERESIS 3 // Hysteresis for next/previous external input

const quint8 VCCueList::nextInputSourceId = 0;
const quint8 VCCueList::previousInputSourceId = 1;
const quint8 VCCueList::stopInputSourceId = 2;

VCCueList::VCCueList(QWidget* parent, Doc* doc) : VCWidget(parent, doc)
    , m_chaser(Function::invalidId())
    , m_runner(NULL)
    , m_stop(false)
{
    /* Set the class name "VCCueList" as the object name as well */
    setObjectName(VCCueList::staticMetaObject.className());

    /* Create a layout for this widget */
    new QVBoxLayout(this);
    layout()->setSpacing(2);

    /* Create a list for scenes (cues) */
    m_tree = new QTreeWidget(this);
    layout()->addWidget(m_tree);
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tree->setAlternatingRowColors(true);
    m_tree->setAllColumnsShowFocus(true);
    m_tree->setRootIsDecorated(false);
    m_tree->setItemsExpandable(false);
    m_tree->header()->setSortIndicatorShown(false);
    m_tree->header()->setClickable(false);
    m_tree->header()->setMovable(false);
    m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);
    connect(m_tree, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(slotItemActivated(QTreeWidgetItem*)));

    /* Create a layout for this widget */
    QHBoxLayout *hbox = new QHBoxLayout(this);
    hbox->setSpacing(2);

    /* Create a stop button */
    m_stopButton = new QPushButton(this);
    m_stopButton->setText(tr("Stop"));
    hbox->addWidget(m_stopButton);
    connect(m_stopButton, SIGNAL(clicked()), this, SLOT(slotStop()));

    /* Create a stop button */
    m_recordButton = new QPushButton(this);
    m_recordButton->setText(tr("Record"));
    hbox->addWidget(m_recordButton);
    connect(m_recordButton, SIGNAL(clicked()), this, SLOT(slotRecord()));

    layout()->addItem(hbox);

    setFrameStyle(KVCFrameStyleSunken);
    setCaption(tr("Cue list"));
    resize(QSize(200, 200));

    slotModeChanged(mode());

    connect(m_doc, SIGNAL(functionRemoved(quint32)),
            this, SLOT(slotFunctionRemoved(quint32)));
    connect(m_doc, SIGNAL(functionChanged(quint32)),
            this, SLOT(slotFunctionChanged(quint32)));

    m_nextLatestValue = 0;
    m_previousLatestValue = 0;
    m_stopLatestValue = 0;
}

VCCueList::~VCCueList()
{
    m_doc->masterTimer()->unregisterDMXSource(this);
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

VCWidget* VCCueList::createCopy(VCWidget* parent)
{
    Q_ASSERT(parent != NULL);

    VCCueList* cuelist = new VCCueList(parent, m_doc);
    if (cuelist->copyFrom(this) == false)
    {
        delete cuelist;
        cuelist = NULL;
    }

    return cuelist;
}

bool VCCueList::copyFrom(VCWidget* widget)
{
    VCCueList* cuelist = qobject_cast<VCCueList*> (widget);
    if (cuelist == NULL)
        return false;

    /* Function list contents */
    setChaser(cuelist->chaser());

    /* Key sequence */
    setNextKeySequence(cuelist->nextKeySequence());
    setPreviousKeySequence(cuelist->previousKeySequence());
    setStopKeySequence(cuelist->stopKeySequence());

    /* Common stuff */
    return VCWidget::copyFrom(widget);
}

/*****************************************************************************
 * Cue list
 *****************************************************************************/

void VCCueList::setChaser(quint32 id)
{
    Chaser* chaser = qobject_cast<Chaser*> (m_doc->function(id));
    if (chaser == NULL)
        m_chaser = Function::invalidId();
    else
        m_chaser = id;
    updateStepList();
}

quint32 VCCueList::chaser() const
{
    return m_chaser;
}

void VCCueList::updateStepList()
{
    m_tree->clear();

    Chaser* chaser = qobject_cast<Chaser*> (m_doc->function(m_chaser));
    if (chaser == NULL)
        return;

    QListIterator <ChaserStep> it(chaser->steps());
    while (it.hasNext() == true)
    {
        ChaserStep step(it.next());

        Function* function = m_doc->function(step.fid);
        Q_ASSERT(function != NULL);

        QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
        int index = m_tree->indexOfTopLevelItem(item) + 1;
        item->setText(COL_NUM, QString("%1").arg(index));
        item->setText(COL_NAME, function->name());
        item->setData(COL_NUM, PROP_ID, function->id());
    }
}

void VCCueList::slotFunctionRemoved(quint32 fid)
{
    if (fid == m_chaser)
    {
        setChaser(Function::invalidId());
        updateStepList();
    }
}

void VCCueList::slotFunctionChanged(quint32 fid)
{
    if (fid == m_chaser)
        updateStepList();
}

void VCCueList::slotNextCue()
{
    if (mode() != Doc::Operate)
        return;

    /* Create the runner only when the first/last cue is engaged. */
    m_mutex.lock();
    if (m_runner == NULL)
        createRunner();
    else
        m_runner->next();
    m_mutex.unlock();
}

void VCCueList::slotPreviousCue()
{
    if (mode() != Doc::Operate)
        return;

    /* Create the runner only when the first/last cue is engaged. */
    m_mutex.lock();
    if (m_runner == NULL)
        createRunner(m_tree->topLevelItemCount() - 1); // Start from end
    else
        m_runner->previous();
    m_mutex.unlock();
}

void VCCueList::slotStop()
{
    if (mode() != Doc::Operate)
        return;

    m_mutex.lock();
    if (m_runner != NULL)
        m_stop = true;
    m_mutex.unlock();

    /* Start from the beginning */
    m_tree->setCurrentItem(NULL);
    m_recordButton->setText(tr("Record"));
}

void VCCueList::slotRecord()
{
    if (mode() != Doc::Operate)
        return;

    Chaser* chaser = qobject_cast<Chaser*> (m_doc->function(m_chaser));
    /* a chaser has to attached to the cuelist */
    if (chaser == NULL)
        return;

    const UniverseArray* universes = m_doc->outputMap()->claimUniverses();
    const QByteArray* postvals = universes->postGMValues();
    m_doc->outputMap()->releaseUniverses(false);

    /* cuelist is active, get chaser step and overwrite values of the current step*/
    if (m_tree->currentItem() != NULL)
    {
        int currentStep = m_runner->currentStep();
        ChaserStep step = chaser->steps()[currentStep];

        Function* function = m_doc->function(step.fid);
        Q_ASSERT(function != NULL);

        if (function->type() == Function::Scene)
        {
            Scene* scene = qobject_cast<Scene*> (m_doc->function(function->id()));

            QListIterator <Fixture*> it(m_doc->fixtures ());

            while (it.hasNext())
            {
                //SceneValue scenevalue = ??
                Fixture* fxi = it.next();

                if ( fxi->isDimmer() )
                {
                    for ( quint32 i = 0; i < fxi->channels(); i++ )
                    {
                        uchar curval = uchar(postvals->at(fxi->channelAddress(i)));
                        if (curval > 0)
                        {
                            if (curval != scene->value(fxi->id(), i))
                                scene->setValue(fxi->id(), i, curval);
                        }
                        else
                        {
                            if (scene->value(fxi->id(), i) > 0)
                                scene->unsetValue(fxi->id(), i);
                        }
                    }
                }
            }
        }
    }
    else //cuelist not active, add a new scene and append it to the cuelist
    {
        Scene* scene = new Scene(m_doc);

        /* add new function and append it to the Cuelist */
        m_doc->addFunction(scene);
        quint32 fid = scene->id();
        scene->setName(QString("Output Snapshot %1").arg(fid));

        /* fill scene with current values from Outputmap (Dimmers only) */
        QListIterator <Fixture*> it(m_doc->fixtures ());

        while (it.hasNext())
        {
            Fixture* fxi = it.next();

            if ( fxi->isDimmer() )
            {
                for ( quint32 i = 0; i < fxi->channels(); i++ )
                {
                    uchar value = uchar(postvals->at(fxi->channelAddress(i)));
                    if (value)
                       scene->setValue(fxi->id(), i, value);
                }
            }
        }
        ChaserStep step(scene->id());
        chaser->addStep(step);
    }
}

void VCCueList::slotCurrentStepChanged(int stepNumber)
{
    Q_ASSERT(stepNumber < m_tree->topLevelItemCount() && stepNumber >= 0);
    QTreeWidgetItem* item = m_tree->topLevelItem(stepNumber);
    Q_ASSERT(item != NULL);
    m_tree->scrollToItem(item, QAbstractItemView::PositionAtCenter);
    m_tree->setCurrentItem(item);
}

void VCCueList::slotItemActivated(QTreeWidgetItem* item)
{
    if (mode() != Doc::Operate)
        return;

    m_mutex.lock();
    if (m_runner == NULL)
        createRunner(m_tree->indexOfTopLevelItem(item));
    else
        m_runner->setCurrentStep(m_tree->indexOfTopLevelItem(item));
    m_mutex.unlock();
}

void VCCueList::createRunner(int startIndex)
{
    Q_ASSERT(m_runner == NULL);

    Chaser* chaser = qobject_cast<Chaser*> (m_doc->function(m_chaser));
    if (chaser != NULL)
    {
        m_runner = Chaser::createRunner(chaser, m_doc);
        m_runner->setCurrentStep(startIndex);
        connect(m_runner, SIGNAL(currentStepChanged(int)),
                this, SLOT(slotCurrentStepChanged(int)));
        m_recordButton->setText(tr("Update"));
    }
}

/*****************************************************************************
 * DMX Source
 *****************************************************************************/

void VCCueList::writeDMX(MasterTimer* timer, UniverseArray* universes)
{
    m_mutex.lock();
    if (m_runner != NULL)
    {
        if (m_stop == false)
        {
            m_runner->write(timer, universes);
        }
        else
        {
            m_runner->postRun(timer, universes);
            delete m_runner;
            m_runner = NULL;
            m_stop = false;
        }
    }
    m_mutex.unlock();
}

/*****************************************************************************
 * Key Sequences
 *****************************************************************************/

void VCCueList::setNextKeySequence(const QKeySequence& keySequence)
{
    m_nextKeySequence = QKeySequence(keySequence);
}

QKeySequence VCCueList::nextKeySequence() const
{
    return m_nextKeySequence;
}

void VCCueList::setPreviousKeySequence(const QKeySequence& keySequence)
{
    m_previousKeySequence = QKeySequence(keySequence);
}

QKeySequence VCCueList::previousKeySequence() const
{
    return m_previousKeySequence;
}

void VCCueList::setStopKeySequence(const QKeySequence& keySequence)
{
    m_stopKeySequence = QKeySequence(keySequence);
}

QKeySequence VCCueList::stopKeySequence() const
{
    return m_stopKeySequence;
}

void VCCueList::slotKeyPressed(const QKeySequence& keySequence)
{
    if (m_nextKeySequence == keySequence)
        slotNextCue();
    else if (m_previousKeySequence == keySequence)
        slotPreviousCue();
    else if (m_stopKeySequence == keySequence)
        slotStop();
}

/*****************************************************************************
 * External Input
 *****************************************************************************/

void VCCueList::slotInputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    QLCInputSource src(universe, channel);

    if (src == inputSource(nextInputSourceId))
    {
        // Use hysteresis for values, in case the cue list is being controlled
        // by a slider. The value has to go to zero before the next non-zero
        // value is accepted as input. And the non-zero values have to visit
        // above $HYSTERESIS before a zero is accepted again.
        if (m_nextLatestValue == 0 && value > 0)
        {
            slotNextCue();
            m_nextLatestValue = value;
        }
        else if (m_nextLatestValue > HYSTERESIS && value == 0)
        {
            m_nextLatestValue = 0;
        }

        if (value > HYSTERESIS)
            m_nextLatestValue = value;
    }
    else if (src == inputSource(previousInputSourceId))
    {
        // Use hysteresis for values, in case the cue list is being controlled
        // by a slider. The value has to go to zero before the next non-zero
        // value is accepted as input. And the non-zero values have to visit
        // above $HYSTERESIS before a zero is accepted again.
        if (m_previousLatestValue == 0 && value > 0)
        {
            slotPreviousCue();
            m_previousLatestValue = value;
        }
        else if (m_previousLatestValue > HYSTERESIS && value == 0)
        {
            m_previousLatestValue = 0;
        }

        if (value > HYSTERESIS)
            m_previousLatestValue = value;
    }
    else if (src == inputSource(stopInputSourceId))
    {
        // Use hysteresis for values, in case the cue list is being controlled
        // by a slider. The value has to go to zero before the next non-zero
        // value is accepted as input. And the non-zero values have to visit
        // above $HYSTERESIS before a zero is accepted again.
        if (m_stopLatestValue == 0 && value > 0)
        {
            slotStop();
            m_stopLatestValue = value;
        }
        else if (m_stopLatestValue > HYSTERESIS && value == 0)
        {
            m_stopLatestValue = 0;
        }

        if (value > HYSTERESIS)
            m_stopLatestValue = value;
    }
}

/*****************************************************************************
 * VCWidget-inherited methods
 *****************************************************************************/

void VCCueList::setCaption(const QString& text)
{
    VCWidget::setCaption(text);

    QStringList list;
    list << tr("Number") << text;
    m_tree->setHeaderLabels(list);
}

void VCCueList::slotModeChanged(Doc::Mode mode)
{
    if (mode == Doc::Operate)
    {
        Q_ASSERT(m_runner == NULL);
        m_doc->masterTimer()->registerDMXSource(this);
        m_tree->setEnabled(true);
        m_recordButton->setEnabled(true);
        m_stopButton->setEnabled(true);
    }
    else
    {
        m_doc->masterTimer()->unregisterDMXSource(this);
        m_mutex.lock();
        if (m_runner != NULL)
            delete m_runner;
        m_runner = NULL;
        m_mutex.unlock();
        m_tree->setEnabled(false);
        m_recordButton->setEnabled(false);
        m_stopButton->setEnabled(false);
    }

    /* Always start from the beginning */
    m_tree->setCurrentItem(NULL);

    VCWidget::slotModeChanged(mode);
}

void VCCueList::editProperties()
{
    VCCueListProperties prop(this, m_doc);
    if (prop.exec() == QDialog::Accepted)
        m_doc->setModified();
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCCueList::loadXML(const QDomElement* root)
{
    QList <quint32> legacyStepList;

    QDomNode node;
    QDomElement tag;
    QString str;

    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCVCCueList)
    {
        qWarning() << Q_FUNC_INFO << "CueList node not found";
        return false;
    }

    /* Caption */
    setCaption(root->attribute(KXMLQLCVCCaption));

    /* Children */
    node = root->firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();
        if (tag.tagName() == KXMLQLCWindowState)
        {
            bool visible = false;
            int x = 0, y = 0, w = 0, h = 0;
            loadXMLWindowState(&tag, &x, &y, &w, &h, &visible);
            setGeometry(x, y, w, h);
        }
        else if (tag.tagName() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(&tag);
        }
        else if (tag.tagName() == KXMLQLCVCCueListNext)
        {
            QDomNode subNode = tag.firstChild();
            while (subNode.isNull() == false)
            {
                QDomElement subTag = subNode.toElement();
                if (subTag.tagName() == KXMLQLCVCWidgetInput)
                {
                    quint32 uni = 0;
                    quint32 ch = 0;
                    if (loadXMLInput(subTag, &uni, &ch) == true)
                        setInputSource(QLCInputSource(uni, ch), nextInputSourceId);
                }
                else if (subTag.tagName() == KXMLQLCVCCueListKey)
                {
                    m_nextKeySequence = stripKeySequence(QKeySequence(subTag.text()));
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Unknown CueList Next tag" << subTag.tagName();
                }

                subNode = subNode.nextSibling();
            }
        }
        else if (tag.tagName() == KXMLQLCVCCueListPrevious)
        {
            QDomNode subNode = tag.firstChild();
            while (subNode.isNull() == false)
            {
                QDomElement subTag = subNode.toElement();
                if (subTag.tagName() == KXMLQLCVCWidgetInput)
                {
                    quint32 uni = 0;
                    quint32 ch = 0;
                    if (loadXMLInput(subTag, &uni, &ch) == true)
                        setInputSource(QLCInputSource(uni, ch), previousInputSourceId);
                }
                else if (subTag.tagName() == KXMLQLCVCCueListKey)
                {
                    m_previousKeySequence = stripKeySequence(QKeySequence(subTag.text()));
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Unknown CueList Previous tag" << subTag.tagName();
                }

                subNode = subNode.nextSibling();
            }
        }
        else if (tag.tagName() == KXMLQLCVCCueListStop)
        {
            QDomNode subNode = tag.firstChild();
            while (subNode.isNull() == false)
            {
                QDomElement subTag = subNode.toElement();
                if (subTag.tagName() == KXMLQLCVCWidgetInput)
                {
                    quint32 uni = 0;
                    quint32 ch = 0;
                    if (loadXMLInput(subTag, &uni, &ch) == true)
                        setInputSource(QLCInputSource(uni, ch), stopInputSourceId);
                }
                else if (subTag.tagName() == KXMLQLCVCCueListKey)
                {
                    m_stopKeySequence = stripKeySequence(QKeySequence(subTag.text()));
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Unknown CueList Stop tag" << subTag.tagName();
                }

                subNode = subNode.nextSibling();
            }
        }
        else if (tag.tagName() == KXMLQLCVCCueListKey) /* Legacy */
        {
            setNextKeySequence(QKeySequence(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCVCCueListChaser)
        {
            setChaser(tag.text().toUInt());
        }
        else if (tag.tagName() == KXMLQLCVCCueListFunction)
        {
            // Collect legacy file format steps into a list
            legacyStepList << tag.text().toUInt();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown cuelist tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    if (legacyStepList.isEmpty() == false)
    {
        /* Construct a new chaser from legacy step functions and use that chaser */
        Chaser* chaser = new Chaser(m_doc);
        chaser->setName(caption());

        // Legacy cue lists relied on individual functions' timings and a common hold time
        chaser->setFadeInMode(Chaser::Default);
        chaser->setFadeOutMode(Chaser::Default);
        chaser->setDurationMode(Chaser::Common);

        foreach (quint32 id, legacyStepList)
        {
            Function* function = m_doc->function(id);
            if (function == NULL)
                continue;

            // Legacy cuelists relied on individual functions' fadein/out speed and
            // infinite duration. So don't touch them at all.
            ChaserStep step(id);
            chaser->addStep(step);
        }

        // Add the chaser to Doc and attach it to the cue list
        m_doc->addFunction(chaser);
        setChaser(chaser->id());
    }

    return true;
}

bool VCCueList::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
    QDomElement root;
    QDomElement tag;
    QDomElement subtag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(vc_root != NULL);

    /* VC CueList entry */
    root = doc->createElement(KXMLQLCVCCueList);
    vc_root->appendChild(root);

    /* Caption */
    root.setAttribute(KXMLQLCVCCaption, caption());

    /* Chaser */
    tag = doc->createElement(KXMLQLCVCCueListChaser);
    root.appendChild(tag);
    text = doc->createTextNode(QString::number(chaser()));
    tag.appendChild(text);

    /* Next cue */
    tag = doc->createElement(KXMLQLCVCCueListNext);
    root.appendChild(tag);
    subtag = doc->createElement(KXMLQLCVCCueListKey);
    tag.appendChild(subtag);
    text = doc->createTextNode(m_nextKeySequence.toString());
    subtag.appendChild(text);
    saveXMLInput(doc, &tag, inputSource(nextInputSourceId));

    /* Previous cue */
    tag = doc->createElement(KXMLQLCVCCueListPrevious);
    root.appendChild(tag);
    subtag = doc->createElement(KXMLQLCVCCueListKey);
    tag.appendChild(subtag);
    text = doc->createTextNode(m_previousKeySequence.toString());
    subtag.appendChild(text);
    saveXMLInput(doc, &tag, inputSource(previousInputSourceId));

    /* Stop cue list */
    tag = doc->createElement(KXMLQLCVCCueListStop);
    root.appendChild(tag);
    subtag = doc->createElement(KXMLQLCVCCueListKey);
    tag.appendChild(subtag);
    text = doc->createTextNode(m_stopKeySequence.toString());
    subtag.appendChild(text);
    saveXMLInput(doc, &tag, inputSource(stopInputSourceId));

    /* Window state */
    saveXMLWindowState(doc, &root);

    /* Appearance */
    saveXMLAppearance(doc, &root);

    return true;
}
