/*
  Q Light Controller
  fixturemanager.cpp

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

#include <QTreeWidgetItem>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QScrollArea>
#include <QMessageBox>
#include <QToolButton>
#include <QFileDialog>
#include <QTabWidget>
#include <QSplitter>
#include <QToolBar>
#include <QAction>
#include <QString>
#include <QDebug>
#include <QIcon>
#include <QMenu>
#include <QtGui>
#include <QtXml>

#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlccapability.h"
#include "qlcchannel.h"
#include "qlcfile.h"

#include "createfixturegroup.h"
#include "fixturegroupeditor.h"
#include "channelsselection.h"
#include "addchannelsgroup.h"
#include "fixturemanager.h"
#include "universearray.h"
#include "fixtureremap.h"
#include "mastertimer.h"
#include "outputpatch.h"
#include "addfixture.h"
#include "collection.h"
#include "outputmap.h"
#include "inputmap.h"
#include "fixture.h"
#include "apputil.h"
#include "doc.h"

#define SETTINGS_SPLITTER "fixturemanager/splitterstate"

#define PROP_FIXTURE Qt::UserRole
#define PROP_GROUP   Qt::UserRole + 1

// List view column numbers
#define KColumnName     0
#define KColumnUniverse 1
#define KColumnChannels 1
#define KColumnAddress  2

#define KXMLQLCFixturesList "FixtureList"

FixtureManager* FixtureManager::s_instance = NULL;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

FixtureManager::FixtureManager(QWidget* parent, Doc* doc)
    : QWidget(parent)
    , m_doc(doc)
    , m_splitter(NULL)
    , m_fixtures_tree(NULL)
    , m_channel_groups_tree(NULL)
    , m_info(NULL)
    , m_groupEditor(NULL)
    , m_currentTabIndex(0)
    , m_addAction(NULL)
    , m_removeAction(NULL)
    , m_propertiesAction(NULL)
    , m_fadeConfigAction(NULL)
    , m_remapAction(NULL)
    , m_groupAction(NULL)
    , m_unGroupAction(NULL)
    , m_newGroupAction(NULL)
    , m_moveUpAction(NULL)
    , m_moveDownAction(NULL)
    , m_importAction(NULL)
    , m_exportAction(NULL)
    , m_groupMenu(NULL)
{
    Q_ASSERT(s_instance == NULL);
    s_instance = this;

    Q_ASSERT(doc != NULL);

    new QVBoxLayout(this);
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->setSpacing(0);

    initActions();
    initToolBar();
    initDataView();
    updateView();
    updateChannelsGroupView();

    QTreeWidgetItem* grpItem = m_fixtures_tree->topLevelItem(0);
    if (grpItem != NULL)
        grpItem->setExpanded(true);

    /* Connect fixture list change signals from the new document object */
    connect(m_doc, SIGNAL(fixtureRemoved(quint32)),
            this, SLOT(slotFixtureRemoved(quint32)));

    connect(m_doc, SIGNAL(channelsGroupRemoved(quint32)),
            this, SLOT(slotChannelsGroupRemoved(quint32)));

    connect(m_doc, SIGNAL(modeChanged(Doc::Mode)),
            this, SLOT(slotModeChanged(Doc::Mode)));

    connect(m_doc, SIGNAL(fixtureGroupRemoved(quint32)),
            this, SLOT(slotFixtureGroupRemoved(quint32)));

    connect(m_doc, SIGNAL(fixtureGroupChanged(quint32)),
            this, SLOT(slotFixtureGroupChanged(quint32)));

    connect(m_doc, SIGNAL(loaded()),
            this, SLOT(slotDocLoaded()));

    slotModeChanged(m_doc->mode());

    QSettings settings;
    QVariant var = settings.value(SETTINGS_SPLITTER);
    if (var.isValid() == true)
        m_splitter->restoreState(var.toByteArray());
    else
        m_splitter->setSizes(QList <int> () << int(this->width() / 2) << int(this->width() / 2));
}

FixtureManager::~FixtureManager()
{
    QSettings settings;
    settings.setValue(SETTINGS_SPLITTER, m_splitter->saveState());
    FixtureManager::s_instance = NULL;

    s_instance = NULL;
}

FixtureManager* FixtureManager::instance()
{
    return s_instance;
}

/*****************************************************************************
 * Doc signal handlers
 *****************************************************************************/

void FixtureManager::slotFixtureRemoved(quint32 id)
{
    for (int i = 0; i < m_fixtures_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* grpItem = m_fixtures_tree->topLevelItem(i);
        Q_ASSERT(grpItem != NULL);
        for (int j = 0; j < grpItem->childCount(); j++)
        {
            QTreeWidgetItem* fxiItem = grpItem->child(j);
            Q_ASSERT(fxiItem != NULL);
            QVariant var = fxiItem->data(KColumnName, PROP_FIXTURE);
            if (var.isValid() == true && var.toUInt() == id)
                delete fxiItem;
        }
    }
}

void FixtureManager::slotChannelsGroupRemoved(quint32 id)
{
    qDebug() << "Channel group removed: " << id;
    for (int i = 0; i < m_channel_groups_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* grpItem = m_channel_groups_tree->topLevelItem(i);
        Q_ASSERT(grpItem != NULL);
        QVariant var = grpItem->data(KColumnName, PROP_FIXTURE);
        if (var.isValid() == true && var.toUInt() == id)
            delete grpItem;
    }
}

void FixtureManager::slotModeChanged(Doc::Mode mode)
{
    if (mode == Doc::Design)
    {
        int selected = m_fixtures_tree->selectedItems().size();

        QTreeWidgetItem* item = m_fixtures_tree->currentItem();
        if (item == NULL)
        {
            m_addAction->setEnabled(true);
            m_removeAction->setEnabled(false);
            m_propertiesAction->setEnabled(false);
            m_groupAction->setEnabled(false);
            m_unGroupAction->setEnabled(false);
            m_importAction->setEnabled(true);
        }
        else if (item->data(KColumnName, PROP_FIXTURE).isValid() == true)
        {
            // Fixture selected
            m_addAction->setEnabled(true);
            m_removeAction->setEnabled(true);
            if (selected == 1)
                m_propertiesAction->setEnabled(true);
            else
                m_propertiesAction->setEnabled(false);
            m_groupAction->setEnabled(true);

            // Don't allow ungrouping from the "All fixtures" group
            if (item->parent()->data(KColumnName, PROP_GROUP).isValid() == true)
                m_unGroupAction->setEnabled(true);
            else
                m_unGroupAction->setEnabled(false);
        }
        else if (item->data(KColumnName, PROP_GROUP).isValid() == true)
        {
            // Fixture group selected
            m_addAction->setEnabled(true);
            m_removeAction->setEnabled(true);
            m_propertiesAction->setEnabled(false);
            m_groupAction->setEnabled(false);
            m_unGroupAction->setEnabled(false);
        }
        else
        {
            // All fixtures selected
            m_addAction->setEnabled(true);
            m_removeAction->setEnabled(false);
            m_propertiesAction->setEnabled(false);
            m_groupAction->setEnabled(false);
            m_unGroupAction->setEnabled(false);
        }
        if (m_doc->fixtures().count() > 0)
            m_fadeConfigAction->setEnabled(true);
        else
            m_fadeConfigAction->setEnabled(false);
    }
    else
    {
        m_addAction->setEnabled(false);
        m_removeAction->setEnabled(false);
        m_propertiesAction->setEnabled(false);
        m_fadeConfigAction->setEnabled(false);
        m_groupAction->setEnabled(false);
        m_unGroupAction->setEnabled(false);
    }
}

void FixtureManager::slotFixtureGroupRemoved(quint32 id)
{
    for (int i = 0; i < m_fixtures_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_fixtures_tree->topLevelItem(i);
        Q_ASSERT(item != NULL);
        QVariant var = item->data(KColumnName, PROP_GROUP);
        if (var.isValid() && var.toUInt() == id)
            delete item;
    }

    updateGroupMenu();
}

void FixtureManager::slotFixtureGroupChanged(quint32 id)
{
    QTreeWidgetItem* item = groupItem(id);
    if (item == NULL)
        return;

    FixtureGroup* grp = m_doc->fixtureGroup(id);
    Q_ASSERT(grp != NULL);
    updateGroupItem(item, grp);
}

void FixtureManager::slotDocLoaded()
{
    slotTabChanged(m_currentTabIndex);
}

/*****************************************************************************
 * Data view
 *****************************************************************************/

void FixtureManager::initDataView()
{
    // Create a splitter to divide list view and text view
    m_splitter = new QSplitter(Qt::Horizontal, this);
    layout()->addWidget(m_splitter);
    m_splitter->setSizePolicy(QSizePolicy::Expanding,
                              QSizePolicy::Expanding);

    QTabWidget *tabs = new QTabWidget(this);
    m_splitter->addWidget(tabs);

    /* Create a tree widget to the left part of the splitter */
    m_fixtures_tree = new QTreeWidget(this);

    QStringList labels;
    labels << tr("Name") << tr("Universe") << tr("Address");
    m_fixtures_tree->setHeaderLabels(labels);
    m_fixtures_tree->setRootIsDecorated(true);
    m_fixtures_tree->setIconSize(QSize(32, 32));
    m_fixtures_tree->setSortingEnabled(true);
    m_fixtures_tree->setAllColumnsShowFocus(true);
    m_fixtures_tree->sortByColumn(KColumnAddress, Qt::AscendingOrder);
    m_fixtures_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    m_fixtures_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    QFont m_font = QApplication::font();
    m_font.setPixelSize(13);
    m_fixtures_tree->setFont(m_font);

    connect(m_fixtures_tree, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(m_fixtures_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotDoubleClicked(QTreeWidgetItem*)));

    connect(m_fixtures_tree, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(slotContextMenuRequested(const QPoint&)));

    connect(m_fixtures_tree, SIGNAL(expanded(QModelIndex)),
            this, SLOT(slotFixtureItemExpanded()));

    connect(m_fixtures_tree, SIGNAL(collapsed(QModelIndex)),
            this, SLOT(slotFixtureItemExpanded()));

    tabs->addTab(m_fixtures_tree, tr("Fixtures Groups"));

    m_channel_groups_tree = new QTreeWidget(this);
    QStringList chan_labels;
    chan_labels << tr("Name") << tr("Channels");
    m_channel_groups_tree->setHeaderLabels(chan_labels);
    m_channel_groups_tree->setRootIsDecorated(false);
    m_channel_groups_tree->setAllColumnsShowFocus(true);
    m_channel_groups_tree->setIconSize(QSize(32, 32));
    m_channel_groups_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_channel_groups_tree->setFont(m_font);

    connect(m_channel_groups_tree, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotChannelsGroupSelectionChanged()));
    connect(m_channel_groups_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotChannelsGroupDoubleClicked(QTreeWidgetItem*)));

    tabs->addTab(m_channel_groups_tree, tr("Channels Groups"));

    connect(tabs, SIGNAL(currentChanged(int)), this, SLOT(slotTabChanged(int)));

    /* Create the text view */
    createInfo();

    slotSelectionChanged();
}

void FixtureManager::updateView()
{
    // Record which groups are open
    QList <QVariant> openGroups;
    for (int i = 0; i < m_fixtures_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_fixtures_tree->topLevelItem(i);
        if (item->isExpanded() == true)
            openGroups << item->data(KColumnName, PROP_GROUP);
    }

    // Clear the view
    m_fixtures_tree->clear();
    if (m_doc->fixtures().count() > 0)
    {
        m_exportAction->setEnabled(true);
        m_remapAction->setEnabled(true);
        m_fadeConfigAction->setEnabled(true);
    }
    else
    {
        m_exportAction->setEnabled(false);
        m_fadeConfigAction->setEnabled(false);
        m_remapAction->setEnabled(false);
    }
    m_importAction->setEnabled(true);
    m_moveUpAction->setEnabled(false);
    m_moveDownAction->setEnabled(false);

    // Insert known fixture groups and their children
    foreach (FixtureGroup* grp, m_doc->fixtureGroups())
    {
        QTreeWidgetItem* grpItem = new QTreeWidgetItem(m_fixtures_tree);
        grpItem->setIcon(KColumnName, QIcon(":/group.png"));
        updateGroupItem(grpItem, grp);
    }

    // Insert the "All fixtures group"
    QTreeWidgetItem* grpItem = new QTreeWidgetItem(m_fixtures_tree);
    grpItem->setIcon(KColumnName, QIcon(":/group.png"));
    grpItem->setText(KColumnName, tr("All fixtures"));

    // Add all fixtures under "All fixture group"
    foreach (Fixture* fixture, m_doc->fixtures())
    {
        Q_ASSERT(fixture != NULL);
        QTreeWidgetItem* item = new QTreeWidgetItem(grpItem);
        updateFixtureItem(item, fixture);
    }

    // Reopen groups that were open before update
    for (int i = 0; i < m_fixtures_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_fixtures_tree->topLevelItem(i);
        QVariant var = item->data(KColumnName, PROP_GROUP);
        if (openGroups.contains(var) == true)
        {
            item->setExpanded(true);
            openGroups.removeAll(var);
        }
    }

    updateGroupMenu();
    slotModeChanged(m_doc->mode());

    m_fixtures_tree->resizeColumnToContents(KColumnName);
    m_fixtures_tree->resizeColumnToContents(KColumnAddress);
    m_fixtures_tree->resizeColumnToContents(KColumnUniverse);
}

void FixtureManager::updateChannelsGroupView()
{
    quint32 selGroupID = ChannelsGroup::invalidId();
    //m_channel_groups_tree->clear();
    if (m_channel_groups_tree->selectedItems().size() > 0)
    {
        QTreeWidgetItem* item = m_channel_groups_tree->selectedItems().first();
        selGroupID = item->data(KColumnName, PROP_FIXTURE).toUInt();
    }

    if (m_channel_groups_tree->topLevelItemCount() > 0)
        for (int i = m_channel_groups_tree->topLevelItemCount() - 1; i >= 0; i--)
            m_channel_groups_tree->takeTopLevelItem(i);

    foreach (ChannelsGroup* grp, m_doc->channelsGroups())
    {
        QTreeWidgetItem* grpItem = new QTreeWidgetItem(m_channel_groups_tree);
        grpItem->setText(KColumnName, grp->name());
        grpItem->setData(KColumnName, PROP_FIXTURE, grp->id());
        grpItem->setText(KColumnChannels, QString("%1").arg(grp->getChannels().count()));
        if (grp->getChannels().count() > 0)
        {
            SceneValue scv = grp->getChannels().at(0);
            Fixture *fxi = m_doc->fixture(scv.fxi);
            if (fxi == NULL)
                continue;

            const QLCChannel* ch = fxi->channel(scv.channel);
            if (ch != NULL)
                grpItem->setIcon(KColumnName, ch->getIconFromGroup(ch->group()));
        }
        if (selGroupID == grp->id())
            m_channel_groups_tree->setItemSelected(grpItem, true);
    }
    m_propertiesAction->setEnabled(false);
    m_groupAction->setEnabled(false);
    m_unGroupAction->setEnabled(false);
    m_fadeConfigAction->setEnabled(false);
    m_exportAction->setEnabled(false);
    m_importAction->setEnabled(false);
    m_remapAction->setEnabled(false);

    m_channel_groups_tree->resizeColumnToContents(KColumnName);
    m_channel_groups_tree->resizeColumnToContents(KColumnChannels);
}

QTreeWidgetItem* FixtureManager::fixtureItem(quint32 id) const
{
    QTreeWidgetItemIterator it(m_fixtures_tree);
    while (*it != NULL)
    {
        QTreeWidgetItem* item(*it);
        QVariant var = item->data(KColumnName, PROP_FIXTURE);
        if (var.isValid() == true && var.toUInt() == id)
            return item;
        ++it;
    }

    return NULL;
}

QTreeWidgetItem* FixtureManager::groupItem(quint32 id) const
{
    QTreeWidgetItemIterator it(m_fixtures_tree);
    while (*it != NULL)
    {
        QTreeWidgetItem* item(*it);
        QVariant var = item->data(KColumnName, PROP_GROUP);
        if (var.isValid() == true && var.toUInt() == id)
            return item;
        ++it;
    }

    return NULL;
}

void FixtureManager::updateFixtureItem(QTreeWidgetItem* item, const Fixture* fxi)
{
    QString s;

    Q_ASSERT(item != NULL);
    if (fxi == NULL)
        return;

    // Universe column
    item->setText(KColumnUniverse, QString("%1").arg(fxi->universe() + 1));

    // Address column
    s.sprintf("%.3d - %.3d", fxi->address() + 1, fxi->address() + fxi->channels());

    item->setText(KColumnAddress, s);

    // Name column
    item->setText(KColumnName, fxi->name());

    // ID column
    item->setData(KColumnName, PROP_FIXTURE, fxi->id());

    item->setIcon(KColumnName, fxi->getIconFromType(fxi->type()));
}

void FixtureManager::updateGroupItem(QTreeWidgetItem* item, const FixtureGroup* grp)
{
    Q_ASSERT(item != NULL);
    Q_ASSERT(grp != NULL);

    item->setText(KColumnName, grp->name());
    item->setData(KColumnName, PROP_GROUP, grp->id());

    // This should be a safe check because simultaneous add/removal is not possible,
    // which could result in changes in fixtures but with the same fixture count.
    if (item->childCount() != grp->fixtureList().size())
    {
        // Remove existing children
        while (item->childCount() > 0)
            delete item->child(0);

        // Add group's children
        foreach (quint32 id, grp->fixtureList())
        {
            QTreeWidgetItem* grpItem = new QTreeWidgetItem(item);
            updateFixtureItem(grpItem, m_doc->fixture(id));
        }
    }
}

void FixtureManager::fixtureSelected(quint32 id)
{
    Fixture* fxi = m_doc->fixture(id);
    if (fxi == NULL)
        return;

    if (m_info == NULL)
        createInfo();

    m_info->setText(QString("%1<BODY>%2</BODY></HTML>")
                    .arg(fixtureInfoStyleSheetHeader())
                    .arg(fxi->status()));

    // Enable/disable actions
    slotModeChanged(m_doc->mode());
}

void FixtureManager::fixtureGroupSelected(FixtureGroup* grp)
{
    QByteArray state = m_splitter->saveState();

    if (m_info != NULL)
    {
        delete m_info;
        m_info = NULL;
    }

    if (m_groupEditor != NULL)
    {
        delete m_groupEditor;
        m_groupEditor = NULL;
    }

    m_groupEditor = new FixtureGroupEditor(grp, m_doc, this);
    m_splitter->addWidget(m_groupEditor);

    m_splitter->restoreState(state);
}

void FixtureManager::createInfo()
{
    QByteArray state = m_splitter->saveState();

    if (m_info != NULL)
    {
        delete m_info;
        m_info = NULL;
    }

    if (m_groupEditor != NULL)
    {
        delete m_groupEditor;
        m_groupEditor = NULL;
    }

    m_info = new QTextBrowser(this);
    m_splitter->addWidget(m_info);

    m_splitter->restoreState(state);
}

void FixtureManager::slotSelectionChanged()
{
    int selectedCount = m_fixtures_tree->selectedItems().size();
    if (selectedCount == 1)
    {
        QTreeWidgetItem* item = m_fixtures_tree->selectedItems().first();
        Q_ASSERT(item != NULL);

        // Set the text view's contents
        QVariant fxivar = item->data(KColumnName, PROP_FIXTURE);
        QVariant grpvar = item->data(KColumnName, PROP_GROUP);
        if (fxivar.isValid() == true)
        {
            // Selected a fixture
            fixtureSelected(fxivar.toUInt());
        }
        else if (grpvar.isValid() == true)
        {
            FixtureGroup* grp = m_doc->fixtureGroup(grpvar.toUInt());
            Q_ASSERT(grp != NULL);
            fixtureGroupSelected(grp);
        }
        else
        {
            QString info("<HTML><BODY><H1>%1</H1><P>%2</P></BODY></HTML>");
            if (m_info == NULL)
                createInfo();
            m_info->setText(info.arg(tr("All fixtures")).arg(tr("This group contains all fixtures.")));
        }
    }
    else
    {
        // More than one or less than one selected
        QString info;
        if (selectedCount > 1)
        {
            // Enable removal of multiple items in design mode
            if (m_doc->mode() == Doc::Design)
            {
                info = tr("<HTML><BODY><H1>Multiple fixtures selected</H1>" \
                          "<P>Click <IMG SRC=\"" ":/edit_remove.png\">" \
                          " to remove the selected fixtures.</P></BODY></HTML>");
            }
            else
            {
                info = tr("<HTML><BODY><H1>Multiple fixtures selected</H1>" \
                          "<P>Fixture list modification is not permitted" \
                          " in operate mode.</P></BODY></HTML>");
            }
        }
        else
        {
            if (m_fixtures_tree->topLevelItemCount() <= 0)
            {
                info = tr("<HTML><BODY><H1>No fixtures</H1>" \
                          "<P>Click <IMG SRC=\"" ":/edit_add.png\">" \
                          " to add fixtures.</P></BODY></HTML>");
            }
            else
            {
                info = tr("<HTML><BODY><H1>Nothing selected</H1>" \
                          "<P>Select a fixture from the list or " \
                          "click <IMG SRC=\"" ":/edit_add.png\">" \
                          " to add fixtures.</P></BODY></HTML>");
            }
        }

        if (m_info == NULL)
            createInfo();
        m_info->setText(info);
    }

    // Enable/disable actions
    slotModeChanged(m_doc->mode());
}

void FixtureManager::slotChannelsGroupSelectionChanged()
{
    if (m_info == NULL)
        createInfo();

    int selectedCount = m_channel_groups_tree->selectedItems().size();

    if (selectedCount == 1)
    {
        QTreeWidgetItem* item = m_channel_groups_tree->selectedItems().first();
        Q_ASSERT(item != NULL);

        // Set the text view's contents
        QVariant grpvar = item->data(KColumnName, PROP_FIXTURE);
        if (grpvar.isValid() == true)
        {
            ChannelsGroup *chGroup = m_doc->channelsGroup(grpvar.toUInt());
            if (chGroup != NULL)
                m_info->setText(QString("%1<BODY>%2</BODY></HTML>")
                                .arg(channelsGroupInfoStyleSheetHeader())
                                .arg(chGroup->status(m_doc)));
        }
        m_removeAction->setEnabled(true);
        m_propertiesAction->setEnabled(true);
        int selIdx = m_channel_groups_tree->currentIndex().row();
        if (selIdx == 0)
            m_moveUpAction->setEnabled(false);
        else
            m_moveUpAction->setEnabled(true);
        if (selIdx == m_channel_groups_tree->topLevelItemCount() - 1)
            m_moveDownAction->setEnabled(false);
        else
            m_moveDownAction->setEnabled(true);
    }
    else if (selectedCount > 1)
    {
        m_info->setText(tr("<HTML><BODY><H1>Multiple groups selected</H1>" \
                  "<P>Click <IMG SRC=\"" ":/edit_remove.png\">" \
                  " to remove the selected groups.</P></BODY></HTML>"));
        m_removeAction->setEnabled(true);
        m_propertiesAction->setEnabled(false);
    }
    else
    {
        m_info->setText(tr("<HTML><BODY><H1>Nothing selected</H1>" \
                  "<P>Select a channel group from the list or " \
                  "click <IMG SRC=\"" ":/edit_add.png\">" \
                  " to add a new channels group.</P></BODY></HTML>"));
        m_removeAction->setEnabled(false);
        m_propertiesAction->setEnabled(false);
    }
}

void FixtureManager::slotDoubleClicked(QTreeWidgetItem* item)
{
    if (item != NULL && m_doc->mode() != Doc::Operate)
        slotProperties();
}

void FixtureManager::slotChannelsGroupDoubleClicked(QTreeWidgetItem*)
{
    slotChannelsGroupSelectionChanged();
    editChannelGroupProperties();
}

void FixtureManager::slotTabChanged(int index)
{
    if (index == 1)
    {
        m_addAction->setToolTip(tr("Add group..."));
        updateChannelsGroupView();
        slotChannelsGroupSelectionChanged();
    }
    else
    {
        m_addAction->setToolTip(tr("Add fixture..."));
        updateView();
        slotSelectionChanged();
    }

    m_currentTabIndex = index;
}

void FixtureManager::slotFixtureItemExpanded()
{
    m_fixtures_tree->resizeColumnToContents(KColumnName);
    m_fixtures_tree->resizeColumnToContents(KColumnAddress);
    m_fixtures_tree->resizeColumnToContents(KColumnUniverse);
}

void FixtureManager::selectGroup(quint32 id)
{
    for (int i = 0; i < m_fixtures_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_fixtures_tree->topLevelItem(i);
        QVariant var = item->data(KColumnName, PROP_GROUP);
        if (var.isValid() == false)
            continue;

        if (var.toUInt() == id)
        {
            m_fixtures_tree->setCurrentItem(item);
            slotSelectionChanged();
            break;
        }
    }
}

QString FixtureManager::fixtureInfoStyleSheetHeader()
{
    QString info;

    QPalette pal;
    QColor hlBack(pal.color(QPalette::Highlight));
    QColor hlText(pal.color(QPalette::HighlightedText));

    info += "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">";
    info += "<HTML><HEAD></HEAD><STYLE>";
    info += QString(".hilite {" \
                    "	background-color: %1;" \
                    "	color: %2;" \
                    "	font-size: x-large;" \
                    "}").arg(hlBack.name()).arg(hlText.name());
    info += QString(".subhi {" \
                    "	background-color: %1;" \
                    "	color: %2;" \
                    "	font-weight: bold;" \
                    "}").arg(hlBack.name()).arg(hlText.name());
    info += QString(".emphasis {" \
                    "	font-weight: bold;" \
                    "}");
    info += QString(".tiny {"\
                    "   font-size: small;" \
                    "}");
    info += QString(".author {" \
                    "	font-weight: light;" \
                    "	font-style: italic;" \
                    "   text-align: right;" \
                    "   font-size: small;"  \
                    "}");
    info += "</STYLE>";
    return info;
}

QString FixtureManager::channelsGroupInfoStyleSheetHeader()
{
    QString info;

    QPalette pal;
    QColor hlBack(pal.color(QPalette::Highlight));
    QColor hlBackSmall(pal.color(QPalette::Shadow));
    QColor hlText(pal.color(QPalette::HighlightedText));

    info += "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">";
    info += "<HTML><HEAD></HEAD><STYLE>";
    info += QString(".hilite {" \
                    "	background-color: %1;" \
                    "	color: %2;" \
                    "	font-size: x-large;" \
                    "}").arg(hlBack.name()).arg(hlText.name());
    info += QString(".subhi {" \
                    "	background-color: %1;" \
                    "	color: %2;" \
                    "	font-weight: bold;" \
                    "}").arg(hlBackSmall.name()).arg(hlText.name());
    info += QString(".emphasis {" \
                    "	font-weight: bold;" \
                    "}");
    info += QString(".tiny {"\
                    "   font-size: small;" \
                    "}");
    info += "</STYLE>";
    return info;
}

/*****************************************************************************
 * Menu, toolbar and actions
 *****************************************************************************/

void FixtureManager::initActions()
{
    // Fixture actions
    m_addAction = new QAction(QIcon(":/edit_add.png"),
                              tr("Add fixture..."), this);
    connect(m_addAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAdd()));

    m_removeAction = new QAction(QIcon(":/edit_remove.png"),
                                 tr("Delete items"), this);
    connect(m_removeAction, SIGNAL(triggered(bool)),
            this, SLOT(slotRemove()));

    m_propertiesAction = new QAction(QIcon(":/configure.png"),
                                     tr("Properties..."), this);
    connect(m_propertiesAction, SIGNAL(triggered(bool)),
            this, SLOT(slotProperties()));

    m_fadeConfigAction = new QAction(QIcon(":/fade.png"),
                                     tr("Channels Fade Configuration..."), this);
    connect(m_fadeConfigAction, SIGNAL(triggered(bool)),
            this, SLOT(slotFadeConfig()));

    // Group actions
    m_groupAction = new QAction(QIcon(":/group.png"),
                                tr("Add fixture to group..."), this);

    m_unGroupAction = new QAction(QIcon(":/ungroup.png"),
                                tr("Remove fixture from group"), this);
    connect(m_unGroupAction, SIGNAL(triggered(bool)),
            this, SLOT(slotUnGroup()));

    m_newGroupAction = new QAction(tr("New Group..."), this);

    m_moveUpAction = new QAction(QIcon(":/up.png"),
                                 tr("Move group up..."), this);
    m_moveUpAction->setEnabled(false);
    connect(m_moveUpAction, SIGNAL(triggered(bool)),
            this, SLOT(slotMoveGroupUp()));

    m_moveDownAction = new QAction(QIcon(":/down.png"),
                                 tr("Move group down..."), this);
    m_moveDownAction->setEnabled(false);
    connect(m_moveDownAction, SIGNAL(triggered(bool)),
            this, SLOT(slotMoveGroupDown()));

    m_importAction = new QAction(QIcon(":/fileimport.png"),
                                 tr("Import fixtures..."), this);
    connect(m_importAction, SIGNAL(triggered(bool)),
            this, SLOT(slotImport()));

    m_exportAction = new QAction(QIcon(":/fileexport.png"),
                                 tr("Export fixtures..."), this);

    connect(m_exportAction, SIGNAL(triggered(bool)),
            this, SLOT(slotExport()));

    m_remapAction = new QAction(QIcon(":/remap.png"),
                               tr("Remap fixtures..."), this);
    connect(m_remapAction, SIGNAL(triggered(bool)),
            this, SLOT(slotRemap()));
}

void FixtureManager::updateGroupMenu()
{
    if (m_groupMenu == NULL)
    {
        m_groupMenu = new QMenu(this);
        connect(m_groupMenu, SIGNAL(triggered(QAction*)),
                this, SLOT(slotGroupSelected(QAction*)));
    }

    foreach (QAction* a, m_groupMenu->actions())
        m_groupMenu->removeAction(a);

    // Put all known fixture groups to the menu
    foreach (FixtureGroup* grp, m_doc->fixtureGroups())
    {
        QAction* a = m_groupMenu->addAction(grp->name());
        a->setData((qulonglong) grp);
    }

    // Put a new group action to the group menu
    m_groupMenu->addAction(m_newGroupAction);

    // Put the group menu to the group action
    m_groupAction->setMenu(m_groupMenu);
}

void FixtureManager::initToolBar()
{
    QToolBar* toolbar = new QToolBar(tr("Fixture manager"), this);
    toolbar->setFloatable(false);
    toolbar->setMovable(false);
    layout()->setMenuBar(toolbar);
    toolbar->addAction(m_addAction);
    toolbar->addAction(m_removeAction);
    toolbar->addAction(m_propertiesAction);
    toolbar->addAction(m_fadeConfigAction);
    toolbar->addSeparator();
    toolbar->addAction(m_groupAction);
    toolbar->addAction(m_unGroupAction);
    toolbar->addSeparator();
    toolbar->addAction(m_moveUpAction);
    toolbar->addAction(m_moveDownAction);
    toolbar->addSeparator();
    toolbar->addAction(m_importAction);
    toolbar->addAction(m_exportAction);
    toolbar->addAction(m_remapAction);

    QToolButton* btn = qobject_cast<QToolButton*> (toolbar->widgetForAction(m_groupAction));
    Q_ASSERT(btn != NULL);
    btn->setPopupMode(QToolButton::InstantPopup);
}

void FixtureManager::addFixture()
{
    AddFixture af(this, m_doc);
    if (af.exec() == QDialog::Rejected)
        return;

    quint32 latestFxi = Fixture::invalidId();

    QString name = af.name();
    quint32 address = af.address();
    quint32 universe = af.universe();
    quint32 channels = af.channels();
    int gap = af.gap();

    const QLCFixtureDef* fixtureDef = af.fixtureDef();
    const QLCFixtureMode* mode = af.mode();

    FixtureGroup* addToGroup = NULL;
    QTreeWidgetItem* current = m_fixtures_tree->currentItem();
    if (current != NULL)
    {
        if (current->parent() != NULL)
        {
            // Fixture selected
            QVariant var = current->parent()->data(KColumnName, PROP_GROUP);
            if (var.isValid() == true)
                addToGroup = m_doc->fixtureGroup(var.toUInt());
        }
        else
        {
            // Group selected
            QVariant var = current->data(KColumnName, PROP_GROUP);
            if (var.isValid() == true)
                addToGroup = m_doc->fixtureGroup(var.toUInt());
        }
    }

    /* If an empty name was given use the model instead */
    if (name.simplified().isEmpty())
    {
        if (fixtureDef != NULL)
            name = fixtureDef->model();
        else
            name = tr("Generic Dimmer");
    }

    /* Add the rest (if any) WITH address gap */
    for (int i = 0; i < af.amount(); i++)
    {
        QString modname;

        /* If we're adding more than one fixture,
           append a number to the end of the name */
        if (af.amount() > 1)
            modname = QString("%1 #%2").arg(name).arg(i + 1);
        else
            modname = name;

        /* Create the fixture */
        Fixture* fxi = new Fixture(m_doc);

        /* Assign the next address AFTER the previous fixture
           address space plus gap. */
        fxi->setAddress(address + (i * channels) + (i * gap));
        fxi->setUniverse(universe);
        fxi->setName(modname);
        /* Set a fixture definition & mode if they were
           selected. Otherwise assign channels to a generic
           dimmer. */
        if (fixtureDef != NULL && mode != NULL)
            fxi->setFixtureDefinition(fixtureDef, mode);
        else
            fxi->setChannels(channels);

        m_doc->addFixture(fxi);
        latestFxi = fxi->id();
        if (addToGroup != NULL)
            addToGroup->assignFixture(latestFxi);
    }

    QTreeWidgetItem* selectItem = fixtureItem(latestFxi);
    if (selectItem != NULL)
        m_fixtures_tree->setCurrentItem(selectItem);

    updateView();
}

void FixtureManager::addChannelsGroup()
{
    ChannelsGroup *group = new ChannelsGroup(m_doc);

    AddChannelsGroup cs(this, m_doc, group);
    if (cs.exec() == QDialog::Accepted)
    {
        qDebug() << "CHANNEL GROUP ADDED. Count: " << group->getChannels().count();
        m_doc->addChannelsGroup(group, group->id());
        updateChannelsGroupView();
    }
    else
        delete group;
}

void FixtureManager::slotAdd()
{
    if (m_currentTabIndex == 1)
        addChannelsGroup();
    else
        addFixture();
}

void FixtureManager::removeFixture()
{
    // Ask before deletion
    if (QMessageBox::question(this, tr("Delete Fixtures"),
                              tr("Do you want to delete the selected items?"),
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
    {
        return;
    }

    QListIterator <QTreeWidgetItem*> it(m_fixtures_tree->selectedItems());
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item(it.next());
        Q_ASSERT(item != NULL);

        QVariant var = item->data(KColumnName, PROP_FIXTURE);
        if (var.isValid() == true)
        {
            quint32 id = var.toUInt();

            /** @todo This is REALLY bogus here, since Fixture or Doc should do
                this. However, FixtureManager is the only place to destroy fixtures,
                so it's rather safe to reset the fixture's address space here. */
            Fixture* fxi = m_doc->fixture(id);
            Q_ASSERT(fxi != NULL);
            UniverseArray* ua = m_doc->outputMap()->claimUniverses();
            ua->reset(fxi->address(), fxi->channels());
            m_doc->outputMap()->releaseUniverses();

            m_doc->deleteFixture(id);
        }
        else
        {
            var = item->data(KColumnName, PROP_GROUP);
            if (var.isValid() == false)
                continue;

            quint32 id = var.toUInt();
            m_doc->deleteFixtureGroup(id);
        }
    }
}

void FixtureManager::removeChannelsGroup()
{
    // Ask before deletion
    if (QMessageBox::question(this, tr("Delete Channels Group"),
                              tr("Do you want to delete the selected groups?"),
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
    {
        return;
    }

    disconnect(m_channel_groups_tree, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotChannelsGroupSelectionChanged()));

    QListIterator <QTreeWidgetItem*> it(m_channel_groups_tree->selectedItems());
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item(it.next());
        Q_ASSERT(item != NULL);

        QVariant var = item->data(KColumnName, PROP_FIXTURE);
        if (var.isValid() == true)
            m_doc->deleteChannelsGroup(var.toUInt());
    }
    updateChannelsGroupView();

    connect(m_channel_groups_tree, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotChannelsGroupSelectionChanged()));
}

void FixtureManager::slotRemove()
{
    if (m_currentTabIndex == 1)
        removeChannelsGroup();
    else
        removeFixture();
}

void FixtureManager::editFixtureProperties()
{
    QTreeWidgetItem* item = m_fixtures_tree->currentItem();
    if (item == NULL)
        return;

    QVariant var = item->data(KColumnName, PROP_FIXTURE);
    if (var.isValid() == false)
        return;

    quint32 id = var.toUInt();
    Fixture* fxi = m_doc->fixture(id);
    if (fxi == NULL)
        return;

    QString manuf;
    QString model;
    QString mode;

    if (fxi->fixtureDef() != NULL)
    {
        manuf = fxi->fixtureDef()->manufacturer();
        model = fxi->fixtureDef()->model();
        mode = fxi->fixtureMode()->name();
    }
    else
    {
        manuf = KXMLFixtureGeneric;
        model = KXMLFixtureGeneric;
    }

    AddFixture af(this, m_doc, fxi);
    af.setWindowTitle(tr("Change fixture properties"));
    if (af.exec() == QDialog::Accepted)
    {
      if (af.invalidAddress() == false)
      {
        if (fxi->name() != af.name())
            fxi->setName(af.name());
        if (fxi->universe() != af.universe())
            fxi->setUniverse(af.universe());
        if (fxi->address() != af.address())
        {
            m_doc->moveFixture(id, af.address());
            fxi->setAddress(af.address());
        }

        if (af.fixtureDef() != NULL && af.mode() != NULL)
        {
            if (fxi->fixtureDef() != af.fixtureDef() ||
                    fxi->fixtureMode() != af.mode())
            {
                m_doc->changeFixtureMode(id, af.mode());
                fxi->setFixtureDefinition(af.fixtureDef(),
                                          af.mode());
            }
        }
        else
        {
            /* Generic dimmer */
            fxi->setFixtureDefinition(NULL, NULL);
            fxi->setChannels(af.channels());
        }

        updateFixtureItem(item, fxi);
        slotSelectionChanged();
      }
      else
      {
          QMessageBox msg(QMessageBox::Critical, tr("Error"),
                          tr("Please enter a valid address"), QMessageBox::Ok);
          msg.exec();
      }
    }
}

void FixtureManager::editChannelGroupProperties()
{
    int selectedCount = m_channel_groups_tree->selectedItems().size();

    if (selectedCount > 0)
    {
        QTreeWidgetItem* current = m_channel_groups_tree->selectedItems().first();
        QVariant var = current->data(KColumnName, PROP_FIXTURE);
        if (var.isValid() == true)
        {
            ChannelsGroup *group = m_doc->channelsGroup(var.toUInt());

            AddChannelsGroup cs(this, m_doc, group);
            if (cs.exec() == QDialog::Accepted)
            {
                qDebug() << "CHANNEL GROUP MODIFIED. Count: " << group->getChannels().count();
                m_doc->addChannelsGroup(group, group->id());
                updateChannelsGroupView();
            }
        }
    }
}

int FixtureManager::headCount(const QList <QTreeWidgetItem*>& items) const
{
    int count = 0;
    QListIterator <QTreeWidgetItem*> it(items);
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item = it.next();
        Q_ASSERT(item != NULL);

        QVariant var = item->data(KColumnName, PROP_FIXTURE);
        if (var.isValid() == false)
            continue;

        Fixture* fxi = m_doc->fixture(var.toUInt());
        count += fxi->heads();
    }

    return count;
}

void FixtureManager::slotProperties()
{
    if (m_currentTabIndex == 1)
        editChannelGroupProperties();
    else
        editFixtureProperties();
}

void FixtureManager::slotFadeConfig()
{
    ChannelsSelection cfg(m_doc, this, ChannelsSelection::ExcludeChannelsMode);
    if (cfg.exec() == QDialog::Rejected)
        return; // User pressed cancel
    m_doc->setModified();
}

void FixtureManager::slotRemap()
{
    FixtureRemap fxr(m_doc);
    if (fxr.exec() == QDialog::Rejected)
        return; // User pressed cancel

    updateView();
}

void FixtureManager::slotUnGroup()
{
    if (QMessageBox::question(this, tr("Ungroup fixtures?"),
                              tr("Do you want to ungroup the selected fixtures?"),
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
    {
        return;
    }

    // Because FixtureGroup::resignFixture() emits changed(), which makes the tree
    // update its contents in the middle, invalidating m_tree->selectedItems(),
    // we must pick the list of fixtures and groups first and then resign them in
    // one big bunch.
    QList <QPair<quint32,quint32> > resignList;

    foreach (QTreeWidgetItem* item, m_fixtures_tree->selectedItems())
    {
        if (item->parent() == NULL)
            continue;

        QVariant var = item->parent()->data(KColumnName, PROP_GROUP);
        if (var.isValid() == false)
            continue;
        quint32 grp = var.toUInt();

        var = item->data(KColumnName, PROP_FIXTURE);
        if (var.isValid() == false)
            continue;
        quint32 fxi = var.toUInt();

        resignList << QPair <quint32,quint32> (grp, fxi);
    }

    QListIterator <QPair<quint32,quint32> > it(resignList);
    while (it.hasNext() == true)
    {
        QPair <quint32,quint32> pair(it.next());
        FixtureGroup* grp = m_doc->fixtureGroup(pair.first);
        Q_ASSERT(grp != NULL);
        grp->resignFixture(pair.second);
    }
}

void FixtureManager::slotGroupSelected(QAction* action)
{
    FixtureGroup* grp = NULL;

    if (action->data().isValid() == true)
    {
        // Existing group selected
        grp = (FixtureGroup*) (action->data().toULongLong());
        Q_ASSERT(grp != NULL);
    }
    else
    {
        // New Group selected.

        // Suggest an equilateral grid
        qreal side = sqrt(headCount(m_fixtures_tree->selectedItems()));
        if (side != floor(side))
            side += 1; // Fixture number doesn't provide a full square

        CreateFixtureGroup cfg(this);
        cfg.setSize(QSize(side, side));
        if (cfg.exec() != QDialog::Accepted)
            return; // User pressed cancel

        grp = new FixtureGroup(m_doc);
        Q_ASSERT(grp != NULL);
        grp->setName(cfg.name());
        grp->setSize(cfg.size());
        m_doc->addFixtureGroup(grp);
        updateGroupMenu();
    }

    // Assign selected fixture items to the group
    foreach (QTreeWidgetItem* item, m_fixtures_tree->selectedItems())
    {
        QVariant var = item->data(KColumnName, PROP_FIXTURE);
        if (var.isValid() == false)
            continue;

        grp->assignFixture(var.toUInt());
    }

    updateView();
}

void FixtureManager::slotMoveGroupUp()
{
    if (m_channel_groups_tree->selectedItems().size() > 0)
    {
        QTreeWidgetItem* item = m_channel_groups_tree->selectedItems().first();
        quint32 grpID = item->data(KColumnName, PROP_FIXTURE).toUInt();
        m_doc->moveChannelGroup(grpID, -1);
        updateChannelsGroupView();
    }
}

void FixtureManager::slotMoveGroupDown()
{
    if (m_channel_groups_tree->selectedItems().size() > 0)
    {
        QTreeWidgetItem* item = m_channel_groups_tree->selectedItems().first();
        quint32 grpID = item->data(KColumnName, PROP_FIXTURE).toUInt();
        m_doc->moveChannelGroup(grpID, 1);
        updateChannelsGroupView();
    }
}

QString FixtureManager::createDialog(bool import)
{
    QString fileName;

    /* Create a file save dialog */
    QFileDialog dialog(this);
    if (import == true)
    {
        dialog.setWindowTitle(tr("Import Fixtures List"));
        dialog.setAcceptMode(QFileDialog::AcceptOpen);
    }
    else
    {
        dialog.setWindowTitle(tr("Export Fixtures List As"));
        dialog.setAcceptMode(QFileDialog::AcceptSave);
    }

    /* Append file filters to the dialog */
    QStringList filters;
    filters << tr("Fixtures List (*%1)").arg(KExtFixtureList);
#if defined(WIN32) || defined(Q_OS_WIN)
    filters << tr("All Files (*.*)");
#else
    filters << tr("All Files (*)");
#endif
    dialog.setNameFilters(filters);

    /* Append useful URLs to the dialog */
    QList <QUrl> sidebar;
    sidebar.append(QUrl::fromLocalFile(QDir::homePath()));
    sidebar.append(QUrl::fromLocalFile(QDir::rootPath()));
    dialog.setSidebarUrls(sidebar);

    /* Get file name */
    if (dialog.exec() != QDialog::Accepted)
        return "";

    fileName = dialog.selectedFiles().first();
    if (fileName.isEmpty() == true)
        return "";

    /* Always use the fixture definition suffix */
    if (import == false && fileName.right(5) != KExtFixtureList)
        fileName += KExtFixtureList;

    return fileName;
}

void FixtureManager::slotImport()
{
    QString fileName = createDialog(true);

    QDomDocument doc(QLCFile::readXML(fileName));
    if (doc.isNull() == false)
    {
        if (doc.doctype().name() == KXMLQLCFixturesList)
        {
            QDomElement root = doc.documentElement();
            if (root.tagName() != KXMLQLCFixturesList)
            {
                qWarning() << Q_FUNC_INFO << "Fixture Definition node not found";
                return;
            }
            QDomNode node = root.firstChild();
            while (node.isNull() == false)
            {
                QDomElement tag = node.toElement();

                if (tag.tagName() == KXMLFixture)
                {
                    Fixture* fxi = new Fixture(m_doc);
                    Q_ASSERT(fxi != NULL);

                    if (fxi->loadXML(tag, m_doc->fixtureDefCache()) == true)
                    {
                        if (m_doc->addFixture(fxi /*, fxi->id()*/) == true)
                        {
                            /* Success */
                            qWarning() << Q_FUNC_INFO << "Fixture" << fxi->name() << "successfully created.";
                        }
                        else
                        {
                            /* Doc is full */
                            qWarning() << Q_FUNC_INFO << "Fixture" << fxi->name() << "cannot be created.";
                            delete fxi;
                        }
                    }
                    else
                    {
                        qWarning() << Q_FUNC_INFO << "Fixture" << fxi->name() << "cannot be loaded.";
                        delete fxi;
                    }
                }
                else if (tag.tagName() == KXMLQLCFixtureGroup)
                {
                    FixtureGroup* grp = new FixtureGroup(m_doc);
                    Q_ASSERT(grp != NULL);

                    if (grp->loadXML(tag) == true)
                    {
                        m_doc->addFixtureGroup(grp, grp->id());
                    }
                    else
                    {
                        qWarning() << Q_FUNC_INFO << "FixtureGroup" << grp->name() << "cannot be loaded.";
                        delete grp;
                    }
                }

                node = node.nextSibling();
            }
            updateView();
        }
    }
}

void FixtureManager::slotExport()
{
    QString fileName = createDialog(false);

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly) == false)
        return;

    QDomDocument doc(QLCFile::getXMLHeader(KXMLQLCFixturesList));

    if (doc.isNull() == false)
    {
        QDomElement root;
        QDomElement tag;
        QDomText text;

        /* Create a text stream for the file */
        QTextStream stream(&file);

        /* THE MASTER XML ROOT NODE */
        root = doc.documentElement();

        QListIterator <Fixture*> fxit(m_doc->fixtures());
        while (fxit.hasNext() == true)
        {
            Fixture* fxi(fxit.next());
            Q_ASSERT(fxi != NULL);
            fxi->saveXML(&doc, &root);
        }

        QListIterator <FixtureGroup*>grpit(m_doc->fixtureGroups());
        while (grpit.hasNext() == true)
        {
            FixtureGroup *fxgrp(grpit.next());
            Q_ASSERT(fxgrp != NULL);
            fxgrp->saveXML(&doc, &root);
        }

        /* Write the XML document to the stream (=file) */
        stream << doc.toString() << "\n";
    }

    file.close();
}

void FixtureManager::slotContextMenuRequested(const QPoint&)
{
    QMenu menu(this);
    menu.addAction(m_addAction);
    menu.addAction(m_propertiesAction);
    menu.addAction(m_removeAction);
    menu.addSeparator();
    menu.addAction(m_groupAction);
    menu.addAction(m_unGroupAction);
    menu.exec(QCursor::pos());
}
