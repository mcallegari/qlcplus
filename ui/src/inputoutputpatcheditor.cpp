/*
  Q Light Controller
  inputoutputpatcheditor.cpp

  Copyright (C) Massimo Callegari

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
#include "inputoutputmap.h"
#include "outputpatch.h"
#include "inputpatch.h"
#include "apputil.h"
#include "doc.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
 #if defined( __APPLE__) || defined(Q_OS_MAC)
  #include "audiorenderer_portaudio.h"
  #include "audiocapture_portaudio.h"
 #elif defined(WIN32) || defined(Q_OS_WIN)
  #include "audiorenderer_waveout.h"
  #include "audiocapture_wavein.h"
 #else
  #include "audiorenderer_alsa.h"
  #include "audiocapture_alsa.h"
 #endif
#else
 #include "audiorenderer_qt.h"
 #include "audiocapture_qt.h"
#endif

/* Plugin column structure */
#define KMapColumnPluginName    0
#define KMapColumnDeviceName    1
#define KMapColumnHasInput      2
#define KMapColumnHasOutput     3
#define KMapColumnHasFeedback   4
#define KMapColumnInputLine     5
#define KMapColumnOutputLine    6

#define KAudioColumnDeviceName  0
#define KAudioColumnHasInput    1
#define KAudioColumnHasOutput   2
#define KAudioColumnPrivate     3

/* Profile column structure */
#define KProfileColumnName 0
#define KProfileColumnType 1

InputOutputPatchEditor::InputOutputPatchEditor(QWidget* parent, quint32 universe, InputOutputMap *ioMap, Doc *doc)
    : QWidget(parent)
    , m_ioMap(ioMap)
    , m_doc(doc)
    , m_universe(universe)
    , m_currentInputPluginName(KInputNone)
    , m_currentInput(QLCIOPlugin::invalidLine())
    , m_currentOutputPluginName(KOutputNone)
    , m_currentOutput(QLCIOPlugin::invalidLine())
    , m_currentProfileName(KInputNone)
    , m_currentFeedbackPluginName(KOutputNone)
    , m_currentFeedback(QLCIOPlugin::invalidLine())
{
    Q_ASSERT(universe < m_ioMap->universes());
    Q_ASSERT(ioMap != NULL);

    setupUi(this);

    m_infoBrowser->setOpenExternalLinks(true);
    m_infoBrowser->setFixedHeight(250);

    InputPatch* inputPatch = m_ioMap->inputPatch(universe);
    OutputPatch* outputPatch = m_ioMap->outputPatch(universe);
    OutputPatch* feedbackPatch = m_ioMap->feedbackPatch(universe);

    /* Copy these so they can be applied if the user cancels */
    if (inputPatch != NULL)
    {
        m_currentInputPluginName = inputPatch->pluginName();
        m_currentInput = inputPatch->input();
        m_currentProfileName = inputPatch->profileName();
    }

    if (outputPatch != NULL)
    {
        m_currentOutputPluginName = outputPatch->pluginName();
        m_currentOutput = outputPatch->output();
    }

    if (feedbackPatch != NULL)
    {
        m_currentFeedbackPluginName = feedbackPatch->pluginName();
        m_currentFeedback = feedbackPatch->output();
    }

    m_mapTree->setSortingEnabled(true);
    m_mapTree->sortByColumn(KMapColumnPluginName, Qt::AscendingOrder);

    /* Setup UI controls */
    setupMappingPage();
    setupProfilePage();

    fillAudioTree();

    /* Listen to itemChanged() signals to catch check state changes */
    connect(m_audioMapTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
            this, SLOT(slotAudioDeviceItemChanged(QTreeWidgetItem*, int)));

    /* Select the top-most "None" item */
    m_mapTree->setCurrentItem(m_mapTree->topLevelItem(0));

    /* Listen to plugin configuration changes */
    connect(m_ioMap, SIGNAL(pluginConfigurationChanged(const QString&, bool)),
            this, SLOT(slotPluginConfigurationChanged(const QString&, bool)));
}

InputOutputPatchEditor::~InputOutputPatchEditor()
{
}

/****************************************************************************
 * Mapping page
 ****************************************************************************/

InputPatch* InputOutputPatchEditor::patch() const
{
    InputPatch* p = m_ioMap->inputPatch(m_universe);
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

    /* Double click acts as edit button click */
    connect(m_mapTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotConfigureInputClicked()));
}

void InputOutputPatchEditor::fillMappingTree()
{
    /* Disable check state change tracking when the tree is filled */
    disconnect(m_mapTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
               this, SLOT(slotMapItemChanged(QTreeWidgetItem*, int)));

    m_mapTree->clear();

    qDebug() << "[InputOutputPatchEditor] Fill tree for universe: " << m_universe;

    // Build a complete list of Input/Output plugins
    QStringList IOplugins = m_ioMap->inputPluginNames();
    foreach (QString out, m_ioMap->outputPluginNames())
        if (IOplugins.contains(out) == false)
            IOplugins.append(out);

    // cycle through available plugins
    foreach (QString pluginName, IOplugins)
    {
        quint32 inputId = 0;
        quint32 outputId = 0;
        QStringList inputs = m_ioMap->pluginInputs(pluginName);
        QStringList outputs = m_ioMap->pluginOutputs(pluginName);
        bool hasFeedback = m_ioMap->pluginSupportsFeedback(pluginName);
        QLCIOPlugin *plugin = m_doc->ioPluginCache()->plugin(pluginName);

        // 1st case: this plugin has no input or output
        if (inputs.length() == 0 && outputs.length() == 0)
        {
            QTreeWidgetItem* pitem = new QTreeWidgetItem(m_mapTree);
            pitem->setText(KMapColumnPluginName, pluginName);
            pitem->setText(KMapColumnDeviceName, KInputNone);
            pitem->setText(KMapColumnInputLine, QString("%1").arg(QLCIOPlugin::invalidLine()));
            pitem->setText(KMapColumnOutputLine, QString("%1").arg(QLCIOPlugin::invalidLine()));
        }
        else
        {
            // 2nd case: plugin with an input and maybe an output
            for (int l = 0; l < inputs.length(); l++)
            {
                quint32 uni = m_ioMap->inputMapping(pluginName, inputId);
                //qDebug() << "Plugin: " << pluginName << ", input: " << id << ", universe:" << uni;
                if (uni == InputOutputMap::invalidUniverse() ||
                   (uni == m_universe || plugin->capabilities() & QLCIOPlugin::Infinite))
                {
                    QTreeWidgetItem* pitem = new QTreeWidgetItem(m_mapTree);
                    pitem->setText(KMapColumnPluginName, pluginName);
                    pitem->setText(KMapColumnDeviceName, inputs.at(l));
                    pitem->setFlags(pitem->flags() | Qt::ItemIsUserCheckable);
                    if (m_currentInputPluginName == pluginName && m_currentInput == inputId)
                        pitem->setCheckState(KMapColumnHasInput, Qt::Checked);
                    else
                        pitem->setCheckState(KMapColumnHasInput, Qt::Unchecked);
                    pitem->setTextAlignment(KMapColumnHasInput, Qt::AlignHCenter);
                    pitem->setText(KMapColumnInputLine, QString("%1").arg(inputId));
                    pitem->setText(KMapColumnOutputLine, QString("%1").arg(QLCIOPlugin::invalidLine()));
                    // check if this plugin has also an output
                    if (outputs.contains(inputs.at(l)))
                    {
                        quint32 outUni = m_ioMap->outputMapping(pluginName, outputId);
                        if (outUni == InputOutputMap::invalidUniverse() ||
                           (outUni == m_universe || plugin->capabilities() & QLCIOPlugin::Infinite))
                        {
                            if (m_currentOutputPluginName == pluginName && m_currentOutput == outputId)
                                pitem->setCheckState(KMapColumnHasOutput, Qt::Checked);
                            else
                                pitem->setCheckState(KMapColumnHasOutput, Qt::Unchecked);
                            pitem->setText(KMapColumnOutputLine, QString("%1").arg(outputId));
                            // add feedback
                            if (hasFeedback)
                            {
                                if (m_currentFeedbackPluginName == pluginName && m_currentFeedback == outputId)
                                    pitem->setCheckState(KMapColumnHasFeedback, Qt::Checked);
                                else
                                    pitem->setCheckState(KMapColumnHasFeedback, Qt::Unchecked);
                            }
                        }
                        outputId++;
                    }
                }
                else // input is mapped to different universe, let's check if outputs are available
                {
                    // check if this plugin has also an output
                    if (outputs.contains(inputs.at(l)))
                    {
                        quint32 outUni = m_ioMap->outputMapping(pluginName, outputId);
                        if (outUni == InputOutputMap::invalidUniverse() ||
                           (outUni == m_universe || plugin->capabilities() & QLCIOPlugin::Infinite))
                        {
                            //qDebug() << "Plugin: " << pluginName << ", output: " << id << ", universe:" << outUni;
                            QTreeWidgetItem* pitem = new QTreeWidgetItem(m_mapTree);
                            pitem->setText(KMapColumnPluginName, pluginName);
                            pitem->setText(KMapColumnDeviceName, inputs.at(l));
                            pitem->setFlags(pitem->flags() | Qt::ItemIsUserCheckable);
                            if (m_currentOutputPluginName == pluginName && m_currentOutput == outputId)
                                pitem->setCheckState(KMapColumnHasOutput, Qt::Checked);
                            else
                                pitem->setCheckState(KMapColumnHasOutput, Qt::Unchecked);
                            // add feedback
                            if (hasFeedback)
                            {
                                if (m_currentFeedbackPluginName == pluginName && m_currentFeedback == outputId)
                                    pitem->setCheckState(KMapColumnHasFeedback, Qt::Checked);
                                else
                                    pitem->setCheckState(KMapColumnHasFeedback, Qt::Unchecked);
                            }
                            pitem->setText(KMapColumnOutputLine, QString("%1").arg(outputId));
                            pitem->setText(KMapColumnInputLine, QString("%1").arg(QLCIOPlugin::invalidLine()));
                        }
                        outputId++;
                    }

                }
                inputId++;
            }
            // 3rd case: output only plugins
            for (int o = 0; o < outputs.length(); o++)
            {
                if (inputs.contains(outputs.at(o)) == false)
                {
                    quint32 outUni = m_ioMap->outputMapping(pluginName, outputId);
                    if (outUni == InputOutputMap::invalidUniverse() ||
                        (outUni == m_universe || plugin->capabilities() & QLCIOPlugin::Infinite))
                    {
                        //qDebug() << "Plugin: " << pluginName << ", output: " << id << ", universe:" << outUni;
                        QTreeWidgetItem* pitem = new QTreeWidgetItem(m_mapTree);
                        pitem->setText(KMapColumnPluginName, pluginName);
                        pitem->setText(KMapColumnDeviceName, outputs.at(o));
                        pitem->setFlags(pitem->flags() | Qt::ItemIsUserCheckable);
                        if (m_currentOutputPluginName == pluginName && m_currentOutput == outputId)
                            pitem->setCheckState(KMapColumnHasOutput, Qt::Checked);
                        else
                            pitem->setCheckState(KMapColumnHasOutput, Qt::Unchecked);
                        // add feedback
                        if (hasFeedback)
                        {
                            if (m_currentFeedbackPluginName == pluginName && m_currentFeedback == outputId)
                                pitem->setCheckState(KMapColumnHasFeedback, Qt::Checked);
                            else
                                pitem->setCheckState(KMapColumnHasFeedback, Qt::Unchecked);
                        }
                        pitem->setText(KMapColumnOutputLine, QString("%1").arg(outputId));
                        pitem->setText(KMapColumnInputLine, QString("%1").arg(QLCIOPlugin::invalidLine()));
                    }
                    outputId++;
                }
            }
        }
    }

    m_mapTree->resizeColumnToContents(KMapColumnPluginName);
    m_mapTree->resizeColumnToContents(KMapColumnDeviceName);

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
        info = m_ioMap->inputPluginStatus(QString(), 0);
        info += m_ioMap->outputPluginStatus(QString(), 0);
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

        info = m_ioMap->pluginDescription(plugin);

        //if (input != QLCIOPlugin::invalidLine())
            info += m_ioMap->inputPluginStatus(plugin, input);
        //if (output != QLCIOPlugin::invalidLine())
            info += m_ioMap->outputPluginStatus(plugin, output);
        configurable = m_ioMap->canConfigurePlugin(plugin);
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
            if (m_ioMap->setInputPatch(m_universe, m_currentInputPluginName,
                                 m_currentInput, m_currentProfileName) == false)
                showPluginMappingError();
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
                m_currentOutput = item->text(KMapColumnOutputLine).toUInt();

                /* Apply the patch immediately */
                if (m_ioMap->setOutputPatch(m_universe, m_currentOutputPluginName, m_currentOutput, false) == false)
                    showPluginMappingError();
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
                m_currentFeedback = item->text(KMapColumnOutputLine).toUInt();

                /* Apply the patch immediately */
                if (m_ioMap->setOutputPatch(m_universe, m_currentFeedbackPluginName, m_currentFeedback, true) == false)
                    showPluginMappingError();
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

            if (m_ioMap->setInputPatch(m_universe, m_currentInputPluginName, m_currentInput) == false)
                showPluginMappingError();
        }
        else if (col == KMapColumnHasOutput)
        {
            m_currentOutputPluginName = KInputNone;
            m_currentOutput = QLCIOPlugin::invalidLine();

            /* Apply the patch immediately */
            if (m_ioMap->setOutputPatch(m_universe, m_currentOutputPluginName, m_currentOutput, false) == false)
                showPluginMappingError();
        }
        else if (col == KMapColumnHasFeedback)
        {
            m_currentFeedbackPluginName = KInputNone;
            m_currentFeedback = QLCIOPlugin::invalidLine();

            /* Apply the patch immediately */
            if (m_ioMap->setOutputPatch(m_universe, m_currentFeedbackPluginName, m_currentFeedback, true) == false)
                showPluginMappingError();
        }
    }

    slotMapCurrentItemChanged(item);

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
    m_ioMap->configurePlugin(plugin);
}

void InputOutputPatchEditor::slotPluginConfigurationChanged(const QString& pluginName, bool success)
{
    if (success == false)
    {
        showPluginMappingError();
        return;
    }

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

void InputOutputPatchEditor::showPluginMappingError()
{
    QMessageBox::critical(this, tr("Error"),
                          tr("An error occurred while trying to open the selected device line.\n"
                             "This can be caused either by a wrong system configuration or "
                             "an unsupported input/output mode.\n"
                             "Please refer to the plugins documentation to troubleshoot this."),
                          QMessageBox::Close);
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
    QStringListIterator it(m_ioMap->profileNames());
    while (it.hasNext() == true)
    {
        item = new QTreeWidgetItem(m_profileTree);
        updateProfileItem(it.next(), item);
    }
    m_profileTree->resizeColumnToContents(KProfileColumnName);
}

void InputOutputPatchEditor::updateProfileItem(const QString& name, QTreeWidgetItem* item)
{
    Q_ASSERT(item != NULL);

    item->setText(KProfileColumnName, name);
    QLCInputProfile * prof = m_ioMap->profile(name);
    if (prof)
    {
        item->setText(KProfileColumnType, QLCInputProfile::typeToString(prof->type()));
    }

    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    if (m_currentProfileName == name)
        item->setCheckState(KProfileColumnName, Qt::Checked);
    else
        item->setCheckState(KProfileColumnName, Qt::Unchecked);
}

QString InputOutputPatchEditor::fullProfilePath(const QString& manufacturer,
                                          const QString& model) const
{
    QDir dir(InputOutputMap::userProfileDirectory());
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
    if (m_ioMap->setInputPatch(m_universe, m_currentInputPluginName,
                               m_currentInput, m_currentProfileName) == false)
        showPluginMappingError();

    emit mappingChanged();
}

void InputOutputPatchEditor::slotAddProfileClicked()
{
    /* Create a new input profile and start editing it */
    InputProfileEditor ite(this, NULL, m_ioMap);
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
            m_ioMap->addProfile(profile);

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
    profile = m_ioMap->profile(name);
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
            m_ioMap->removeProfile(name);
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
    profile = m_ioMap->profile(name);
    if (profile == NULL)
        return;

    /* Edit the profile and update the item if OK was pressed */
    InputProfileEditor ite(this, profile, m_ioMap);
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

/****************************************************************************
 * Audio tree
 ****************************************************************************/

void InputOutputPatchEditor::fillAudioTree()
{
    QList<AudioDeviceInfo> devList;
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
 #if defined( __APPLE__) || defined(Q_OS_MAC)
    devList = AudioRendererPortAudio::getDevicesInfo();
 #elif defined(WIN32) || defined(Q_OS_WIN)
    devList = AudioRendererWaveOut::getDevicesInfo();
 #else
    devList = AudioRendererAlsa::getDevicesInfo();
 #endif
#else
    devList = AudioRendererQt::getDevicesInfo();
#endif

    m_audioMapTree->clear();
    QSettings settings;
    QString inputName, outputName;
    bool inputFound = false, outputFound = false;

    QTreeWidgetItem* defItem = new QTreeWidgetItem(m_audioMapTree);
    defItem->setText(KAudioColumnDeviceName, tr("Default device"));
    defItem->setText(KAudioColumnPrivate, "__qlcplusdefault__");

    QVariant var = settings.value(SETTINGS_AUDIO_INPUT_DEVICE);
    if (var.isValid() == true)
        inputName = var.toString();

    var = settings.value(SETTINGS_AUDIO_OUTPUT_DEVICE);
    if (var.isValid() == true)
        outputName = var.toString();

    foreach( AudioDeviceInfo info, devList)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(m_audioMapTree);
        item->setText(KAudioColumnDeviceName, info.deviceName);
        item->setText(KAudioColumnPrivate, info.privateName);

        if (info.capabilities & AUDIO_CAP_INPUT)
        {
            if (info.privateName == inputName)
            {
                item->setCheckState(KAudioColumnHasInput, Qt::Checked);
                inputFound = true;
            }
            else
                item->setCheckState(KAudioColumnHasInput, Qt::Unchecked);
        }
        if (info.capabilities & AUDIO_CAP_OUTPUT)
        {
            if (info.privateName == outputName)
            {
                item->setCheckState(KAudioColumnHasOutput, Qt::Checked);
                outputFound = true;
            }
            else
                item->setCheckState(KAudioColumnHasOutput, Qt::Unchecked);
        }
    }

    if (inputFound == true)
        defItem->setCheckState(KAudioColumnHasInput, Qt::Unchecked);
    else
        defItem->setCheckState(KAudioColumnHasInput, Qt::Checked);

    if (outputFound == true)
        defItem->setCheckState(KAudioColumnHasOutput, Qt::Unchecked);
    else
        defItem->setCheckState(KAudioColumnHasOutput, Qt::Checked);

    m_audioMapTree->resizeColumnToContents(KAudioColumnDeviceName);
}

void InputOutputPatchEditor::slotAudioDeviceItemChanged(QTreeWidgetItem *item, int col)
{
    if (item == NULL)
        return;

    /* Temporarily disable this signal to prevent an endless loop */
    disconnect(m_audioMapTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
               this, SLOT(slotAudioDeviceItemChanged(QTreeWidgetItem*, int)));

    QSettings settings;

    if (item->checkState(col) == Qt::Checked)
    {
        /* Set all other items unchecked... */
        QTreeWidgetItemIterator it(m_audioMapTree);
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

        if (col == KAudioColumnHasInput)
        {
            if (item == m_audioMapTree->topLevelItem(0))
                settings.remove(SETTINGS_AUDIO_INPUT_DEVICE);
            else
                settings.setValue(SETTINGS_AUDIO_INPUT_DEVICE, QVariant(item->text(KAudioColumnPrivate)));
            emit audioInputDeviceChanged();
        }
        else if (col == KAudioColumnHasOutput)
        {
            if (item == m_audioMapTree->topLevelItem(0))
                settings.remove(SETTINGS_AUDIO_OUTPUT_DEVICE);
            else
                settings.setValue(SETTINGS_AUDIO_OUTPUT_DEVICE, QVariant(item->text(KAudioColumnPrivate)));
        }
    }
    else
    {
        QTreeWidgetItem* defItem = m_audioMapTree->topLevelItem(0);

        if (col == KAudioColumnHasInput)
        {
            settings.remove(SETTINGS_AUDIO_INPUT_DEVICE);
            defItem->setCheckState(KAudioColumnHasInput, Qt::Checked);
            emit audioInputDeviceChanged();
        }
        else if (col == KAudioColumnHasOutput)
        {
            settings.remove(SETTINGS_AUDIO_OUTPUT_DEVICE);
            defItem->setCheckState(KAudioColumnHasOutput, Qt::Checked);
        }
    }

    /* Start listening to this signal once again */
    connect(m_audioMapTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
            this, SLOT(slotAudioDeviceItemChanged(QTreeWidgetItem*, int)));
}
