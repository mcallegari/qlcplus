/*
  Q Light Controller
  inputoutputpatcheditor.cpp

  Copyright (C) Massimo Callegari

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

#include "inputoutputpatcheditor.h"
#include "inputprofileeditor.h"
#include "outputpatch.h"
#include "inputpatch.h"
#include "outputmap.h"
#include "inputmap.h"
#include "apputil.h"

/* Plugin column structure */
#define KMapColumnPluginName    0
#define KMapColumnDeviceName    1
#define KMapColumnHasInput      2
#define KMapColumnHasOutput     3
#define KMapColumnHasFeedback   4
#define KMapColumnInputLine     5
#define KMapColumnOutputLine    6

/* Profile column structure */
#define KProfileColumnName 0

InputOutputPatchEditor::InputOutputPatchEditor(QWidget* parent, quint32 universe, InputMap* inputMap, OutputMap* outputMap)
    : QWidget(parent)
    , m_inputMap(inputMap)
    , m_outputMap(outputMap)
    , m_universe(universe)
{
    Q_ASSERT(universe < m_inputMap->universes());
    Q_ASSERT(inputMap != NULL);
    Q_ASSERT(outputMap != NULL);

    setupUi(this);

    m_infoBrowser->setOpenExternalLinks(true);

    InputPatch* inputPatch = m_inputMap->patch(universe);
    OutputPatch* outputPatch = m_outputMap->patch(universe);
    OutputPatch* feedbackPatch = m_outputMap->feedbackPatch(universe);

    Q_ASSERT(inputPatch != NULL);
    Q_ASSERT(outputPatch != NULL);

    /* Copy these so they can be applied if the user cancels */
    m_currentInputPluginName = inputPatch->pluginName();
    m_currentInput = inputPatch->input();
    m_currentProfileName = inputPatch->profileName();
    //m_currentFeedbackEnabled = inputPatch->feedbackEnabled();

    m_currentOutputPluginName = outputPatch->pluginName();
    m_currentOutput = outputPatch->output();

    m_currentFeedbackPluginName = feedbackPatch->pluginName();
    m_currentFeedback = feedbackPatch->output();

    m_mapTree->header()->setResizeMode(QHeaderView::ResizeToContents);
    m_mapTree->setSortingEnabled(true);
    m_mapTree->sortByColumn(KMapColumnPluginName, Qt::AscendingOrder);

    /* Setup UI controls */
    setupMappingPage();
    setupProfilePage();

    /* Select the top-most "None" item */
    m_mapTree->setCurrentItem(m_mapTree->topLevelItem(0));

    /* Listen to plugin configuration changes */
    connect(m_inputMap, SIGNAL(pluginConfigurationChanged(const QString&)),
            this, SLOT(slotPluginConfigurationChanged(const QString&)));
    /* Listen to plugin configuration changes */
    connect(m_outputMap, SIGNAL(pluginConfigurationChanged(const QString&)),
            this, SLOT(slotPluginConfigurationChanged(const QString&)));
}

InputOutputPatchEditor::~InputOutputPatchEditor()
{
}

/****************************************************************************
 * Mapping page
 ****************************************************************************/

InputPatch* InputOutputPatchEditor::patch() const
{
    InputPatch* p = m_inputMap->patch(m_universe);
    Q_ASSERT(p != NULL);
    return p;
}

QTreeWidgetItem* InputOutputPatchEditor::currentlyMappedItem() const
{
    for (int i = 0; i < m_mapTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* pluginItem = m_mapTree->topLevelItem(i);
        Q_ASSERT(pluginItem != NULL);

        if (pluginItem->text(KMapColumnPluginName) == patch()->pluginName())
        {
            QTreeWidgetItem* inputItem = pluginItem->child(patch()->input());
            return inputItem;
        }
    }

    return NULL;
}

void InputOutputPatchEditor::setupMappingPage()
{
    /* Fill the map tree with available plugins */
    fillMappingTree();

    /* Selection changes */
    connect(m_mapTree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
            this, SLOT(slotMapCurrentItemChanged(QTreeWidgetItem*)));

    /* Configure button */
    connect(m_configureButton, SIGNAL(clicked()),
            this, SLOT(slotConfigureInputClicked()));

    //m_feedbackEnabledCheck->setChecked(m_currentFeedbackEnabled);
}

QTreeWidgetItem *InputOutputPatchEditor::itemLookup(QString pluginName, QString devName)
{
    for (int d = 0; d < m_mapTree->topLevelItemCount(); d++)
    {
        QTreeWidgetItem *item = m_mapTree->topLevelItem(d);
        if(item->text(KMapColumnPluginName) == pluginName)
        {
            if (devName.isEmpty())
                return item;
            else
            {
                if (item->text(KMapColumnDeviceName) == devName ||
                    item->text(KMapColumnDeviceName) == KInputNone)
                        return item;
            }
        }
    }
    return NULL;
}

void InputOutputPatchEditor::fillMappingTree()
{
    /* Disable check state change tracking when the tree is filled */
    disconnect(m_mapTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
               this, SLOT(slotMapItemChanged(QTreeWidgetItem*, int)));

    m_mapTree->clear();

    /* Go through available input plugins and create tree nodes. */
    QStringListIterator inputIt(m_inputMap->pluginNames());
    while (inputIt.hasNext() == true)
    {
        quint32 i = 0;
        QString pluginName = inputIt.next();
        QStringListIterator iit(m_inputMap->pluginInputs(pluginName));
        while (iit.hasNext() == true)
        {
            QTreeWidgetItem* pitem = new QTreeWidgetItem(m_mapTree);
            pitem->setText(KMapColumnPluginName, pluginName);
            pitem->setText(KMapColumnDeviceName, iit.next());
            pitem->setFlags(pitem->flags() | Qt::ItemIsUserCheckable);
            if (m_currentInputPluginName == pluginName && m_currentInput == i)
                pitem->setCheckState(KMapColumnHasInput, Qt::Checked);
            else
                pitem->setCheckState(KMapColumnHasInput, Qt::Unchecked);
            pitem->setTextAlignment(KMapColumnHasInput, Qt::AlignHCenter);
            pitem->setText(KMapColumnInputLine, QString("%1").arg(i));
            pitem->setText(KMapColumnOutputLine, QString("%1").arg(QLCIOPlugin::invalidLine()));
            i++;
        }

        if (i == 0)
        {
            QTreeWidgetItem* pitem = new QTreeWidgetItem(m_mapTree);
            pitem->setText(KMapColumnPluginName, pluginName);
            pitem->setText(KMapColumnDeviceName, KInputNone);
            pitem->setText(KMapColumnInputLine, QString("%1").arg(QLCIOPlugin::invalidLine()));
            pitem->setText(KMapColumnOutputLine, QString("%1").arg(QLCIOPlugin::invalidLine()));
        }
    }
    /* Go through available output plugins and create/update tree nodes. */
    QStringListIterator outputIt(m_outputMap->pluginNames());
    while (outputIt.hasNext() == true)
    {
        QString pluginName = outputIt.next();
        quint32 i = 0;

        QTreeWidgetItem *item = itemLookup(pluginName, "");
        /* 1st case: Add new plugin */
        if (item == NULL)
        {
            QStringListIterator iit(m_outputMap->pluginOutputs(pluginName));
            while (iit.hasNext() == true)
            {
                /* Output capable device */
                QString devName = iit.next();
                QTreeWidgetItem* pitem = new QTreeWidgetItem(m_mapTree);
                pitem->setText(KMapColumnPluginName, pluginName);
                pitem->setText(KMapColumnDeviceName, devName);
                pitem->setFlags(pitem->flags() | Qt::ItemIsUserCheckable);
                if (m_currentOutputPluginName == pluginName && m_currentOutput == i)
                    pitem->setCheckState(KMapColumnHasOutput, Qt::Checked);
                else
                    pitem->setCheckState(KMapColumnHasOutput, Qt::Unchecked);
                pitem->setText(KMapColumnOutputLine, QString("%1").arg(i));
                pitem->setText(KMapColumnInputLine, QString("%1").arg(QLCIOPlugin::invalidLine()));
                /* If a device has an output, it means it can send feedbacks */
                if (pluginName == "MIDI")
                {
                    if (m_currentFeedbackPluginName == pluginName && m_currentFeedback == i)
                        pitem->setCheckState(KMapColumnHasFeedback, Qt::Checked);
                    else
                        pitem->setCheckState(KMapColumnHasFeedback, Qt::Unchecked);
                }
                i++;
            }
            if (i == 0)
            {
                /* Device not available: No input and no output */
                QTreeWidgetItem* pitem = new QTreeWidgetItem(m_mapTree);
                pitem->setText(KMapColumnPluginName, pluginName);
                pitem->setText(KMapColumnDeviceName, KInputNone);
                pitem->setText(KMapColumnOutputLine, QString("%1").arg(QLCIOPlugin::invalidLine()));
                pitem->setText(KMapColumnInputLine, QString("%1").arg(QLCIOPlugin::invalidLine()));
            }
        }
        else
        {
            QStringListIterator iit(m_outputMap->pluginOutputs(pluginName));
            while (iit.hasNext() == true)
            {
                QString devName = iit.next();
                QTreeWidgetItem *item = itemLookup(pluginName, devName);
                /* 2nd case: add new output-only device to an existing plugin */
                if (item == NULL)
                {
                    item = new QTreeWidgetItem(m_mapTree);
                    item->setText(KMapColumnPluginName, pluginName);
                    item->setText(KMapColumnDeviceName, devName);
                    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                    if (m_currentOutputPluginName == pluginName && m_currentOutput == i)
                        item->setCheckState(KMapColumnHasOutput, Qt::Checked);
                    else
                        item->setCheckState(KMapColumnHasOutput, Qt::Unchecked);

                    item->setText(KMapColumnOutputLine, QString("%1").arg(i));
                    item->setText(KMapColumnInputLine, QString("%1").arg(QLCIOPlugin::invalidLine()));
                }
                else
                {
                    /* 3rd case: add an output line to an existing device with an input line */
                    item->setText(KMapColumnDeviceName, devName);
                    if (m_currentOutputPluginName == pluginName && m_currentOutput == i)
                        item->setCheckState(KMapColumnHasOutput, Qt::Checked);
                    else
                        item->setCheckState(KMapColumnHasOutput, Qt::Unchecked);
                    item->setText(KMapColumnOutputLine, QString("%1").arg(i));
                }
                /* If a device has both input and output, it means it can send feedbacks */
                if (pluginName == "MIDI")
                {
                    if (m_currentFeedbackPluginName == pluginName && m_currentFeedback == i)
                        item->setCheckState(KMapColumnHasFeedback, Qt::Checked);
                    else
                        item->setCheckState(KMapColumnHasFeedback, Qt::Unchecked);
                }
                i++;
            }
        }
    }

    /* Enable check state change tracking after the tree has been filled */
    connect(m_mapTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotMapItemChanged(QTreeWidgetItem*, int)));
}

void InputOutputPatchEditor::slotMapCurrentItemChanged(QTreeWidgetItem* item)
{
    QString info;
    bool configurable;

    if (item == NULL)
    {
        info = m_inputMap->pluginStatus(QString(), 0);
        info += m_outputMap->pluginStatus(QString(), 0);
        configurable = false;
    }
    else
    {
        QString plugin;
        quint32 input;
        quint32 output;

        /* Input node selected */
        plugin = item->text(KMapColumnPluginName);
        input = item->text(KMapColumnInputLine).toUInt();
        output = item->text(KMapColumnOutputLine).toUInt();

        info = m_inputMap->pluginDescription(plugin);

        //if (input != QLCIOPlugin::invalidLine())
            info += m_inputMap->pluginStatus(plugin, input);
        //if (output != QLCIOPlugin::invalidLine())
            info += m_outputMap->pluginStatus(plugin, output);
        configurable = m_inputMap->canConfigurePlugin(plugin) | m_outputMap->canConfigurePlugin(plugin);
    }

    /* Display information for the selected plugin or input */
    m_infoBrowser->setText(info);

    /* Enable configuration if plugin supports it */
    m_configureButton->setEnabled(configurable);
}

void InputOutputPatchEditor::slotMapItemChanged(QTreeWidgetItem* item, int col)
{
    if (item == NULL)
        return;

    /* Temporarily disable this signal to prevent an endless loop */
    disconnect(m_mapTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
               this, SLOT(slotMapItemChanged(QTreeWidgetItem*, int)));


    if (item->checkState(col) == Qt::Checked)
    {
        /* Set all other items unchecked... */
        QTreeWidgetItemIterator it(m_mapTree);
        while ((*it) != NULL)
        {
            /* Don't touch the item that was just checked */
            if (*it != item && (*it)->checkState(col))
            {
                /* Set all the rest of the nodes unchecked */
                (*it)->setCheckState(col, Qt::Unchecked);
            }
            ++it;
        }
        if (col == KMapColumnHasInput)
        {
            /* Store the selected plugin name & input */
            m_currentInputPluginName = item->text(KMapColumnPluginName);
            m_currentInput = item->text(KMapColumnInputLine).toInt();

            /* Apply the patch immediately so that input data can be used in the
               input profile editor */
            m_inputMap->setPatch(m_universe, m_currentInputPluginName,
                                 m_currentInput, m_currentProfileName);
        }
        else if (col == KMapColumnHasOutput)
        {
            if (item->checkState(KMapColumnHasFeedback) == Qt::Checked)
            {
                item->setCheckState(KMapColumnHasOutput, Qt::Unchecked);
                QMessageBox::warning(this, tr("Error"),
                                     tr("Output line already assigned"));
            }
            else
            {
                /* Store the selected plugin name & line */
                m_currentOutputPluginName = item->text(KMapColumnPluginName);
                m_currentOutput = item->text(KMapColumnOutputLine).toInt();

                /* Apply the patch immediately */
                m_outputMap->setPatch(m_universe, m_currentOutputPluginName, m_currentOutput, false);
            }
        }
        else if (col == KMapColumnHasFeedback)
        {
            if (item->checkState(KMapColumnHasOutput) == Qt::Checked)
            {
                item->setCheckState(KMapColumnHasFeedback, Qt::Unchecked);
                QMessageBox::warning(this, tr("Error"),
                                     tr("Output line already assigned"));
            }
            else
            {
                m_currentFeedbackPluginName = item->text(KMapColumnPluginName);
                m_currentFeedback = item->text(KMapColumnOutputLine).toInt();

                /* Apply the patch immediately */
                m_outputMap->setPatch(m_universe, m_currentFeedbackPluginName, m_currentFeedback, true);
            }
        }
    }
    else
    {
        /* Unchecked action. Set the patch to none */
        if (col == KMapColumnHasInput)
        {
            m_currentInputPluginName = KInputNone;
            m_currentInput = QLCIOPlugin::invalidLine();

            m_inputMap->setPatch(m_universe, m_currentInputPluginName, m_currentInput);
        }
        else if (col == KMapColumnHasOutput)
        {
            m_currentOutputPluginName = KInputNone;
            m_currentOutput = QLCIOPlugin::invalidLine();

            /* Apply the patch immediately */
            m_outputMap->setPatch(m_universe, m_currentOutputPluginName, m_currentOutput, false);
        }
        else if (col == KMapColumnHasFeedback)
        {
            m_currentFeedbackPluginName = KInputNone;
            m_currentFeedback = QLCIOPlugin::invalidLine();

            /* Apply the patch immediately */
            m_outputMap->setPatch(m_universe, m_currentFeedbackPluginName, m_currentFeedback, true);
        }
    }

    /* Start listening to this signal once again */
    connect(m_mapTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotMapItemChanged(QTreeWidgetItem*, int)));

    emit mappingChanged();
}

void InputOutputPatchEditor::slotConfigureInputClicked()
{
    QTreeWidgetItem* item;
    QString plugin;

    /* Find out the currently selected plugin */
    item = m_mapTree->currentItem();
    if (item == NULL)
        return;
    else
        plugin = item->text(KMapColumnPluginName);

    /* Configure the plugin. Changes in plugin outputs are handled with
       slotPluginConfigurationChanged(). */
    m_inputMap->configurePlugin(plugin);
}

void InputOutputPatchEditor::slotPluginConfigurationChanged(const QString& pluginName)
{
    QTreeWidgetItem* item = pluginItem(pluginName);
    if (item == NULL)
        return;

    /* Disable check state tracking while the item is being filled */
    disconnect(m_mapTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
               this, SLOT(slotMapItemChanged(QTreeWidgetItem*, int)));

    /* Update the IO map */
    slotMapCurrentItemChanged(item);

    /* Enable check state tracking after the item has been filled */
    connect(m_mapTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotMapItemChanged(QTreeWidgetItem*, int)));
}

QTreeWidgetItem* InputOutputPatchEditor::pluginItem(const QString& pluginName)
{
    for (int i = 0; i < m_mapTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_mapTree->topLevelItem(i);
        if (item->text(KMapColumnPluginName) == pluginName)
            return item;
    }

    return NULL;
}

/****************************************************************************
 * Profile tree
 ****************************************************************************/

void InputOutputPatchEditor::setupProfilePage()
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

void InputOutputPatchEditor::fillProfileTree()
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

void InputOutputPatchEditor::updateProfileItem(const QString& name, QTreeWidgetItem* item)
{
    Q_ASSERT(item != NULL);

    item->setText(KProfileColumnName, name);

    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    if (m_currentProfileName == name)
        item->setCheckState(KProfileColumnName, Qt::Checked);
    else
        item->setCheckState(KProfileColumnName, Qt::Unchecked);
}

QString InputOutputPatchEditor::fullProfilePath(const QString& manufacturer,
                                          const QString& model) const
{
    QDir dir(InputMap::userProfileDirectory());
    QString path = QString("%1/%2-%3%4").arg(dir.absolutePath())
                                        .arg(manufacturer).arg(model)
                                        .arg(KExtInputProfile);

    return path;
}

void InputOutputPatchEditor::slotProfileItemChanged(QTreeWidgetItem* item)
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
    m_inputMap->setPatch(m_universe, m_currentInputPluginName,
                               m_currentInput, m_currentProfileName);
    emit mappingChanged();
}

void InputOutputPatchEditor::slotAddProfileClicked()
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

void InputOutputPatchEditor::slotRemoveProfileClicked()
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

void InputOutputPatchEditor::slotEditProfileClicked()
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
