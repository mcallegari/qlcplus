/*
  Q Light Controller
  configuremidiplugin.cpp

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
#include <QTreeWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QDebug>
#include <QSettings>

#include "configuremidiplugin.h"
#include "midioutputdevice.h"
#include "midiinputdevice.h"
#include "midienumerator.h"
#include "midiprotocol.h"
#include "mididevice.h"
#include "midiplugin.h"

#define PROP_DEV        "dev"
#define COL_NAME        0
#define COL_CHANNEL     1
#define COL_MODE        2
#define COL_INITMESSAGE 3

#define SETTINGS_GEOMETRY "configuremidiplugin/geometry"

ConfigureMidiPlugin::ConfigureMidiPlugin(MidiPlugin* plugin, QWidget* parent)
    : QDialog(parent)
    , m_plugin(plugin)
{
    Q_ASSERT(plugin != NULL);
    setupUi(this);

    QSettings settings;
    QVariant geometrySettings = settings.value(SETTINGS_GEOMETRY);
    if (geometrySettings.isValid() == true)
        restoreGeometry(geometrySettings.toByteArray());

    connect(plugin, SIGNAL(configurationChanged()), this, SLOT(slotUpdateTree()));
    slotUpdateTree();
}

ConfigureMidiPlugin::~ConfigureMidiPlugin()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

void ConfigureMidiPlugin::slotRefresh()
{
    m_plugin->m_enumerator->rescan();
}

void ConfigureMidiPlugin::slotMidiChannelValueChanged(int value)
{
    QWidget* widget = qobject_cast<QWidget*> (QObject::sender());
    Q_ASSERT(widget != NULL);

    QVariant var = widget->property(PROP_DEV);
    Q_ASSERT(var.isValid() == true);

    MidiDevice* dev = (MidiDevice*) var.toULongLong();
    Q_ASSERT(dev != NULL);

    // Zero is a special value for OMNI mode
    if (value == 0)
        dev->setMidiChannel(MAX_MIDI_CHANNELS);
    else
        dev->setMidiChannel(value - 1);
}

void ConfigureMidiPlugin::slotModeActivated(int index)
{
    QComboBox* combo = qobject_cast<QComboBox*> (QObject::sender());
    Q_ASSERT(combo != NULL);

    QVariant var = combo->property(PROP_DEV);
    Q_ASSERT(var.isValid() == true);

    MidiDevice* dev = (MidiDevice*) var.toULongLong();
    Q_ASSERT(dev != NULL);

    MidiDevice::Mode mode = (MidiDevice::Mode) combo->itemData(index).toInt();
    dev->setMode(mode);
}

void ConfigureMidiPlugin::slotInitMessageActivated(int index)
{
    QComboBox* combo = qobject_cast<QComboBox*> (QObject::sender());
    Q_ASSERT(combo != NULL);

    QVariant var = combo->property(PROP_DEV);
    Q_ASSERT(var.isValid() == true);

    MidiDevice* dev = (MidiDevice*) var.toULongLong();
    Q_ASSERT(dev != NULL);

    QString initMessage = combo->itemText(index);
    dev->setMidiTemplateName(initMessage);
}

void ConfigureMidiPlugin::slotInitMessageChanged(QString midiTemplateName)
{
    QComboBox* combo = qobject_cast<QComboBox*> (QObject::sender());
    Q_ASSERT(combo != NULL);

    QVariant var = combo->property(PROP_DEV);
    Q_ASSERT(var.isValid() == true);

    MidiDevice* dev = (MidiDevice*) var.toULongLong();
    Q_ASSERT(dev != NULL);
    dev->setMidiTemplateName(midiTemplateName);
}


void ConfigureMidiPlugin::slotUpdateTree()
{
    m_tree->clear();

    QTreeWidgetItem* outputs = new QTreeWidgetItem(m_tree);
    outputs->setText(COL_NAME, tr("Outputs"));

    QListIterator <MidiOutputDevice*> oit(m_plugin->m_enumerator->outputDevices());
    while (oit.hasNext() == true)
    {
        MidiOutputDevice* dev(oit.next());
        QTreeWidgetItem* item = new QTreeWidgetItem(outputs);
        item->setText(COL_NAME, dev->name());

        QWidget* widget = createMidiChannelWidget(dev->midiChannel());
        widget->setProperty(PROP_DEV, (qulonglong) dev);
        m_tree->setItemWidget(item, COL_CHANNEL, widget);

        widget = createModeWidget(dev->mode());
        widget->setProperty(PROP_DEV, (qulonglong) dev);
        m_tree->setItemWidget(item, COL_MODE, widget);

        widget = createInitMessageWidget(dev->midiTemplateName());
        widget->setProperty(PROP_DEV, (qulonglong) dev);
        m_tree->setItemWidget(item, COL_INITMESSAGE, widget);
    }

    QTreeWidgetItem* inputs = new QTreeWidgetItem(m_tree);
    inputs->setText(COL_NAME, tr("Inputs"));

    QListIterator <MidiInputDevice*> iit(m_plugin->m_enumerator->inputDevices());
    while (iit.hasNext() == true)
    {
        MidiInputDevice* dev(iit.next());
        QTreeWidgetItem* item = new QTreeWidgetItem(inputs);
        item->setText(COL_NAME, dev->name());

        QWidget* widget = createMidiChannelWidget(dev->midiChannel());
        widget->setProperty(PROP_DEV, (qulonglong) dev);
        m_tree->setItemWidget(item, COL_CHANNEL, widget);

        widget = createModeWidget(dev->mode());
        widget->setProperty(PROP_DEV, (qulonglong) dev);
        m_tree->setItemWidget(item, COL_MODE, widget);

        widget = createInitMessageWidget(dev->midiTemplateName());
        widget->setProperty(PROP_DEV, (qulonglong) dev);
        m_tree->setItemWidget(item, COL_INITMESSAGE, widget);
    }

    outputs->setExpanded(true);
    inputs->setExpanded(true);

    m_tree->resizeColumnToContents(COL_NAME);
    m_tree->resizeColumnToContents(COL_CHANNEL);
}

QWidget* ConfigureMidiPlugin::createMidiChannelWidget(int select)
{
    QSpinBox* spin = new QSpinBox;
    spin->setRange(0, MAX_MIDI_CHANNELS);
    spin->setSpecialValueText(QString("1-16"));
    if (select >= MAX_MIDI_CHANNELS)
        spin->setValue(0);
    else
        spin->setValue(select + 1);
    connect(spin, SIGNAL(valueChanged(int)), this, SLOT(slotMidiChannelValueChanged(int)));
    return spin;
}

QWidget* ConfigureMidiPlugin::createModeWidget(MidiDevice::Mode mode)
{
    QComboBox* combo = new QComboBox;
    combo->addItem(MidiDevice::modeToString(MidiDevice::Note), MidiDevice::Note);
    combo->addItem(MidiDevice::modeToString(MidiDevice::ControlChange), MidiDevice::ControlChange);
    combo->addItem(MidiDevice::modeToString(MidiDevice::ProgramChange), MidiDevice::ProgramChange);

    if (mode == MidiDevice::ControlChange)
        combo->setCurrentIndex(1);
    else if (mode == MidiDevice::ProgramChange)
        combo->setCurrentIndex(2);
    else
        combo->setCurrentIndex(0);

    connect(combo, SIGNAL(activated(int)), this, SLOT(slotModeActivated(int)));

    return combo;
}

QWidget* ConfigureMidiPlugin::createInitMessageWidget(QString midiTemplateName)
{
    QComboBox* combo = new QComboBox;
    combo->addItem(tr("None"), "");

    QListIterator <MidiTemplate*> it(m_plugin->midiTemplates());
    while (it.hasNext() == true)
    {
        MidiTemplate* templ = it.next();
        combo->addItem(templ->name(), templ->initMessage());
        //qDebug() << "msg: " << templ->initMessage();
    }

    for (int i = 0; i < combo->count(); ++i)
    {
        if (combo->itemText(i) == midiTemplateName)
            combo->setCurrentIndex(i);
    }

    //combo->setEditable(true);
    qDebug() << "[MIDI] Selected template: " << midiTemplateName;

    connect(combo, SIGNAL(activated(int)), this, SLOT(slotInitMessageActivated(int)));
    connect(combo, SIGNAL(editTextChanged(QString)), this, SLOT(slotInitMessageChanged(QString)));

    return combo;
}
