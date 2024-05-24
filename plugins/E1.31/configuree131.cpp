/*
  Q Light Controller Plus
  configuree131.cpp

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
#include <QSettings>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QString>
#include <QDebug>

#include "configuree131.h"
#include "e131plugin.h"
#include "e131packetizer.h"

#define KMapColumnInterface     0
#define KMapColumnUniverse      1
#define KMapColumnMulticast     2
#define KMapColumnIPAddress     3
#define KMapColumnPort          4
#define KMapColumnE131Uni       5
#define KMapColumnTransmitMode  6
#define KMapColumnPriority      7

#define PROP_UNIVERSE (Qt::UserRole + 0)
#define PROP_LINE (Qt::UserRole + 1)
#define PROP_TYPE (Qt::UserRole + 2)

#define E131_PRIORITY_MIN 0
#define E131_PRIORITY_MAX 200

#define SETTINGS_GEOMETRY "conifguree131/geometry"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

ConfigureE131::ConfigureE131(E131Plugin* plugin, QWidget* parent)
        : QDialog(parent)
{
    Q_ASSERT(plugin != NULL);
    m_plugin = plugin;

    /* Setup UI controls */
    setupUi(this);

    fillMappingTree();

    QSettings settings;
    QVariant value = settings.value(SETTINGS_IFACE_WAIT_TIME);
    if (value.isValid() == true)
        m_waitReadySpin->setValue(value.toInt());
    QVariant geometrySettings = settings.value(SETTINGS_GEOMETRY);
    if (geometrySettings.isValid() == true)
        restoreGeometry(geometrySettings.toByteArray());
}

ConfigureE131::~ConfigureE131()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

void ConfigureE131::fillMappingTree()
{
    QTreeWidgetItem* inputItem = NULL;
    QTreeWidgetItem* outputItem = NULL;

    QList<E131IO> IOmap = m_plugin->getIOMapping();
    foreach (E131IO io, IOmap)
    {
        E131Controller *controller = io.controller;
        if (controller == NULL)
            continue;

        qDebug() << "[E1.31] controller IP" << controller->getNetworkIP() << "type:" << controller->type();
        if ((controller->type() & E131Controller::Input) && inputItem == NULL)
        {
            inputItem = new QTreeWidgetItem(m_uniMapTree);
            inputItem->setText(KMapColumnInterface, tr("Inputs"));
            inputItem->setExpanded(true);
        }
        if ((controller->type() & E131Controller::Output) && outputItem == NULL)
        {
            outputItem = new QTreeWidgetItem(m_uniMapTree);
            outputItem->setText(KMapColumnInterface, tr("Outputs"));
            outputItem->setExpanded(true);
        }
        foreach (quint32 universe, controller->universesList())
        {
            UniverseInfo *info = controller->getUniverseInfo(universe);
            qDebug() << Q_FUNC_INFO << "uni" << universe << "type" << info->type;

            if (info->type & E131Controller::Input)
            {
                QTreeWidgetItem *item = new QTreeWidgetItem(inputItem);
                item->setData(KMapColumnInterface, PROP_UNIVERSE, universe);
                item->setData(KMapColumnInterface, PROP_LINE, controller->line());
                item->setData(KMapColumnInterface, PROP_TYPE, E131Controller::Input);

                item->setText(KMapColumnInterface, controller->getNetworkIP());
                item->setText(KMapColumnUniverse, QString::number(universe + 1));
                item->setTextAlignment(KMapColumnUniverse, Qt::AlignHCenter | Qt::AlignVCenter);

                QCheckBox* multicastCb = new QCheckBox();
                if (info->inputMulticast)
                {
                    multicastCb->setChecked(true);
                    m_uniMapTree->setItemWidget(item, KMapColumnIPAddress,
                            createMcastIPWidget(info->inputMcastAddress.toString()));
                    item->setText(KMapColumnPort, QString("%1").arg(E131_DEFAULT_PORT));
                }
                else
                {
                    multicastCb->setChecked(false);
                    item->setText(KMapColumnIPAddress, controller->getNetworkIP());
                    QSpinBox* portSpin = new QSpinBox(this);
                    portSpin->setRange(0, 0xffff);
                    portSpin->setValue(info->inputUcastPort);
                    m_uniMapTree->setItemWidget(item, KMapColumnPort, portSpin);
                }
                connect(multicastCb, SIGNAL(clicked()), this, SLOT(slotMulticastCheckboxClicked()));
                m_uniMapTree->setItemWidget(item, KMapColumnMulticast, multicastCb);

                QSpinBox *universeSpin = new QSpinBox(this);
                universeSpin->setRange(1, 63999);
                universeSpin->setValue(info->inputUniverse);
                m_uniMapTree->setItemWidget(item, KMapColumnE131Uni, universeSpin);
            }
            if (info->type & E131Controller::Output)
            {
                QTreeWidgetItem *item = new QTreeWidgetItem(outputItem);
                item->setData(KMapColumnInterface, PROP_UNIVERSE, universe);
                item->setData(KMapColumnInterface, PROP_LINE, controller->line());
                item->setData(KMapColumnInterface, PROP_TYPE, E131Controller::Output);

                item->setText(KMapColumnInterface, controller->getNetworkIP());
                item->setText(KMapColumnUniverse, QString::number(universe + 1));
                item->setTextAlignment(KMapColumnUniverse, Qt::AlignHCenter | Qt::AlignVCenter);

                QCheckBox* multicastCb = new QCheckBox(this);
                if (info->outputMulticast)
                {
                    multicastCb->setChecked(true);
                    m_uniMapTree->setItemWidget(item, KMapColumnIPAddress,
                            createMcastIPWidget(info->outputMcastAddress.toString()));
                    item->setText(KMapColumnPort, QString("%1").arg(E131_DEFAULT_PORT));
                }
                else
                {
                    multicastCb->setChecked(false);
                    m_uniMapTree->setItemWidget(item, KMapColumnIPAddress,
                            new QLineEdit(info->outputUcastAddress.toString()));
                    if (QHostAddress(controller->getNetworkIP()) == QHostAddress::LocalHost)
                        m_uniMapTree->itemWidget(item, KMapColumnIPAddress)->setEnabled(false);
                    QSpinBox* portSpin = new QSpinBox(this);
                    portSpin->setRange(0, 0xffff);
                    portSpin->setValue(info->outputUcastPort);
                    m_uniMapTree->setItemWidget(item, KMapColumnPort, portSpin);
                }
                connect(multicastCb, SIGNAL(clicked()), this, SLOT(slotMulticastCheckboxClicked()));
                m_uniMapTree->setItemWidget(item, KMapColumnMulticast, multicastCb);

                QSpinBox *universeSpin = new QSpinBox(this);
                universeSpin->setRange(1, 63999);
                universeSpin->setValue(info->outputUniverse);
                m_uniMapTree->setItemWidget(item, KMapColumnE131Uni, universeSpin);

                QComboBox *transCombo = new QComboBox(this);
                transCombo->addItem(tr("Full"));
                transCombo->addItem(tr("Partial"));
                if (info->outputTransmissionMode == E131Controller::Partial)
                    transCombo->setCurrentIndex(1);
                m_uniMapTree->setItemWidget(item, KMapColumnTransmitMode, transCombo);

                QSpinBox *prioritySpin = new QSpinBox(this);
                prioritySpin->setRange(E131_PRIORITY_MIN, E131_PRIORITY_MAX);
                prioritySpin->setValue(info->outputPriority);
                prioritySpin->setToolTip(tr("%1 - min, %2 - default, %3 - max").arg(E131_PRIORITY_MIN).arg(E131_PRIORITY_DEFAULT).arg(E131_PRIORITY_MAX));
                m_uniMapTree->setItemWidget(item, KMapColumnPriority, prioritySpin);
            }
        }
    }

    m_uniMapTree->header()->resizeSections(QHeaderView::ResizeToContents);
}

QWidget *ConfigureE131::createMcastIPWidget(QString ip)
{
    QWidget* widget = new QWidget(this);
    widget->setLayout(new QHBoxLayout);
    widget->layout()->setContentsMargins(0, 0, 0, 0);

    QStringList ipNibbles = ip.split(".");

    QLabel *label1 = new QLabel(QString("%1.%2.").arg(ipNibbles.at(0)).arg(ipNibbles.at(1)), this);

    QSpinBox *spin1 = new QSpinBox(this);
    spin1->setRange(0, 255);
    spin1->setValue(ipNibbles.at(2).toInt());

    QLabel *label2 = new QLabel(".");

    QSpinBox *spin2 = new QSpinBox(this);
    spin2->setRange(1, 255);
    spin2->setValue(ipNibbles.at(3).toInt());

    widget->layout()->addWidget(label1);
    widget->layout()->addWidget(spin1);
    widget->layout()->addWidget(label2);
    widget->layout()->addWidget(spin2);

    return widget;
}

QString ConfigureE131::getIPAddress(QWidget *ipw)
{
    QSpinBox *ip1Spin = qobject_cast<QSpinBox*>(ipw->layout()->itemAt(1)->widget());
    QSpinBox *ip2Spin = qobject_cast<QSpinBox*>(ipw->layout()->itemAt(3)->widget());

    return QString("239.255.%1.%2").arg(ip1Spin->value()).arg(ip2Spin->value());
}

void ConfigureE131::showIPAlert(QString ip)
{
    QMessageBox::critical(this, tr("Invalid IP"), tr("%1 is not a valid IP.\nPlease fix it before confirming.").arg(ip));
}

void ConfigureE131::slotMulticastCheckboxClicked()
{
    QCheckBox* checkBox = qobject_cast<QCheckBox*>(sender());
    Q_ASSERT(checkBox != NULL);

    for (QTreeWidgetItem* item = m_uniMapTree->topLevelItem(0); item != NULL;
            item = m_uniMapTree->itemBelow(item))
    {
        QCheckBox* cb = qobject_cast<QCheckBox*>(m_uniMapTree->itemWidget(item, KMapColumnMulticast));
        if (checkBox == cb)
        {
            quint32 universe = item->data(KMapColumnInterface, PROP_UNIVERSE).toUInt();
            quint32 line = item->data(KMapColumnInterface, PROP_LINE).toUInt();
            E131Controller::Type type = E131Controller::Type(item->data(KMapColumnInterface, PROP_TYPE).toUInt());

            qDebug() << Q_FUNC_INFO << "uni" << universe << "line" << line << "type" << type;

            E131Controller* controller = m_plugin->getIOMapping().at(line).controller;
            Q_ASSERT(controller != NULL);
            UniverseInfo* info = controller->getUniverseInfo(universe);
            Q_ASSERT(info != NULL);

            if (type == E131Controller::Input)
            {
                if (checkBox->isChecked())
                {
                    item->setText(KMapColumnIPAddress, "");
                    m_uniMapTree->setItemWidget(item, KMapColumnPort, NULL);

                    m_uniMapTree->setItemWidget(item, KMapColumnIPAddress,
                            createMcastIPWidget(info->inputMcastAddress.toString()));
                    item->setText(KMapColumnPort, QString("%1").arg(E131_DEFAULT_PORT));
                }
                else
                {
                    m_uniMapTree->setItemWidget(item, KMapColumnIPAddress, NULL);
                    item->setText(KMapColumnPort, "");

                    item->setText(KMapColumnIPAddress, controller->getNetworkIP());
                    QSpinBox* portSpin = new QSpinBox(this);
                    portSpin->setRange(0, 0xffff);
                    portSpin->setValue(info->inputUcastPort);
                    m_uniMapTree->setItemWidget(item, KMapColumnPort, portSpin);
                }
            }
            else if (type == E131Controller::Output)
            {
                if (checkBox->isChecked())
                {
                    m_uniMapTree->setItemWidget(item, KMapColumnIPAddress, NULL);
                    m_uniMapTree->setItemWidget(item, KMapColumnPort, NULL);

                    m_uniMapTree->setItemWidget(item, KMapColumnIPAddress,
                            createMcastIPWidget(info->outputMcastAddress.toString()));
                    item->setText(KMapColumnPort, QString("%1").arg(E131_DEFAULT_PORT));
                }
                else
                {
                    m_uniMapTree->setItemWidget(item, KMapColumnIPAddress, NULL);
                    item->setText(KMapColumnPort, "");

                    m_uniMapTree->setItemWidget(item, KMapColumnIPAddress,
                            new QLineEdit(info->outputUcastAddress.toString()));
                    if (QHostAddress(controller->getNetworkIP()) == QHostAddress::LocalHost)
                        m_uniMapTree->itemWidget(item, KMapColumnIPAddress)->setEnabled(false);
                    QSpinBox* portSpin = new QSpinBox(this);
                    portSpin->setRange(0, 0xffff);
                    portSpin->setValue(info->outputUcastPort);
                    m_uniMapTree->setItemWidget(item, KMapColumnPort, portSpin);
                }
            }
            else
            {
                Q_ASSERT(false);
            }
            m_uniMapTree->resizeColumnToContents(KMapColumnIPAddress);
            m_uniMapTree->resizeColumnToContents(KMapColumnPort);
            return;
        }
    }

    Q_ASSERT(false);
}

/*****************************************************************************
 * Dialog actions
 *****************************************************************************/

void ConfigureE131::accept()
{
    for (int i = 0; i < m_uniMapTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *topItem = m_uniMapTree->topLevelItem(i);
        for (int c = 0; c < topItem->childCount(); c++)
        {
            QTreeWidgetItem *item = topItem->child(c);
            if (item->data(KMapColumnInterface, PROP_UNIVERSE).isValid() == false)
                continue;

            quint32 universe = item->data(KMapColumnInterface, PROP_UNIVERSE).toUInt();
            quint32 line = item->data(KMapColumnInterface, PROP_LINE).toUInt();
            E131Controller::Type type = E131Controller::Type(item->data(KMapColumnInterface, PROP_TYPE).toUInt());

            if (type == E131Controller::Input)
            {
                QCheckBox* multicastCb = qobject_cast<QCheckBox*>(m_uniMapTree->itemWidget(item, KMapColumnMulticast));
                if (multicastCb->isChecked())
                {
                    m_plugin->setParameter(universe, line, QLCIOPlugin::Input,
                            E131_MULTICAST, 1);
                    QWidget *ipWidget = m_uniMapTree->itemWidget(item, KMapColumnIPAddress);
                    // if present, remove any legacy parameter
                    m_plugin->unSetParameter(universe, line, QLCIOPlugin::Input, E131_MCASTIP);
                    // set the new full IP address
                    m_plugin->setParameter(universe, line, QLCIOPlugin::Input, E131_MCASTFULLIP, getIPAddress(ipWidget));
                }
                else
                {
                    m_plugin->setParameter(universe, line, QLCIOPlugin::Input,
                            E131_MULTICAST, 0);
                    QSpinBox* portSpin = qobject_cast<QSpinBox*>(m_uniMapTree->itemWidget(item, KMapColumnPort));
                    m_plugin->setParameter(universe, line, QLCIOPlugin::Input,
                            E131_UCASTPORT, portSpin->value());
                }

                QSpinBox* e131uniSpin = qobject_cast<QSpinBox*>(m_uniMapTree->itemWidget(item, KMapColumnE131Uni));
                m_plugin->setParameter(universe, line, QLCIOPlugin::Input,
                        E131_UNIVERSE, e131uniSpin->value());
            }
            else // if (type == E131Controller::Output)
            {
                QCheckBox* multicastCb = qobject_cast<QCheckBox*>(m_uniMapTree->itemWidget(item, KMapColumnMulticast));
                if (multicastCb->isChecked())
                {
                    m_plugin->setParameter(universe, line, QLCIOPlugin::Output,
                            E131_MULTICAST, 1);
                    QWidget *ipWidget = m_uniMapTree->itemWidget(item, KMapColumnIPAddress);
                    // if present, remove any legacy parameter
                    m_plugin->unSetParameter(universe, line, QLCIOPlugin::Output, E131_MCASTIP);
                    // set the new full IP address
                    m_plugin->setParameter(universe, line, QLCIOPlugin::Output, E131_MCASTFULLIP, getIPAddress(ipWidget));
                }
                else
                {
                    m_plugin->setParameter(universe, line, QLCIOPlugin::Output,
                            E131_MULTICAST, 0);
                    QLineEdit* ipEdit = qobject_cast<QLineEdit*>(m_uniMapTree->itemWidget(item, KMapColumnIPAddress));
                    QHostAddress newUnicastAddress(ipEdit->text());
                    if (newUnicastAddress.isNull())
                    {
                        showIPAlert(ipEdit->text());
                        return;
                    }
                    m_plugin->setParameter(universe, line, QLCIOPlugin::Output,
                            E131_UCASTIP, newUnicastAddress.toString());
                    QSpinBox* portSpin = qobject_cast<QSpinBox*>(m_uniMapTree->itemWidget(item, KMapColumnPort));
                    m_plugin->setParameter(universe, line, QLCIOPlugin::Output,
                            E131_UCASTPORT, portSpin->value());
                }
                QSpinBox* universeSpin = qobject_cast<QSpinBox*>(m_uniMapTree->itemWidget(item, KMapColumnE131Uni));
                m_plugin->setParameter(universe, line, QLCIOPlugin::Output,
                        E131_UNIVERSE, universeSpin->value());

                QComboBox* transCombo = qobject_cast<QComboBox*>(m_uniMapTree->itemWidget(item, KMapColumnTransmitMode));
                if (transCombo->currentIndex() == 1)
                    m_plugin->setParameter(universe, line, QLCIOPlugin::Output,
                            E131_TRANSMITMODE, E131Controller::transmissionModeToString(E131Controller::Partial));
                else
                    m_plugin->setParameter(universe, line, QLCIOPlugin::Output,
                            E131_TRANSMITMODE, E131Controller::transmissionModeToString(E131Controller::Full));

                QSpinBox* prioSpin = qobject_cast<QSpinBox*>(m_uniMapTree->itemWidget(item, KMapColumnPriority));
                m_plugin->setParameter(universe, line, QLCIOPlugin::Output,
                        E131_PRIORITY, prioSpin->value());
            }
        }
    }

    QSettings settings;
    int waitTime = m_waitReadySpin->value();
    if (waitTime == 0)
        settings.remove(SETTINGS_IFACE_WAIT_TIME);
    else
        settings.setValue(SETTINGS_IFACE_WAIT_TIME, waitTime);

    QDialog::accept();
}

int ConfigureE131::exec()
{
    return QDialog::exec();
}

