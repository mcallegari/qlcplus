/*
  Q Light Controller Plus
  configureosc.cpp

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

#include <QTreeWidgetItem>
#include <QMessageBox>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QDebug>

#include "configureosc.h"
#include "oscplugin.h"

#define KMapColumnInterface     0
#define KMapColumnUniverse      1
#define KMapColumnInputPort     2
#define KMapColumnOutputAddress 3
#define KMapColumnOutputPort    4

#define PROP_UNIVERSE (Qt::UserRole + 0)
#define PROP_LINE (Qt::UserRole + 1)
#define PROP_TYPE (Qt::UserRole + 2)

/*****************************************************************************
 * Initialization
 *****************************************************************************/

ConfigureOSC::ConfigureOSC(OSCPlugin* plugin, QWidget* parent)
        : QDialog(parent)
{
    Q_ASSERT(plugin != NULL);
    m_plugin = plugin;

    /* Setup UI controls */
    setupUi(this);

    connect(m_oscPathEdit, SIGNAL(textChanged(QString)),
            this, SLOT(slotOSCPathChanged(QString)));

    fillMappingTree();
}

ConfigureOSC::~ConfigureOSC()
{
}

void ConfigureOSC::fillMappingTree()
{
    QTreeWidgetItem* inputItem = NULL;
    QTreeWidgetItem* outputItem = NULL;

    QList<OSCIO> IOmap = m_plugin->getIOMapping();
    foreach(OSCIO io, IOmap)
    {
        if (io.controller == NULL)
            continue;

        OSCController *controller = io.controller;
        if (controller == NULL)
            continue;

        qDebug() << "[ArtNet] controller IP" << controller->getNetworkIP().toString() << "type:" << controller->type();
        if ((controller->type() & OSCController::Input) && inputItem == NULL)
        {
            inputItem = new QTreeWidgetItem(m_uniMapTree);
            inputItem->setText(KMapColumnInterface, tr("Inputs"));
            inputItem->setExpanded(true);
        }
        if ((controller->type() & OSCController::Output) && outputItem == NULL)
        {
            outputItem = new QTreeWidgetItem(m_uniMapTree);
            outputItem->setText(KMapColumnInterface, tr("Outputs"));
            outputItem->setExpanded(true);
        }
        foreach(quint32 universe, controller->universesList())
        {
            UniverseInfo *info = controller->getUniverseInfo(universe);
            QString networkIP = controller->getNetworkIP().toString();
            QString baseIP = networkIP.mid(0, networkIP.lastIndexOf(".") + 1);
            baseIP.append("1");

            if (info->type & OSCController::Input)
            {
                QTreeWidgetItem *item = new QTreeWidgetItem(inputItem);
                item->setData(KMapColumnInterface, PROP_UNIVERSE, universe);
                item->setData(KMapColumnInterface, PROP_LINE, controller->line());
                item->setData(KMapColumnInterface, PROP_TYPE, OSCController::Input);
                item->setText(KMapColumnInterface, networkIP);
                item->setText(KMapColumnUniverse, QString::number(universe + 1));

                QSpinBox *inSpin = new QSpinBox(this);
                inSpin->setRange(1, 65535);
                inSpin->setValue(info->inputPort);
                m_uniMapTree->setItemWidget(item, KMapColumnInputPort, inSpin);

                if (controller->getNetworkIP() == QHostAddress::LocalHost)
                {
                    // localhost (127.0.0.1) does not need configuration
                    item->setText(KMapColumnOutputAddress, info->feedbackAddress.toString());
                }
                else
                {
                    QWidget *IPwidget;
                    if (info->feedbackAddress == QHostAddress::Null)
                        IPwidget = new QLineEdit(baseIP);
                    else
                        IPwidget = new QLineEdit(info->feedbackAddress.toString());
                    m_uniMapTree->setItemWidget(item, KMapColumnOutputAddress, IPwidget);
                }

                QSpinBox *outSpin = new QSpinBox(this);
                outSpin->setRange(1, 65535);
                outSpin->setValue(info->feedbackPort);
                m_uniMapTree->setItemWidget(item, KMapColumnOutputPort, outSpin);
            }
            if (info->type & OSCController::Output)
            {
                QTreeWidgetItem *item = new QTreeWidgetItem(outputItem);
                item->setData(KMapColumnInterface, PROP_UNIVERSE, universe);
                item->setData(KMapColumnInterface, PROP_LINE, controller->line());
                item->setData(KMapColumnInterface, PROP_TYPE, OSCController::Output);

                item->setText(KMapColumnInterface, networkIP);
                item->setText(KMapColumnUniverse, QString::number(universe + 1));

                if (controller->getNetworkIP() == QHostAddress::LocalHost)
                {
                    // localhost (127.0.0.1) does not need configuration
                    item->setText(KMapColumnOutputAddress, info->outputAddress.toString());
                }
                else
                {
                    QWidget *IPwidget;
                    if (info->outputAddress == QHostAddress::Null)
                        IPwidget = new QLineEdit(baseIP);
                    else
                        IPwidget = new QLineEdit(info->outputAddress.toString());
                    m_uniMapTree->setItemWidget(item, KMapColumnOutputAddress, IPwidget);
                }

                QSpinBox *spin = new QSpinBox(this);
                spin->setRange(1, 65535);
                spin->setValue(info->outputPort);
                m_uniMapTree->setItemWidget(item, KMapColumnOutputPort, spin);
            }
        }
    }

    m_uniMapTree->header()->resizeSections(QHeaderView::ResizeToContents);
}

void ConfigureOSC::showIPAlert(QString ip)
{
    QMessageBox::critical(this, tr("Invalid IP"), tr("%1 is not a valid IP.\nPlease fix it before confirming.").arg(ip));
}

/*****************************************************************************
 * Dialog actions
 *****************************************************************************/

void ConfigureOSC::accept()
{
    for(int i = 0; i < m_uniMapTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *topItem = m_uniMapTree->topLevelItem(i);
        for(int c = 0; c < topItem->childCount(); c++)
        {
            QTreeWidgetItem *item = topItem->child(c);
            if (item->data(KMapColumnInterface, PROP_UNIVERSE).isValid() == false)
                continue;

            quint32 universe = item->data(KMapColumnInterface, PROP_UNIVERSE).toUInt();
            quint32 line = item->data(KMapColumnInterface, PROP_LINE).toUInt();
            OSCController::Type type = OSCController::Type(item->data(KMapColumnInterface, PROP_TYPE).toInt());
            QLCIOPlugin::Capability cap = QLCIOPlugin::Input;
            if (type == OSCController::Output)
                cap = QLCIOPlugin::Output;

            QSpinBox *inSpin = qobject_cast<QSpinBox*>(m_uniMapTree->itemWidget(item, KMapColumnInputPort));
            if (inSpin != NULL)
                m_plugin->setParameter(universe, line, cap, OSC_INPUTPORT, inSpin->value());

            QLineEdit *ipEdit = qobject_cast<QLineEdit*>(m_uniMapTree->itemWidget(item, KMapColumnOutputAddress));
            if (ipEdit != NULL)
            {
                QHostAddress newHostAddress(ipEdit->text());
                if (newHostAddress.isNull() && ipEdit->text().size() > 0)
                {
                    showIPAlert(ipEdit->text());
                    return;
                }

                if (type == OSCController::Input)
                    m_plugin->setParameter(universe, line, QLCIOPlugin::Output, OSC_FEEDBACKIP, newHostAddress.toString());
                else
                    m_plugin->setParameter(universe, line, cap, OSC_OUTPUTIP, newHostAddress.toString());
            }

            QSpinBox *outSpin = qobject_cast<QSpinBox*>(m_uniMapTree->itemWidget(item, KMapColumnOutputPort));
            if (outSpin != NULL)
            {
                if (type == OSCController::Input)
                    m_plugin->setParameter(universe, line, QLCIOPlugin::Output, OSC_FEEDBACKPORT, outSpin->value());
                else
                    m_plugin->setParameter(universe, line, cap, OSC_OUTPUTPORT, outSpin->value());
            }
        }
    }

    QDialog::accept();
}

void ConfigureOSC::slotOSCPathChanged(QString path)
{
    m_chNumSpin->setValue(qChecksum(path.toUtf8().data(), path.length()));
}

int ConfigureOSC::exec()
{
    return QDialog::exec();
}

