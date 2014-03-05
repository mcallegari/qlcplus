/*
  Q Light Controller
  fx5.cpp

  Copyright (c) Florian Euchner

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

#include <QString>
#include <QDebug>
#include <QMessageBox>

#include "fx5.h"

/*****************************************************************************
 * FX5 - QLCIOPlugin (QLC I/O Plugin Class)
 *****************************************************************************/

FX5::~FX5()
{
    CloseAllLinks();
}

void FX5::init()
{
    scanInterfaces();
    RegisterInputChangeNotification(FX5::notifyInputWrapper, this);
}

QString FX5::name()
{
    return QString("FX5");
}

int FX5::capabilities() const
{
    return QLCIOPlugin::Output | QLCIOPlugin::Input;
}

/*****************************************************************************
 * Outputs
 *****************************************************************************/

void FX5::openOutput(quint32 output)
{
    if (output < quint32(m_devices.size()))
        m_devices.at(output)->openOutput();
}

void FX5::closeOutput(quint32 output)
{
    if (output < quint32(m_devices.size()))
        m_devices.at(output)->closeOutput();
}

QStringList FX5::outputs()
{
    QStringList list;
    int i = 1;

    QListIterator <FX5Device*> it(m_devices);
    while (it.hasNext() == true)
        list << QString("%1: %2").arg(i++).arg(it.next()->getName());
    return list;
}

QString FX5::pluginInfo()
{
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");

    str += QString("<BODY>");
    str += QString("<P>");
    str += QString("<H3>%1</H3>").arg(name());
    str += tr("This plugin provides DMX output and input support for Digital Enlightenment / FX5 devices.");
    str += QString("</P>");

    return str;
}

QString FX5::outputInfo(quint32 output)
{
    QString str;

    str += QString("<H4>" + tr("Outputs") + "</H4>");
    if (output != QLCIOPlugin::invalidLine() && output < quint32(m_devices.size()))
        str += m_devices.at(output)->getInfoText()  + tr(" as Output");

    if (quint32(m_devices.size()) == 0)
        str += tr("No output devices found.");

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void FX5::writeUniverse(quint32 universe, quint32 output, const QByteArray &data)
{
    Q_UNUSED(universe)

    if (output < quint32(m_devices.size()))
        m_devices.at(output)->outputDMX(data);
}

void FX5::scanInterfaces()
{
    // Remove all interfaces and renew the list
    m_devices.clear();

    // Retrieve a list of connected interfaces
    TSERIALLIST interfaces;
    GetAllConnectedInterfaces(&interfaces);

    for (quint32 port = 0; port < 32; ++port)
    {
        char str[17] = {};
        str[16] = 0; // Null-Terminate string
        memcpy(str, interfaces[port], 16);
        QString serial(str);
        quint32 version = GetDeviceVersion(interfaces[port]);

        if (serial == "0000000000000000") break; // No more device found

        m_devices.push_back(new FX5Device(str, version, port));
    }

    emit configurationChanged();
}

/*****************************************************************************
 * Input
 *****************************************************************************/
void FX5::openInput(quint32 input)
{
    if (input < quint32(m_devices.size()))
        m_devices.at(input)->openInput();
}

void FX5::closeInput(quint32 input)
{
    if (input < quint32(m_devices.size()))
        m_devices.at(input)->closeInput();
}

QStringList FX5::inputs()
{
    // FX5 Outputs are Inputs at the same time
    return outputs();
}

QString FX5::inputInfo(quint32 input)
{
    QString str;

    str += QString("<H4>" + tr("Inputs") + "</H4>");
    if (input != QLCIOPlugin::invalidLine() && input < quint32(m_devices.size()))
        str += m_devices.at(input)->getInfoText() + tr(" as Input");

    if (quint32(m_devices.size()) == 0)
        str += tr("No input devices found.");

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void FX5::notifyInput()
{
    /*
        If this function is called, any of the FX5 interfaces has had an
        input state change; so tell all of them to check for changes
    */
    QListIterator <FX5Device*> it(m_devices);
    while (it.hasNext() == true)
        it.next()->notifyInput(this);
}

void FX5::notifyInputWrapper(void *self)
{
    ((FX5*)self)->notifyInput();
}

void FX5::emitChangeValue(quint32 port, quint32 channel, uchar value)
{
    emit valueChanged(port, channel, value);
}


/*****************************************************************************
 * Configuration
 *****************************************************************************/

void FX5::configure()
{
    int r = QMessageBox::question(NULL, name(),
                tr("Do you wish to re-scan your hardware?"),
                QMessageBox::Yes, QMessageBox::No);
    if (r == QMessageBox::Yes)
        scanInterfaces();
}

bool FX5::canConfigure()
{
    return true;
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(fx5, FX5)
#endif
