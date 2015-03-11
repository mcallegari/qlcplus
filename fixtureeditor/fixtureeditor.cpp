/*
  Q Light Controller - Fixture Definition Editor
  qlcfixtureeditor.cpp

  Copyright (C) Heikki Junnila

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
#include <QInputDialog>
#include <QHeaderView>
#include <QTreeWidget>
#include <QMessageBox>
#include <QToolButton>
#include <QFileDialog>
#include <QCloseEvent>
#include <QSettings>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QTabWidget>
#include <QIcon>
#include <QMenu>
#include <QList>
#include <QUrl>

#include "qlcfixturedefcache.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlccapability.h"
#include "qlcphysical.h"
#include "qlcchannel.h"
#include "qlcconfig.h"
#include "qlcfile.h"

#ifdef Q_WS_X11
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#endif

#include "editcapability.h"
#include "fixtureeditor.h"
#include "editchannel.h"
#include "editmode.h"
#include "util.h"
#include "app.h"

extern App* _app;
extern int errno;

#define PROP_PTR Qt::UserRole

#define CH_COL_NAME 0
#define CH_COL_GRP  1

#define MODE_COL_NAME 0
#define MODE_COL_CHS  1
#define MODE_COL_HEAD 2

#define SETTINGS_GEOMETRY "fixtureeditor/geometry"

QLCFixtureEditor::QLCFixtureEditor(QWidget* parent, QLCFixtureDef* fixtureDef,
                                   const QString& fileName)
    : QWidget(parent)
    , m_fixtureDef(fixtureDef)
    , m_fileName(fileName)
    , m_modified(false)
{
    setupUi(this);
    init();
    setCaption();

    setModified(false);

    /* Connect to be able to enable/disable clipboard actions */
    connect(_app, SIGNAL(clipboardChanged()),
            this, SLOT(slotClipboardChanged()));

    /* Initial update to clipboard actions */
    slotClipboardChanged();

    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        parentWidget()->restoreGeometry(var.toByteArray());
}

QLCFixtureEditor::~QLCFixtureEditor()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, parentWidget()->saveGeometry());

    delete m_fixtureDef;
    m_fixtureDef = NULL;
}

void QLCFixtureEditor::init()
{
    /* General page */
    m_manufacturerEdit->setText(m_fixtureDef->manufacturer());
    m_manufacturerEdit->setValidator(CAPS_VALIDATOR(this));
    connect(m_manufacturerEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotManufacturerTextEdited(const QString&)));

    m_modelEdit->setText(m_fixtureDef->model());
    m_modelEdit->setValidator(CAPS_VALIDATOR(this));
    connect(m_modelEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotModelTextEdited(const QString&)));

    m_typeCombo->setCurrentIndex(m_typeCombo->findText(m_fixtureDef->type()));
    connect(m_typeCombo, SIGNAL(activated(const QString&)),
            this, SLOT(slotTypeActivated(const QString&)));

    // Display author name or suggest current user name if there isn't one.
    // When the def already has an author, disable the field to prevent modification.
    m_authorEdit->setText(m_fixtureDef->author());
    if (m_authorEdit->text().length() > 0)
    {
        // Temporarily allow editing author name since most definitions contain wrong name:
        // m_authorEdit->setEnabled(false); 
    }
    else
    {
        m_authorEdit->setText(QLCFile::currentUserName());
    }
    connect(m_authorEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotAuthorTextEdited(const QString&)));

    /* Channel page */
    connect(m_addChannelButton, SIGNAL(clicked()),
            this, SLOT(slotAddChannel()));
    connect(m_removeChannelButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveChannel()));
    connect(m_editChannelButton, SIGNAL(clicked()),
            this, SLOT(slotEditChannel()));
    connect(m_copyChannelButton, SIGNAL(clicked()),
            this, SLOT(slotCopyChannel()));
    connect(m_pasteChannelButton, SIGNAL(clicked()),
            this, SLOT(slotPasteChannel()));
    connect(m_expandChannelsButton, SIGNAL(clicked()),
            this, SLOT(slotExpandChannels()));

    connect(m_channelList, SIGNAL(currentItemChanged(QTreeWidgetItem*,
                                  QTreeWidgetItem*)),
            this, SLOT(slotChannelListSelectionChanged(QTreeWidgetItem*)));
    connect(m_channelList, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(slotChannelListContextMenuRequested()));
    connect(m_channelList, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(slotEditChannel()));
    connect(m_channelList, SIGNAL(expanded(QModelIndex)),
            this, SLOT(slotChannelItemExpanded()));

    m_channelList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_channelList->setIconSize(QSize(24, 24));
    refreshChannelList();

    /* Mode page */
    connect(m_addModeButton, SIGNAL(clicked()),
            this, SLOT(slotAddMode()));
    connect(m_removeModeButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveMode()));
    connect(m_editModeButton, SIGNAL(clicked()),
            this, SLOT(slotEditMode()));
    connect(m_cloneModeButton, SIGNAL(clicked()),
            this, SLOT(slotCloneMode()));
    connect(m_expandModesButton, SIGNAL(clicked()),
            this, SLOT(slotExpandModes()));

    connect(m_modeList, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
            this, SLOT(slotModeListSelectionChanged(QTreeWidgetItem*)));
    connect(m_modeList, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(slotModeListContextMenuRequested()));
    connect(m_modeList, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(slotEditMode()));
    connect(m_modeList, SIGNAL(expanded(QModelIndex)),
            this, SLOT(slotModeItemExpanded()));

    m_modeList->setContextMenuPolicy(Qt::CustomContextMenu);
    refreshModeList();
}

void QLCFixtureEditor::closeEvent(QCloseEvent* e)
{
    int r = 0;

    if (m_modified)
    {
        r = QMessageBox::information(this, tr("Close"),
                                     tr("Do you want to save changes to fixture\n\""
                                        "%1\"\nbefore closing?").arg(m_fixtureDef->name()),
                                     QMessageBox::Yes,
                                     QMessageBox::No,
                                     QMessageBox::Cancel);

        if (r == QMessageBox::Yes)
            if (save())
                e->accept();
            else
                e->ignore();
        else if (r == QMessageBox::No)
            e->accept();
        else
            e->ignore();
    }
    else
    {
        e->accept();
    }
}

bool QLCFixtureEditor::checkManufacturerModel()
{
    /* Check that the fixture has a manufacturer and a model for
       unique identification */
    if (m_fixtureDef->manufacturer().length() == 0)
    {
        QMessageBox::warning(this,
                             tr("Missing important information"),
                             tr("Missing manufacturer name.\n"
                                "Unable to save fixture."));
        m_tab->setCurrentIndex(0);
        m_manufacturerEdit->setFocus();
        return false;
    }
    else if (m_fixtureDef->model().length() == 0)
    {
        QMessageBox::warning(this,
                             tr("Missing important information"),
                             tr("Missing fixture model name.\n"
                                "Unable to save fixture."));
        m_tab->setCurrentIndex(0);
        m_modelEdit->setFocus();
        return false;
    }

    return true;
}

bool QLCFixtureEditor::save()
{
    if (checkManufacturerModel() == false)
        return false;

    if (m_fileName.simplified().isEmpty() == true)
    {
        return saveAs();
    }
    else
    {
        QFile::FileError error = m_fixtureDef->saveXML(m_fileName);
        if (error == QFile::NoError)
        {
            setModified(false);
            return true;
        }
        else
        {
            QMessageBox::critical(this, tr("Fixture saving failed"),
                                  tr("Unable to save fixture definition:\n%1")
                                  .arg(QLCFile::errorString(error)));
            return false;
        }
    }
}

bool QLCFixtureEditor::saveAs()
{
    /* Bail out if there is no manufacturer or model */
    if (checkManufacturerModel() == false)
        return false;

    /* Create a file save dialog */
    QFileDialog dialog(this);
    dialog.setWindowTitle(tr("Save fixture definition"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilter(KQXFFilter);

    QString path;
    QDir dir = QLCFixtureDefCache::userDefinitionDirectory();
    if (m_fileName.isEmpty() == true)
    {
        /* Construct a new path for the (yet) unnamed file */
        QString man = m_fixtureDef->manufacturer().replace(" ", "-");
        QString mod = m_fixtureDef->model().replace(" ", "-");
        path = QString("%1-%2%3").arg(man).arg(mod).arg(KExtFixture);
        dialog.setDirectory(dir);
        dialog.selectFile(path);
    }
    else
    {
        /* The fixture already has a file name. Use that then. */
        dialog.setDirectory(QDir(m_fileName));
        dialog.selectFile(m_fileName);
    }

    /* Execute the dialog */
    if (dialog.exec() != QDialog::Accepted)
        return false;

    path = dialog.selectedFiles().first();
    if (path.length() != 0)
    {
        if (path.right(strlen(KExtFixture)) != QString(KExtFixture))
            path += QString(KExtFixture);

        QFile::FileError error = m_fixtureDef->saveXML(path);
        if (error == QFile::NoError)
        {
            m_fileName = path;
            setCaption();
            setModified(false);
            return true;
        }
        else
        {
            QMessageBox::critical(this, tr("Fixture saving failed"),
                                  tr("Unable to save fixture definition:\n%1")
                                  .arg(QLCFile::errorString(error)));
            return false;
        }
    }
    else
    {
        return false;
    }
}

void QLCFixtureEditor::setCaption()
{
    QString caption;
    QString fileName;

    fileName = m_fileName;
    if (fileName.isEmpty())
        fileName = tr("New Fixture");

    /* If the document is modified, append an asterisk after the
       filename. Otherwise the caption is just the current filename */
    caption = QDir::toNativeSeparators(fileName);
    if (m_modified == true)
        caption += QString(" *");

    parentWidget()->setWindowTitle(caption);
}

void QLCFixtureEditor::setModified(bool modified)
{
    m_modified = modified;
    setCaption();
}

/*****************************************************************************
 * General tab functions
 *****************************************************************************/

void QLCFixtureEditor::slotManufacturerTextEdited(const QString &text)
{
    m_fixtureDef->setManufacturer(text);
    setModified();
}

void QLCFixtureEditor::slotModelTextEdited(const QString &text)
{
    m_fixtureDef->setModel(text);
    setModified();
}

void QLCFixtureEditor::slotAuthorTextEdited(const QString &text)
{
    m_fixtureDef->setAuthor(text);
    setModified();
}

void QLCFixtureEditor::slotTypeActivated(const QString &text)
{
    m_fixtureDef->setType(text);
    setModified();
}

/*****************************************************************************
 * Channels tab functions
 *****************************************************************************/

void QLCFixtureEditor::slotChannelListSelectionChanged(QTreeWidgetItem* item)
{
    if (item == NULL)
    {
        m_removeChannelButton->setEnabled(false);
        m_editChannelButton->setEnabled(false);
        m_copyChannelButton->setEnabled(false);
    }
    else
    {
        m_removeChannelButton->setEnabled(true);
        m_editChannelButton->setEnabled(true);
        m_copyChannelButton->setEnabled(true);
    }
}

void QLCFixtureEditor::slotAddChannel()
{
    EditChannel ec(this);

    bool ok = false;
    while (ok == false)
    {
        if (ec.exec() == QDialog::Accepted)
        {
            if (m_fixtureDef->channel(ec.channel()->name()) != NULL)
            {
                QMessageBox::warning(this,
                                     tr("Channel already exists"),
                                     tr("A channel by the name \"%1\" already exists!")
                                     .arg(ec.channel()->name()));
                ok = false;
            }
            else if (ec.channel()->name().length() == 0)
            {
                QMessageBox::warning(this,
                                     tr("Channel has no name"),
                                     tr("You must give the channel a descriptive name!"));
                ok = false;
            }
            else
            {
                /* Create a new channel and item for it */
                QTreeWidgetItem* item;
                QLCChannel* ch;

                ch = new QLCChannel(ec.channel());
                item = new QTreeWidgetItem(m_channelList);

                m_fixtureDef->addChannel(ch);
                updateChannelItem(ch, item);
                m_channelList->setCurrentItem(item);
                m_channelList->resizeColumnToContents(CH_COL_NAME);

                setModified();
                ok = true;
            }
        }
        else
        {
            /* Cancel pressed */
            ok = true;
        }
    }
}

void QLCFixtureEditor::slotRemoveChannel()
{
    QLCChannel* channel = currentChannel();
    Q_ASSERT(channel != NULL);

    if (QMessageBox::question(this, "Remove Channel",
                              tr("Are you sure you wish to remove channel: %1 ?")
                              .arg(channel->name()),
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
    {
        QTreeWidgetItem* item;
        QTreeWidgetItem* next;

        item = m_channelList->currentItem();
        if (m_channelList->itemBelow(item) != NULL)
            next = m_channelList->itemBelow(item);
        else if (m_channelList->itemAbove(item) != NULL)
            next = m_channelList->itemAbove(item);
        else
            next = NULL;

        // Remove the selected channel from the fixture (also deleted)
        m_fixtureDef->removeChannel(currentChannel());
        delete item;

        m_channelList->setCurrentItem(next);
        setModified();
    }
}

void QLCFixtureEditor::slotEditChannel()
{
    QLCChannel* real = NULL;
    QTreeWidgetItem* item = NULL;

    // Initialize the dialog with the selected logical channel or
    // bail out if there is no current selection
    real = currentChannel();
    if (real == NULL)
        return;

    EditChannel ec(this, real);
    if (ec.exec() == QDialog::Accepted)
    {
        if (m_fixtureDef->channel(ec.channel()->name()) != NULL && ec.channel()->name() != real->name())
        {
            QMessageBox::warning(this,
                                 tr("Channel already exists"),
                                 tr("A channel by the name \"%1\" already exists!")
                                 .arg(ec.channel()->name()));
        }
        else if (ec.channel()->name().length() == 0)
        {
            QMessageBox::warning(this,
                                 tr("Channel has no name"),
                                 tr("You must give the channel a descriptive name!"));
        }
        else
        {
            // Copy the channel's contents to the real channel
            *real = *(ec.channel());

            item = m_channelList->currentItem();
            updateChannelItem(real, item);
            m_channelList->resizeColumnToContents(CH_COL_NAME);

            setModified();
        }
    }
}

void QLCFixtureEditor::slotCopyChannel()
{
    _app->setCopyChannel(currentChannel());
}

void QLCFixtureEditor::slotPasteChannel()
{
    QLCChannel* ch = _app->copyChannel();
    if (ch != NULL && m_fixtureDef != NULL)
    {
        /* Create new mode and an item for it */
        QTreeWidgetItem* item;
        QLCChannel* copy;

        copy = new QLCChannel(ch);
        item = new QTreeWidgetItem(m_channelList);

        int cpIdx = 1;
        QString copyName;
        do
        {
            copyName = QString("%1 %2").arg(ch->name()).arg(cpIdx);
            cpIdx++;
        } while (m_fixtureDef->channel(copyName) != NULL);

        copy->setName(copyName);

        m_fixtureDef->addChannel(copy);
        updateChannelItem(copy, item);
        m_channelList->setCurrentItem(item);
        m_channelList->resizeColumnToContents(CH_COL_NAME);

        setModified();
    }
}

void QLCFixtureEditor::slotExpandChannels()
{
    if (m_channelList->topLevelItemCount() <= 0)
        return;
    else if (m_channelList->topLevelItem(0)->isExpanded() == true)
        m_channelList->collapseAll();
    else
        m_channelList->expandAll();
}

void QLCFixtureEditor::refreshChannelList()
{
    m_channelList->clear();

    /* Fill channels list */
    QListIterator <QLCChannel*> it(m_fixtureDef->channels());
    while (it.hasNext() == true)
        updateChannelItem(it.next(), new QTreeWidgetItem(m_channelList));

    slotChannelListSelectionChanged(m_channelList->currentItem());

    m_channelList->resizeColumnToContents(CH_COL_NAME);
    m_channelList->resizeColumnToContents(CH_COL_GRP);
}

void QLCFixtureEditor::updateChannelItem(const QLCChannel* channel, QTreeWidgetItem* item)
{
    QString str;

    Q_ASSERT(channel != NULL);
    Q_ASSERT(item != NULL);

    item->setText(CH_COL_NAME, channel->name());
    item->setIcon(CH_COL_NAME, channel->getIcon());
    item->setText(CH_COL_GRP, QLCChannel::groupToString(channel->group()));
    item->setData(CH_COL_NAME, PROP_PTR, (qulonglong) channel);

    /* Destroy the existing list of children */
    QList <QTreeWidgetItem*> children(item->takeChildren());
    foreach (QTreeWidgetItem* child, children)
    delete child;

    /* Put all capabilities as non-selectable sub items */
    QListIterator <QLCCapability*> capit(channel->capabilities());
    while (capit.hasNext() == true)
    {
        QLCCapability* cap = capit.next();
        Q_ASSERT(cap != NULL);

        QTreeWidgetItem* capitem = new QTreeWidgetItem(item);
        capitem->setText(CH_COL_NAME,
                         QString("[%1-%2]: %3").arg(cap->min())
                         .arg(cap->max()).arg(cap->name()));
        capitem->setFlags(0); /* No selection etc. */
    }
}


void QLCFixtureEditor::slotChannelListContextMenuRequested()
{
    QAction editAction(QIcon(":/edit.png"), tr("Edit"), this);
    QAction copyAction(QIcon(":/editcopy.png"), tr("Copy"), this);
    QAction pasteAction(QIcon(":/editpaste.png"), tr("Paste"), this);
    QAction removeAction(QIcon(":/editdelete.png"), tr("Remove"), this);

    /* Group menu */
    QMenu groupMenu;
    groupMenu.setTitle("Set group");
    QStringListIterator it(QLCChannel::groupList());
    while (it.hasNext() == true)
        groupMenu.addAction(it.next());

    /* Master edit menu */
    QMenu menu;
    menu.setTitle(tr("Channels"));
    menu.addAction(&editAction);
    menu.addAction(&copyAction);
    menu.addAction(&pasteAction);
    menu.addSeparator();
    menu.addAction(&removeAction);
    menu.addSeparator();
    menu.addMenu(&groupMenu);

    if (m_channelList->currentItem() == NULL)
    {
        copyAction.setEnabled(false);
        removeAction.setEnabled(false);
    }

    if (_app->copyChannel() == NULL)
        pasteAction.setEnabled(false);

    QAction* selectedAction = menu.exec(QCursor::pos());
    if (selectedAction == NULL)
        return;
    else if (selectedAction->text() == tr("Edit"))
        slotEditChannel();
    else if (selectedAction->text() == tr("Copy"))
        slotCopyChannel();
    else if (selectedAction->text() == tr("Paste"))
        slotPasteChannel();
    else if (selectedAction->text() == tr("Remove"))
        slotRemoveChannel();
    else
    {
        /* Group menu hook */
        QLCChannel* ch = NULL;
        QTreeWidgetItem* node = NULL;

        ch = currentChannel();
        if (ch != NULL)
            ch->setGroup(QLCChannel::stringToGroup(selectedAction->text()));
        node = m_channelList->currentItem();
        if (node != NULL)
            node->setText(CH_COL_GRP, selectedAction->text());
        setModified();
    }
}

void QLCFixtureEditor::slotChannelItemExpanded()
{
    m_channelList->resizeColumnToContents(CH_COL_NAME);
    m_channelList->resizeColumnToContents(CH_COL_GRP);
}

QLCChannel* QLCFixtureEditor::currentChannel()
{
    QLCChannel* ch = NULL;
    QTreeWidgetItem* item = NULL;

    // Convert the string-form ulong to a QLCChannel pointer and return it
    item = m_channelList->currentItem();
    if (item != NULL)
        ch = (QLCChannel*) item->data(CH_COL_NAME, PROP_PTR).toULongLong();

    return ch;
}

/*****************************************************************************
 * Modes tab functions
 *****************************************************************************/

void QLCFixtureEditor::slotModeListSelectionChanged(QTreeWidgetItem* item)
{
    if (item == NULL)
    {
        m_removeModeButton->setEnabled(false);
        m_editModeButton->setEnabled(false);
        m_cloneModeButton->setEnabled(false);
    }
    else
    {
        m_removeModeButton->setEnabled(true);
        m_editModeButton->setEnabled(true);
        m_cloneModeButton->setEnabled(true);
    }
}

void QLCFixtureEditor::slotAddMode()
{
    EditMode em(_app, m_fixtureDef);
    em.setClipboard(m_physicalCopy);
    bool ok = false;
    while (ok == false)
    {
        if (em.exec() == QDialog::Accepted)
        {
            if (m_fixtureDef->mode(em.mode()->name()) != NULL)
            {
                QMessageBox::warning(this,
                                     tr("Unable to add mode"),
                                     tr("Another mode by that name already exists"));

                // User must rename the mode to continue
                ok = false;
            }
            else if (em.mode()->name().length() == 0)
            {
                QMessageBox::warning(this,
                                     tr("Unable to add mode"),
                                     tr("You must give a name to the mode"));
                ok = false;
            }
            else
            {
                /* Create new mode and an item for it */
                QTreeWidgetItem* item;
                QLCFixtureMode* mode;

                mode = new QLCFixtureMode(m_fixtureDef, em.mode());
                item = new QTreeWidgetItem(m_modeList);

                m_fixtureDef->addMode(mode);
                updateModeItem(mode, item);
                m_modeList->setCurrentItem(item);
                m_modeList->resizeColumnToContents(MODE_COL_NAME);

                setModified();
                ok = true;
            }
        }
        else
        {
            /* Cancel pressed */
            ok = true;
        }
    }
    m_physicalCopy = em.getClipboard();
}

void QLCFixtureEditor::slotRemoveMode()
{
    QLCFixtureMode* mode = currentMode();

    if (QMessageBox::question(this, tr("Remove Mode"),
                              tr("Are you sure you wish to remove mode: %1 ?").arg(mode->name()),
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
    {
        m_fixtureDef->removeMode(mode);
        delete m_modeList->currentItem();
        setModified();
    }
}

void QLCFixtureEditor::slotEditMode()
{
    QLCFixtureMode* mode = currentMode();
    QTreeWidgetItem* item = NULL;

    if (mode == NULL)
        return;

    EditMode em(this, mode);
    em.setClipboard(m_physicalCopy);
    if (em.exec() == QDialog::Accepted)
    {
        *mode = *(em.mode());

        item = m_modeList->currentItem();
        updateModeItem(mode, item);
        setModified();
    }
    m_physicalCopy = em.getClipboard();
}

void QLCFixtureEditor::slotCloneMode()
{
    QLCFixtureMode* mode = NULL;
    bool ok = false;
    QString text;

    mode = currentMode();
    if (mode == NULL)
        return;

    while (1)
    {
        text = QInputDialog::getText(this, tr("Rename new mode"),
                                     tr("Give a unique name for the mode"),
                                     QLineEdit::Normal,
                                     tr("Copy of %1").arg(mode->name()),
                                     &ok);

        if (ok == true && text.isEmpty() == false)
        {
            /* User entered a name that is already found from
               the fixture definition -> again */
            if (mode->fixtureDef()->mode(text) != NULL)
            {
                QMessageBox::information(this,
                                         tr("Invalid name"),
                                         tr("Another mode by that name already exists."));
                ok = false;
                continue;
            }

            /* Create new mode and an item for it */
            QTreeWidgetItem* item;
            QLCFixtureMode* clone;

            clone = new QLCFixtureMode(mode->fixtureDef(), mode);
            clone->setName(text);
            item = new QTreeWidgetItem(m_modeList);

            mode->fixtureDef()->addMode(clone);
            updateModeItem(clone, item);
            m_modeList->setCurrentItem(item);

            setModified();
            break;
        }
        else
        {
            // User pressed cancel
            break;
        }
    }
}

void QLCFixtureEditor::slotExpandModes()
{
    if (m_modeList->topLevelItemCount() <= 0)
        return;
    else if (m_modeList->topLevelItem(0)->isExpanded() == true)
        m_modeList->collapseAll();
    else
        m_modeList->expandAll();
}

void QLCFixtureEditor::slotModeListContextMenuRequested()
{
    QAction editAction(QIcon(":/edit.png"), tr("Edit"), this);
    connect(&editAction, SIGNAL(triggered(bool)),
            this, SLOT(slotEditMode()));
    QAction cloneAction(QIcon(":/editcopy.png"), tr("Clone"), this);
    connect(&cloneAction, SIGNAL(triggered(bool)),
            this, SLOT(slotCloneMode()));
    QAction removeAction(QIcon(":/editdelete.png"), tr("Remove"), this);
    connect(&removeAction, SIGNAL(triggered(bool)),
            this, SLOT(slotRemoveMode()));

    QMenu menu;
    menu.setTitle(tr("Modes"));
    menu.addAction(&editAction);
    menu.addAction(&cloneAction);
    menu.addSeparator();
    menu.addAction(&removeAction);

    if (m_modeList->currentItem() == NULL)
    {
        editAction.setEnabled(false);
        cloneAction.setEnabled(false);
        removeAction.setEnabled(false);
    }

    menu.exec(QCursor::pos());
}

void QLCFixtureEditor::slotModeItemExpanded()
{
    m_modeList->resizeColumnToContents(MODE_COL_NAME);
}

void QLCFixtureEditor::refreshModeList()
{
    m_modeList->clear();

    /* Fill the list of modes */
    QListIterator <QLCFixtureMode*> it(m_fixtureDef->modes());
    while (it.hasNext() == true)
        updateModeItem(it.next(), new QTreeWidgetItem(m_modeList));

    slotModeListSelectionChanged(m_modeList->currentItem());

    m_modeList->resizeColumnToContents(MODE_COL_NAME);
    m_modeList->resizeColumnToContents(MODE_COL_HEAD);
    m_modeList->resizeColumnToContents(MODE_COL_CHS);
}

void QLCFixtureEditor::updateModeItem(const QLCFixtureMode* mode,
                                      QTreeWidgetItem* item)
{
    Q_ASSERT(mode != NULL);
    Q_ASSERT(item != NULL);

    item->setText(MODE_COL_NAME, mode->name());
    item->setData(MODE_COL_NAME, PROP_PTR, (qulonglong) mode);
    item->setText(MODE_COL_CHS, QString::number(mode->channels().size()));
    if (mode->heads().size() > 0)
        item->setText(MODE_COL_HEAD, QString::number(mode->heads().size()));
    else
        item->setText(MODE_COL_HEAD, QString());

    /* Destroy the existing list of children */
    QList <QTreeWidgetItem*> children(item->takeChildren());
    foreach (QTreeWidgetItem* child, children)
    delete child;

    /* Put all mode channels as non-selectable sub items */
    for (int i = 0; i < mode->channels().size(); i++)
    {
        QLCChannel* ch = mode->channel(i);
        Q_ASSERT(ch != NULL);

        QTreeWidgetItem* chitem = new QTreeWidgetItem(item);
        chitem->setText(MODE_COL_NAME, ch->name());
        chitem->setIcon(MODE_COL_NAME, ch->getIcon());
        chitem->setText(MODE_COL_CHS, QString("%1").arg(i + 1));
        chitem->setFlags(0); /* No selection etc. */
    }
}

QLCFixtureMode* QLCFixtureEditor::currentMode()
{
    QLCFixtureMode* mode = NULL;
    QTreeWidgetItem* item = NULL;

    // Convert the string-form ulong to a QLCChannel pointer and return it
    item = m_modeList->currentItem();
    if (item != NULL)
        mode = (QLCFixtureMode*) item->data(MODE_COL_NAME, PROP_PTR).toULongLong();

    return mode;
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

void QLCFixtureEditor::slotClipboardChanged()
{
    if (_app->copyChannel() != NULL)
        m_pasteChannelButton->setEnabled(true);
    else
        m_pasteChannelButton->setEnabled(false);
}
