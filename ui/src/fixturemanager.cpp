/*
  Q Light Controller
  fixturemanager.cpp

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
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QScrollArea>
#include <QMessageBox>
#include <QToolButton>
#include <QTabWidget>
#include <QSplitter>
#include <QToolBar>
#include <QAction>
#include <QString>
#include <QDebug>
#include <QIcon>
#include <QMenu>
#include <QtXml>

#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlccapability.h"
#include "qlcchannel.h"
#include "qlcfile.h"

#include "createfixturegroup.h"
#include "fixturegroupeditor.h"
#include "channelselection.h"
#include "fixturemanager.h"
#include "universearray.h"
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

FixtureManager* FixtureManager::s_instance = NULL;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

FixtureManager::FixtureManager(QWidget* parent, Doc* doc)
    : QWidget(parent)
    , m_doc(doc)
    , m_splitter(NULL)
    , m_tree(NULL)
    , m_channels_tree(NULL)
    , m_info(NULL)
    , m_groupEditor(NULL)
    , m_currentTabIndex(0)
    , m_addAction(NULL)
    , m_removeAction(NULL)
    , m_propertiesAction(NULL)
    , m_groupAction(NULL)
    , m_unGroupAction(NULL)
    , m_newGroupAction(NULL)
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

    /* Connect fixture list change signals from the new document object */
    connect(m_doc, SIGNAL(fixtureRemoved(quint32)),
            this, SLOT(slotFixtureRemoved(quint32)));

    connect(m_doc, SIGNAL(modeChanged(Doc::Mode)),
            this, SLOT(slotModeChanged(Doc::Mode)));

    connect(m_doc, SIGNAL(fixtureGroupRemoved(quint32)),
            this, SLOT(slotFixtureGroupRemoved(quint32)));

    connect(m_doc, SIGNAL(fixtureGroupChanged(quint32)),
            this, SLOT(slotFixtureGroupChanged(quint32)));

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
    for (int i = 0; i < m_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* grpItem = m_tree->topLevelItem(i);
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

void FixtureManager::slotModeChanged(Doc::Mode mode)
{
    if (mode == Doc::Design)
    {
        int selected = m_tree->selectedItems().size();

        QTreeWidgetItem* item = m_tree->currentItem();
        if (item == NULL)
        {
            m_addAction->setEnabled(true);
            m_removeAction->setEnabled(false);
            m_propertiesAction->setEnabled(false);
            m_groupAction->setEnabled(false);
            m_unGroupAction->setEnabled(false);
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
            // Group selected
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
    }
    else
    {
        m_addAction->setEnabled(false);
        m_removeAction->setEnabled(false);
        m_propertiesAction->setEnabled(false);
        m_groupAction->setEnabled(false);
        m_unGroupAction->setEnabled(false);
    }
}

void FixtureManager::slotFixtureGroupRemoved(quint32 id)
{
    for (int i = 0; i < m_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_tree->topLevelItem(i);
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
    m_tree = new QTreeWidget(this);

    QStringList labels;
    labels << tr("Name") << tr("Universe") << tr("Address");
    m_tree->setHeaderLabels(labels);
    m_tree->setRootIsDecorated(true);
    m_tree->setIconSize(QSize(32, 32));
    m_tree->setSortingEnabled(true);
    m_tree->setAllColumnsShowFocus(true);
    m_tree->sortByColumn(KColumnAddress, Qt::AscendingOrder);
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);
    QFont m_font = QApplication::font();
    m_font.setPixelSize(13);
    m_tree->setFont(m_font);

    connect(m_tree, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotDoubleClicked(QTreeWidgetItem*)));

    connect(m_tree, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(slotContextMenuRequested(const QPoint&)));


    tabs->addTab(m_tree, tr("Fixtures Groups"));

    m_channels_tree = new QTreeWidget(this);
    QStringList chan_labels;
    chan_labels << tr("Name") << tr("Channels");
    m_channels_tree->setHeaderLabels(chan_labels);
    m_channels_tree->setRootIsDecorated(true);
    m_channels_tree->setIconSize(QSize(32, 32));
    m_channels_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_channels_tree->header()->setResizeMode(QHeaderView::ResizeToContents);
    m_channels_tree->setFont(m_font);

    connect(m_channels_tree, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotChannelsGroupSelectionChanged()));
    connect(m_channels_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotChannelsGroupDoubleClicked(QTreeWidgetItem*)));

    tabs->addTab(m_channels_tree, tr("Channels Groups"));

    connect(tabs, SIGNAL(currentChanged(int)), this, SLOT(slotTabChanged(int)));

    /* Create the text view */
    createInfo();

    slotSelectionChanged();
}

void FixtureManager::updateView()
{
    // Record which groups are open
    QList <QVariant> openGroups;
    for (int i = 0; i < m_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_tree->topLevelItem(i);
        if (item->isExpanded() == true)
            openGroups << item->data(KColumnName, PROP_GROUP);
    }

    // Clear the view
    m_tree->clear();

    // Insert known fixture groups and their children
    foreach (FixtureGroup* grp, m_doc->fixtureGroups())
    {
        QTreeWidgetItem* grpItem = new QTreeWidgetItem(m_tree);
        grpItem->setIcon(KColumnName, QIcon(":/group.png"));
        updateGroupItem(grpItem, grp);
    }

    // Insert the "All fixtures group"
    QTreeWidgetItem* grpItem = new QTreeWidgetItem(m_tree);
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
    for (int i = 0; i < m_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_tree->topLevelItem(i);
        QVariant var = item->data(KColumnName, PROP_GROUP);
        if (openGroups.contains(var) == true)
        {
            item->setExpanded(true);
            openGroups.removeAll(var);
        }
    }

    updateGroupMenu();
    slotModeChanged(m_doc->mode());
}

QIcon FixtureManager::getIntensityIcon(const QLCChannel* channel)
{
    QPixmap pm(32, 32);
    if (channel->colour() == QLCChannel::Red ||
        channel->name().contains("red", Qt::CaseInsensitive) == true)
    {
        QColor color(255, 0, 0);
        pm.fill(color);
        return QIcon(pm);
    }
    else if (channel->colour() == QLCChannel::Green ||
             channel->name().contains("green", Qt::CaseInsensitive) == true)
    {
        QColor color(0, 255, 0);
        pm.fill(color);
        return QIcon(pm);
    }
    else if (channel->colour() == QLCChannel::Blue ||
             channel->name().contains("blue", Qt::CaseInsensitive) == true)
    {
        QColor color(0, 0, 255);
        pm.fill(color);
        return QIcon(pm);
    }
    else if (channel->colour() == QLCChannel::Cyan ||
             channel->name().contains("cyan", Qt::CaseInsensitive) == true)
    {
        QColor color(0, 255, 255);
        pm.fill(color);
        return QIcon(pm);
    }
    else if (channel->colour() == QLCChannel::Magenta ||
             channel->name().contains("magenta", Qt::CaseInsensitive) == true)
    {
        QColor color(255, 0, 255);
        pm.fill(color);
        return QIcon(pm);
    }
    else if (channel->colour() == QLCChannel::Yellow ||
             channel->name().contains("yellow", Qt::CaseInsensitive) == true)
    {
        QColor color(255, 255, 0);
        pm.fill(color);
        return QIcon(pm);
    }
    else
    {
        // None of the primary colours matched and since this is an
        // intensity channel, it must be controlling a plain dimmer OSLT.
        return QIcon(":/intensity.png");
    }
}

void FixtureManager::updateChannelsGroupView()
{
    m_channels_tree->clear();
    foreach (ChannelsGroup* grp, m_doc->channelsGroups())
    {
        QTreeWidgetItem* grpItem = new QTreeWidgetItem(m_channels_tree);
        grpItem->setText(KColumnName, grp->name());
        grpItem->setData(KColumnName, PROP_FIXTURE, grp->id());
        grpItem->setText(KColumnChannels, QString("%1").arg(grp->getChannels().count()));
        if (grp->getChannels().count() > 0)
        {
            SceneValue scv = grp->getChannels().at(0);
            Fixture *fxi = m_doc->fixture(scv.fxi);
            const QLCChannel* ch = fxi->channel(scv.channel);
            switch(ch->group())
            {
            case QLCChannel::Pan: grpItem->setIcon(KColumnName, QIcon(":/pan.png")); break;
            case QLCChannel::Tilt: grpItem->setIcon(KColumnName, QIcon(":/tilt.png")); break;
            case QLCChannel::Colour: grpItem->setIcon(KColumnName, QIcon(":/color.png")); break;
            case QLCChannel::Effect: grpItem->setIcon(KColumnName, QIcon(":/star.png")); break;
            case QLCChannel::Gobo: grpItem->setIcon(KColumnName, QIcon(":/gobo.png")); break;
            case QLCChannel::Shutter: grpItem->setIcon(KColumnName, QIcon(":/shutter.png")); break;
            case QLCChannel::Speed: grpItem->setIcon(KColumnName, QIcon(":/speed.png")); break;
            case QLCChannel::Prism: grpItem->setIcon(KColumnName, QIcon(":/prism.png")); break;
            case QLCChannel::Maintenance: grpItem->setIcon(KColumnName, QIcon(":/maintenance.png")); break;
            case QLCChannel::Intensity: grpItem->setIcon(KColumnName, getIntensityIcon(ch)); break;
            case QLCChannel::Beam: grpItem->setIcon(KColumnName, QIcon(":/beam.png")); break;
            default:
            break;
            }
        }
    }
}

QTreeWidgetItem* FixtureManager::fixtureItem(quint32 id) const
{
    QTreeWidgetItemIterator it(m_tree);
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
    QTreeWidgetItemIterator it(m_tree);
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
    Q_ASSERT(fxi != NULL);

    // Universe column
    item->setText(KColumnUniverse, QString("%1").arg(fxi->universe() + 1));

    // Address column
    s.sprintf("%.3d - %.3d", fxi->address() + 1, fxi->address() + fxi->channels());

    item->setText(KColumnAddress, s);

    // Name column
    item->setText(KColumnName, fxi->name());

    // ID column
    item->setData(KColumnName, PROP_FIXTURE, fxi->id());

    QString ftype = fxi->type();
    if (ftype == "Color Changer")
        item->setIcon(KColumnName, QIcon(":/fixture.png"));
    else if (ftype == "Dimmer")
        item->setIcon(KColumnName, QIcon(":/dimmer.png"));
    else if (ftype == "Effect")
        item->setIcon(KColumnName, QIcon(":/effect.png"));
    else if (ftype == "Fan")
        item->setIcon(KColumnName, QIcon(":/fan.png"));
    else if (ftype == "Flower")
        item->setIcon(KColumnName, QIcon(":/flower.png"));
    else if (ftype == "Hazer")
        item->setIcon(KColumnName, QIcon(":/hazer.png"));
    else if (ftype == "Laser")
        item->setIcon(KColumnName, QIcon(":/laser.png"));
    else if (ftype == "Moving Head")
        item->setIcon(KColumnName, QIcon(":/movinghead.png"));
    else if (ftype == "Scanner")
        item->setIcon(KColumnName, QIcon(":/scanner.png"));
    else if (ftype == "Smoke")
        item->setIcon(KColumnName, QIcon(":/smoke.png"));
    else if (ftype == "Strobe")
        item->setIcon(KColumnName, QIcon(":/strobe.png"));
    else if (ftype == "Other")
        item->setIcon(KColumnName, QIcon(":/other.png"));
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
    int selectedCount = m_tree->selectedItems().size();
    if (selectedCount == 1)
    {
        QTreeWidgetItem* item = m_tree->selectedItems().first();
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
            if (m_tree->topLevelItemCount() <= 0)
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

    int selectedCount = m_channels_tree->selectedItems().size();

    if (selectedCount == 1)
    {
        QTreeWidgetItem* item = m_channels_tree->selectedItems().first();
        Q_ASSERT(item != NULL);

        // Set the text view's contents
        QVariant grpvar = item->data(KColumnName, PROP_FIXTURE);
        if (grpvar.isValid() == true)
        {
            ChannelsGroup *chGroup = m_doc->channelsGroup(grpvar.toUInt());
            m_info->setText(QString("%1<BODY>%2</BODY></HTML>")
                            .arg(channelsGroupInfoStyleSheetHeader())
                            .arg(chGroup->status(m_doc)));
        }
        m_removeAction->setEnabled(true);
        m_addAction->setToolTip(tr("Edit group..."));
    }
    else
    {
        QString info = tr("<HTML><BODY><H1>Nothing selected</H1>" \
                  "<P>Select a channel group from the list or " \
                  "click <IMG SRC=\"" ":/edit_add.png\">" \
                  " to add a new channels group.</P></BODY></HTML>");
        m_info->setText(info);
        m_removeAction->setEnabled(false);
        m_addAction->setToolTip(tr("Add group..."));
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
    addChannelsGroup();
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

void FixtureManager::selectGroup(quint32 id)
{
    for (int i = 0; i < m_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_tree->topLevelItem(i);
        QVariant var = item->data(KColumnName, PROP_GROUP);
        if (var.isValid() == false)
            continue;

        if (var.toUInt() == id)
        {
            m_tree->setCurrentItem(item);
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

    // Group actions
    m_groupAction = new QAction(QIcon(":/group.png"),
                                tr("Add fixture to group..."), this);

    m_unGroupAction = new QAction(QIcon(":/ungroup.png"),
                                tr("Remove fixture from group"), this);
    connect(m_unGroupAction, SIGNAL(triggered(bool)),
            this, SLOT(slotUnGroup()));

    m_newGroupAction = new QAction(tr("New Group..."), this);
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
    toolbar->addSeparator();
    toolbar->addAction(m_groupAction);
    toolbar->addAction(m_unGroupAction);

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
    QTreeWidgetItem* current = m_tree->currentItem();
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

    QString modname;

    /* If an empty name was given use the model instead */
    if (name.simplified().isEmpty())
    {
        if (fixtureDef != NULL)
            name = fixtureDef->model();
        else
            name = tr("Generic Dimmer");
    }

    /* If we're adding more than one fixture,
       append a number to the end of the name */
    if (af.amount() > 1)
        modname = QString("%1 #1").arg(name);
    else
        modname = name;

    /* Create the fixture */
    Fixture* fxi = new Fixture(m_doc);

    /* Add the first fixture without gap, at the given address */
    fxi->setAddress(address);
    fxi->setUniverse(universe);
    fxi->setName(modname);

    /* Set a fixture definition & mode if they were selected.
       Otherwise assign channels to a generic dimmer. */
    if (fixtureDef != NULL && mode != NULL)
        fxi->setFixtureDefinition(fixtureDef, mode);
    else
        fxi->setChannels(channels);

    m_doc->addFixture(fxi);
    latestFxi = fxi->id();
    if (addToGroup != NULL)
        addToGroup->assignFixture(latestFxi);

    /* Add the rest (if any) WITH address gap */
    for (int i = 1; i < af.amount(); i++)
    {
        /* If we're adding more than one fixture,
           append a number to the end of the name */
        if (af.amount() > 1)
            modname = QString("%1 #%2").arg(name).arg(i +1);
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
        m_tree->setCurrentItem(selectItem);

    updateView();
}

void FixtureManager::addChannelsGroup()
{
    ChannelsGroup *group = NULL;

    int selectedCount = m_channels_tree->selectedItems().size();

    if (selectedCount > 0)
    {
        QTreeWidgetItem* current = m_channels_tree->selectedItems().first();
        QVariant var = current->data(KColumnName, PROP_FIXTURE);
        if (var.isValid() == true)
            group = m_doc->channelsGroup(var.toUInt());
    }
    else
        group = new ChannelsGroup(m_doc);

    ChannelSelection cs(this, m_doc, group);
    if (cs.exec() == QDialog::Accepted)
    {
        qDebug() << "CHANNEL GROUP ADDED. Count: " << group->getChannels().count();
        m_doc->addChannelsGroup(group, group->id());
        updateChannelsGroupView();
    }

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

    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
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
                              tr("Do you want to delete the selected group?"),
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
    {
        return;
    }
    if (m_channels_tree->selectedItems().count() > 0)
    {
        QVariant var = m_channels_tree->currentItem()->data(KColumnName, PROP_FIXTURE);
        if (var.isValid() == true)
        {
            m_doc->deleteChannelsGroup(var.toUInt());
            updateChannelsGroupView();
        }
    }
}

void FixtureManager::slotRemove()
{
    if (m_currentTabIndex == 1)
        removeChannelsGroup();
    else
        removeFixture();
}

void FixtureManager::editFixtureProperties(QTreeWidgetItem* item)
{
    Q_ASSERT(item != NULL);

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
        if (fxi->name() != af.name())
            fxi->setName(af.name());
        if (fxi->universe() != af.universe())
            fxi->setUniverse(af.universe());
        if (fxi->address() != af.address())
            fxi->setAddress(af.address());

        if (af.fixtureDef() != NULL && af.mode() != NULL)
        {
            if (fxi->fixtureDef() != af.fixtureDef() ||
                    fxi->fixtureMode() != af.mode())
            {
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
    QTreeWidgetItem* item = m_tree->currentItem();
    if (item == NULL)
        return;

    QVariant var = item->data(KColumnName, PROP_FIXTURE);
    if (var.isValid() == true)
        editFixtureProperties(item);
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

    foreach (QTreeWidgetItem* item, m_tree->selectedItems())
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
        qreal side = sqrt(headCount(m_tree->selectedItems()));
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
    foreach (QTreeWidgetItem* item, m_tree->selectedItems())
    {
        QVariant var = item->data(KColumnName, PROP_FIXTURE);
        if (var.isValid() == false)
            continue;

        grp->assignFixture(var.toUInt());
    }

    updateView();
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
