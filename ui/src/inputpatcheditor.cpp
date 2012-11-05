/*
  Q Light Controller
  inputpatcheditor.cpp

  Copyright (C) Heikki Junnila

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
#include <QButtonGroup>
#include <QTreeWidget>
#include <QToolButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QComboBox>
#include <QGroupBox>
#include <QVariant>
#include <QDebug>
#include <QFile>
#include <QDir>

#include "qlcinputprofile.h"
#include "qlcioplugin.h"
#include "qlcconfig.h"
#include "qlcfile.h"

#include "inputprofileeditor.h"
#include "inputpatcheditor.h"
#include "inputpatch.h"
#include "inputmap.h"
#include "apputil.h"

/* Plugin column structure */
#define KMapColumnName  0
#define KMapColumnInput 1

/* Profile column structure */
#define KProfileColumnName 0

InputPatchEditor::InputPatchEditor(QWidget* parent, quint32 universe, InputMap* inputMap)
    : QWidget(parent)
    , m_inputMap(inputMap)
    , m_universe(universe)
{
    Q_ASSERT(universe < m_inputMap->universes());
    Q_ASSERT(inputMap != NULL);

    setupUi(this);

    m_infoBrowser->setOpenExternalLinks(true);

    InputPatch* inputPatch = m_inputMap->patch(universe);
    Q_ASSERT(inputPatch != NULL);

    /* Copy these so they can be applied if the user cancels */
    m_currentPluginName = inputPatch->pluginName();
    m_currentInput = inputPatch->input();
    m_currentProfileName = inputPatch->profileName();
    m_currentFeedbackEnabled = inputPatch->feedbackEnabled();

    /* Setup UI controls */
    setupMappingPage();
    setupProfilePage();

    /* Select the top-most "None" item */
    m_mapTree->setCurrentItem(m_mapTree->topLevelItem(0));

    /* Listen to plugin configuration changes */
    connect(m_inputMap, SIGNAL(pluginConfigurationChanged(const QString&)),
            this, SLOT(slotPluginConfigurationChanged(const QString&)));
}

InputPatchEditor::~InputPatchEditor()
{
}

/****************************************************************************
 * Mapping page
 ****************************************************************************/

InputPatch* InputPatchEditor::patch() const
{
    InputPatch* p = m_inputMap->patch(m_universe);
    Q_ASSERT(p != NULL);
    return p;
}

QTreeWidgetItem* InputPatchEditor::currentlyMappedItem() const
{
    for (int i = 0; i < m_mapTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* pluginItem = m_mapTree->topLevelItem(i);
        Q_ASSERT(pluginItem != NULL);

        if (pluginItem->text(KMapColumnName) == patch()->pluginName())
        {
            QTreeWidgetItem* inputItem = pluginItem->child(patch()->input());
            return inputItem;
        }
    }

    return NULL;
}

void InputPatchEditor::setupMappingPage()
{
    /* Fill the map tree with available plugins */
    fillMappingTree();

    /* Selection changes */
    connect(m_mapTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,
                              QTreeWidgetItem*)),
            this, SLOT(slotMapCurrentItemChanged(QTreeWidgetItem*)));

    /* Configure button */
    connect(m_configureButton, SIGNAL(clicked()),
            this, SLOT(slotConfigureInputClicked()));

    m_feedbackEnabledCheck->setChecked(m_currentFeedbackEnabled);
}

void InputPatchEditor::fillMappingTree()
{
    /* Disable check state change tracking when the tree is filled */
    disconnect(m_mapTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
               this, SLOT(slotMapItemChanged(QTreeWidgetItem*)));

    m_mapTree->clear();

    QStringList labels;
    labels << tr("Mapping for input universe %1").arg(m_universe + 1);
    m_mapTree->setHeaderLabels(labels);

    /* Add an empty item so that user can choose not to assign any plugin
       to an input universe */
    QTreeWidgetItem* pitem = new QTreeWidgetItem(m_mapTree);
    pitem->setText(KMapColumnName, KInputNone);
    pitem->setText(KMapColumnInput, QString("%1").arg(QLCIOPlugin::invalidLine()));
    pitem->setFlags(pitem->flags() | Qt::ItemIsUserCheckable);

    /* Set "Nothing" selected if there is no valid input selected */
    if (m_currentInput == QLCIOPlugin::invalidLine())
        pitem->setCheckState(KMapColumnName, Qt::Checked);
    else
        pitem->setCheckState(KMapColumnName, Qt::Unchecked);

    /* Go thru available plugins and put them as the tree's root nodes. */
    QStringListIterator pit(m_inputMap->pluginNames());
    while (pit.hasNext() == true)
        fillPluginItem(pit.next(), new QTreeWidgetItem(m_mapTree));

    /* Enable check state change tracking after the tree has been filled */
    connect(m_mapTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotMapItemChanged(QTreeWidgetItem*)));
}

void InputPatchEditor::fillPluginItem(const QString& pluginName, QTreeWidgetItem* pitem)
{
    QTreeWidgetItem* iitem = NULL;

    Q_ASSERT(pitem != NULL);

    /* Get rid of any existing children */
    while (pitem->childCount() > 0)
        delete pitem->child(0);

    pitem->setText(KMapColumnName, pluginName);
    pitem->setText(KMapColumnInput, QString("%1").arg(QLCIOPlugin::invalidLine()));

    /* Go thru available inputs provided by each plugin and put
       them as their parent plugin's leaf nodes. */
    quint32 i = 0;
    QStringListIterator iit(m_inputMap->pluginInputs(pluginName));
    while (iit.hasNext() == true)
    {
        iitem = new QTreeWidgetItem(pitem);
        iitem->setText(KMapColumnName, iit.next());
        iitem->setText(KMapColumnInput, QString("%1").arg(i));
        iitem->setFlags(iitem->flags() | Qt::ItemIsUserCheckable);

        /* Select the currently mapped plugin input and expand
           its parent node. */
        if (m_currentPluginName == pluginName && m_currentInput == i)
        {
            iitem->setCheckState(KMapColumnName, Qt::Checked);
            pitem->setExpanded(true);
        }
        else
        {
            iitem->setCheckState(KMapColumnName, Qt::Unchecked);
            quint32 uni = m_inputMap->mapping(pluginName, i);
            if (uni != InputMap::invalidUniverse())
            {
                /* If a mapping exists for this plugin and
                   output, make it impossible to map it to
                   another universe. */
                iitem->setFlags(iitem->flags() & (!Qt::ItemIsEnabled));
                QString name = iitem->text(KMapColumnName);
                name += QString(" (Mapped to universe %1)").arg(uni + 1);
                iitem->setText(KMapColumnName, name);
            }
        }

        i++;
    }

    /* If no inputs were appended to the plugin node, put a
       "Nothing" node there. */
    if (i == 0)
    {
        iitem = new QTreeWidgetItem(pitem);
        iitem->setText(KMapColumnName, KInputNone);
        iitem->setText(KMapColumnInput, QString("%1").arg(QLCIOPlugin::invalidLine()));
        iitem->setFlags(iitem->flags() & ~Qt::ItemIsEnabled);
        iitem->setFlags(iitem->flags() & ~Qt::ItemIsSelectable);
        iitem->setCheckState(KMapColumnName, Qt::Unchecked);
    }
}

void InputPatchEditor::slotMapCurrentItemChanged(QTreeWidgetItem* item)
{
    QString info;
    bool configurable;

    if (item == NULL)
    {
        info = m_inputMap->pluginStatus(QString(), 0);
        configurable = false;
    }
    else
    {
        QString plugin;
        quint32 input;

        if (item->parent() != NULL)
        {
            /* Input node selected */
            plugin = item->parent()->text(KMapColumnName);
            input = item->text(KMapColumnInput).toInt();
        }
        else
        {
            /* Plugin node selected */
            plugin = item->text(KMapColumnName);
            input = QLCIOPlugin::invalidLine();
        }

        info = m_inputMap->pluginStatus(plugin, input);
        configurable = m_inputMap->canConfigurePlugin(plugin);
    }

    /* Display information for the selected plugin or input */
    m_infoBrowser->setText(info);

    /* Enable configuration if plugin supports it */
    m_configureButton->setEnabled(configurable);
}

void InputPatchEditor::slotMapItemChanged(QTreeWidgetItem* item)
{
    if (item == NULL)
        return;

    if (item->checkState(KMapColumnName) == Qt::Checked)
    {
        /* Temporarily disable this signal to prevent an endless loop */
        disconnect(m_mapTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
                   this, SLOT(slotMapItemChanged(QTreeWidgetItem*)));

        /* Set all other items unchecked... */
        QTreeWidgetItemIterator it(m_mapTree);
        while ((*it) != NULL)
        {
            /* Don't touch the item that was just checked nor
               any parent nodes. */
            if (*it != item && (*it)->childCount() == 0)
            {
                /* Set all the rest of the nodes unchecked */
                (*it)->setCheckState(KMapColumnName,
                                     Qt::Unchecked);
            }

            /* Next one */
            ++it;
        }

        /* Start listening to this signal once again */
        connect(m_mapTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
                this, SLOT(slotMapItemChanged(QTreeWidgetItem*)));
    }
    else
    {
        /* Don't allow unchecking an item by clicking it. Only allow
           changing the check state by checking another item. */
        item->setCheckState(KMapColumnName, Qt::Checked);
    }

    /* Store the selected plugin name & input */
    if (item->parent() != NULL)
    {
        m_currentPluginName = item->parent()->text(KMapColumnName);
        m_currentInput = item->text(KMapColumnInput).toInt();
    }
    else
    {
        m_currentPluginName = KInputNone;
        m_currentInput = QLCIOPlugin::invalidLine();
    }

    /* Apply the patch immediately so that input data can be used in the
       input profile editor */
    m_inputMap->setPatch(m_universe, m_currentPluginName,
                         m_currentInput, m_currentFeedbackEnabled,
                         m_currentProfileName);
    emit mappingChanged();
}

void InputPatchEditor::slotConfigureInputClicked()
{
    QTreeWidgetItem* item;
    QString plugin;

    /* Find out the currently selected plugin */
    item = m_mapTree->currentItem();
    if (item == NULL)
        return;
    else if (item->parent() != NULL)
        plugin = item->parent()->text(KMapColumnName);
    else
        plugin = item->text(KMapColumnName);

    /* Configure the plugin. Changes in plugin outputs are handled with
       slotPluginConfigurationChanged(). */
    m_inputMap->configurePlugin(plugin);
}

void InputPatchEditor::slotFeedbackToggled(bool enable)
{
    m_currentFeedbackEnabled = enable;

    /* Apply the patch immediately so that input data can be used in the
       input profile editor */
    m_inputMap->setPatch(m_universe, m_currentPluginName,
                               m_currentInput, m_currentFeedbackEnabled,
                               m_currentProfileName);
    emit mappingChanged();
}

void InputPatchEditor::slotPluginConfigurationChanged(const QString& pluginName)
{
    QTreeWidgetItem* item = pluginItem(pluginName);
    if (item == NULL)
        return;

    /* Disable check state tracking while the item is being filled */
    disconnect(m_mapTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
               this, SLOT(slotMapItemChanged(QTreeWidgetItem*)));

    /* Re-fill the children for the plugin that's been changed */
    fillPluginItem(pluginName, pluginItem(pluginName));

    slotMapCurrentItemChanged(item);

    /* Enable check state tracking after the item has been filled */
    connect(m_mapTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotMapItemChanged(QTreeWidgetItem*)));
}

QTreeWidgetItem* InputPatchEditor::pluginItem(const QString& pluginName)
{
    for (int i = 0; i < m_mapTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_mapTree->topLevelItem(i);
        if (item->text(KMapColumnName) == pluginName)
            return item;
    }

    return NULL;
}

/****************************************************************************
 * Profile tree
 ****************************************************************************/

void InputPatchEditor::setupProfilePage()
{
    connect(m_addProfileButton, SIGNAL(clicked()),
            this, SLOT(slotAddProfileClicked()));
    connect(m_removeProfileButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveProfileClicked()));
    connect(m_editProfileButton, SIGNAL(clicked()),
            this, SLOT(slotEditProfileClicked()));

    /* Fill the profile tree with available profile names */
    fillProfileTree();

    /* Listen to itemChanged() signals to catch check state changes */
    connect(m_profileTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotProfileItemChanged(QTreeWidgetItem*)));

    /* Double click acts as edit button click */
    connect(m_profileTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotEditProfileClicked()));
}

void InputPatchEditor::fillProfileTree()
{
    QTreeWidgetItem* item;

    m_profileTree->clear();

    /* Add an option for having no profile at all */
    item = new QTreeWidgetItem(m_profileTree);
    updateProfileItem(KInputNone, item);

    /* Insert available input profiles to the tree */
    QStringListIterator it(m_inputMap->profileNames());
    while (it.hasNext() == true)
    {
        item = new QTreeWidgetItem(m_profileTree);
        updateProfileItem(it.next(), item);
    }
}

void InputPatchEditor::updateProfileItem(const QString& name, QTreeWidgetItem* item)
{
    Q_ASSERT(item != NULL);

    item->setText(KProfileColumnName, name);

    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    if (m_currentProfileName == name)
        item->setCheckState(KProfileColumnName, Qt::Checked);
    else
        item->setCheckState(KProfileColumnName, Qt::Unchecked);
}

QString InputPatchEditor::fullProfilePath(const QString& manufacturer,
                                          const QString& model) const
{
    QDir dir(InputMap::userProfileDirectory());
    QString path = QString("%1/%2-%3%4").arg(dir.absolutePath())
                                        .arg(manufacturer).arg(model)
                                        .arg(KExtInputProfile);

    return path;
}

void InputPatchEditor::slotProfileItemChanged(QTreeWidgetItem* item)
{
    if (item->checkState(KProfileColumnName) == Qt::Checked)
    {
        /* Temporarily disable this signal to prevent an endless loop */
        disconnect(m_profileTree,
                   SIGNAL(itemChanged(QTreeWidgetItem*,int)),
                   this,
                   SLOT(slotProfileItemChanged(QTreeWidgetItem*)));

        /* Set all other items unchecked... */
        QTreeWidgetItemIterator it(m_profileTree);
        while (*it != NULL)
        {
            /* ...except the one that was just checked */
            if (*it != item)
                (*it)->setCheckState(KProfileColumnName,
                                     Qt::Unchecked);
            ++it;
        }

        /* Start listening to this signal once again */
        connect(m_profileTree,
                SIGNAL(itemChanged(QTreeWidgetItem*,int)),
                this,
                SLOT(slotProfileItemChanged(QTreeWidgetItem*)));
    }
    else
    {
        /* Don't allow unchecking an item by clicking it. Only allow
           changing the check state by checking another item. */
        item->setCheckState(KProfileColumnName, Qt::Checked);
    }

    /* Store the selected profile name */
    m_currentProfileName = item->text(KProfileColumnName);

    /* Apply the patch immediately */
    m_inputMap->setPatch(m_universe, m_currentPluginName,
                               m_currentInput, m_currentFeedbackEnabled,
                               m_currentProfileName);
    emit mappingChanged();
}

void InputPatchEditor::slotAddProfileClicked()
{
    /* Create a new input profile and start editing it */
    InputProfileEditor ite(this, NULL, m_inputMap);
edit:
    if (ite.exec() == QDialog::Accepted)
    {
        /* Remove spaces from these */
        QString manufacturer = ite.profile()->manufacturer().remove(QChar(' '));
        QString model = ite.profile()->model().remove(QChar(' '));
        QString path = fullProfilePath(manufacturer, model);

        /* If another profile with the same name exists, ask permission to overwrite */
        if (QFile::exists(path) == true && path != ite.profile()->path())
        {
            int r = QMessageBox::warning(this, tr("Existing Input Profile"),
                    tr("An input profile at %1 already exists. Do you wish to overwrite it?").arg(path),
                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                    QMessageBox::No);
            if (r == QMessageBox::Cancel)
            {
                goto edit;
            }
            else if (r == QMessageBox::No)
            {
                path = QFileDialog::getSaveFileName(this, tr("Save Input Profile"),
                                                    path, tr("Input Profiles (*.qxi)"));
                if (path.isEmpty() == true)
                    goto edit;
            }
        }

        /* Create a new non-const copy of the profile and
           reparent it to the input map */
        QLCInputProfile* profile = new QLCInputProfile(*ite.profile());

        /* Save it to a file, go back to edit if save failed */
        if (profile->saveXML(path) == false)
        {
            QMessageBox::warning(this, tr("Saving failed"),
                                 tr("Unable to save the profile to %1")
                                 .arg(QDir::toNativeSeparators(path)));
            delete profile;
            goto edit;
        }
        else
        {
            /* Add the new profile to input map */
            m_inputMap->addProfile(profile);

            /* Add the new profile to our tree widget */
            QTreeWidgetItem* item;
            item = new QTreeWidgetItem(m_profileTree);
            updateProfileItem(profile->name(), item);
        }
    }
}

void InputPatchEditor::slotRemoveProfileClicked()
{
    QLCInputProfile* profile;
    QTreeWidgetItem* item;
    QString name;
    int r;

    /* Find out the currently selected item */
    item = m_profileTree->currentItem();
    if (item == NULL)
        return;

    /* Get the currently selected profile object by its name */
    name = item->text(KProfileColumnName);
    profile = m_inputMap->profile(name);
    if (profile == NULL)
        return;

    /* Ask for user confirmation */
    r = QMessageBox::question(this, tr("Delete profile"),
                              tr("Do you wish to permanently delete profile \"%1\"?")
                              .arg(profile->name()),
                              QMessageBox::Yes, QMessageBox::No);
    if (r == QMessageBox::Yes)
    {
        /* Attempt to delete the file first */
        QFile file(profile->path());
        if (file.remove() == true)
        {
            if (item->checkState(KProfileColumnName) == Qt::Checked)
            {
                /* The currently assigned profile is removed,
                   so select "None" next. */
                QTreeWidgetItem* none;
                none = m_profileTree->topLevelItem(0);
                Q_ASSERT(none != NULL);
                none->setCheckState(KProfileColumnName,
                                    Qt::Checked);
            }

            /* Successful deletion. Remove the profile from
               input map and our tree widget */
            m_inputMap->removeProfile(name);
            delete item;
        }
        else
        {
            /* Annoy the user even more after deletion failure */
            QMessageBox::warning(this, tr("File deletion failed"),
                                 tr("Unable to delete file %1")
                                 .arg(file.errorString()));
        }
    }
}

void InputPatchEditor::slotEditProfileClicked()
{
    QLCInputProfile* profile;
    QTreeWidgetItem* item;
    QString name;

    /* Get the currently selected item and bail out if nothing or "None"
       is selected */
    item = m_profileTree->currentItem();
    if (item == NULL || item->text(KProfileColumnName) == KInputNone)
        return;

    /* Get the currently selected profile by its name */
    name = item->text(KProfileColumnName);
    profile = m_inputMap->profile(name);
    if (profile == NULL)
        return;

    /* Edit the profile and update the item if OK was pressed */
    InputProfileEditor ite(this, profile, m_inputMap);
edit:
    if (ite.exec() == QDialog::Rejected)
        return;

    /* Copy the channel's contents from the editor's copy to
       the actual object (with QLCInputProfile::operator=()). */
    *profile = *ite.profile();

    /* Remove spaces from these */
    QString manufacturer = ite.profile()->manufacturer().remove(QChar(' '));
    QString model = ite.profile()->model().remove(QChar(' '));
    QString path = fullProfilePath(manufacturer, model);

    /* If another profile with the same name exists, ask permission to overwrite */
    if (QFile::exists(path) == true && path != ite.profile()->path())
    {
        int r = QMessageBox::warning(this, tr("Existing Input Profile"),
                tr("An input profile at %1 already exists. Do you wish to overwrite it?").arg(path),
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                QMessageBox::No);
        if (r == QMessageBox::Cancel)
        {
            goto edit;
        }
        else if (r == QMessageBox::No)
        {
            path = QFileDialog::getSaveFileName(this, tr("Save Input Profile"),
                                                path, tr("Input Profiles (*.qxi)"));
            if (path.isEmpty() == true)
                goto edit;
        }
    }

    /* Save the profile */
    if (profile->saveXML(path) == true)
    {
        /* Get the profile's name from the profile itself
           since it may have changed making local variable
           "name" invalid */
        updateProfileItem(profile->name(), item);
    }
    else
    {
        QMessageBox::warning(this, tr("Saving failed"),
                             tr("Unable to save %1 to %2")
                             .arg(profile->name())
                             .arg(QDir::toNativeSeparators(path)));
        goto edit;
    }
}
