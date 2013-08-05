/*
  Q Light Controller
  configuremidiplugin.cpp

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
#include <QComboBox>
#include <QSpinBox>

#include "configuremidiplugin.h"
#include "midioutputdevice.h"
#include "midiinputdevice.h"
#include "midienumerator.h"
#include "midiprotocol.h"
#include "mididevice.h"
#include "midiplugin.h"

#define PROP_DEV    "dev"
#define COL_NAME    0
#define COL_CHANNEL 1
#define COL_MODE    2

ConfigureMidiPlugin::ConfigureMidiPlugin(MidiPlugin* plugin, QWidget* parent)
    : QDialog(parent)
    , m_plugin(plugin)
{
    Q_ASSERT(plugin != NULL);
    setupUi(this);
    m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);

    connect(plugin, SIGNAL(configurationChanged()), this, SLOT(slotUpdateTree()));
    slotUpdateTree();
}

ConfigureMidiPlugin::~ConfigureMidiPlugin()
{
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
    }

    outputs->setExpanded(true);
    inputs->setExpanded(true);
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
