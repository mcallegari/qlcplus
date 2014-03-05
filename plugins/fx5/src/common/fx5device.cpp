/*
  Q Light Controller
  fx5device.cpp

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

extern "C" {
    #include "fx5driver.h"
}

#include "fx5.h"
#include "fx5device.h"

/*****************************************************************************
 * FX5Device - Custom device class for each connected FX5 Interface
 *****************************************************************************/

FX5Device::FX5Device(char *serial, quint32 version, quint32 port) :
m_version(version),
m_port(port),
m_mode(FX5_MODE_NONE),
m_link_open(false)
{
    memcpy(m_serial, serial, 16);
    memset(m_dmx_in , 0, sizeof(TDMXArray));
    memset(m_dmx_out, 0, sizeof(TDMXArray));
    memset(m_dmx_instate, 0, 512);

    qDebug() << "FX5Device::FX5Device()";
}

QString FX5Device::getName()
{
    char str[17];

    str[16] = 0; // Null-Terminate string
    memcpy(str, m_serial, 16);

    return QString(str);
}

QString FX5Device::getInfoText()
{
    QString infotext;

    infotext = tr("FX5 USB-DMX Interface with serial number ") + getName()
                    + tr(" and version ") + QString::number(m_version);

    return infotext;
}

/*
    Device Links & Mode Manager
*/

void FX5Device::updateMode()
{
    if (!m_link_open)
    {
        if(OpenLink(m_serial, &m_dmx_out, &m_dmx_in) != 1)
        {
            QMessageBox::warning(NULL, (tr("FX5 USB DMX Interface Error")),
                (tr("Unable to open the FX5 Interface. Make sure the udev rule is installed.")),
                 QMessageBox::AcceptRole, QMessageBox::AcceptRole);
        }
    }

    int driver_mode = 0;
    if (m_mode & FX5_MODE_OUTPUT)
        driver_mode += 2;
    if (m_mode & FX5_MODE_INPUT)
        driver_mode += 4;

    if (driver_mode == 0) // Neither input nor output are in use
        CloseLink(m_serial);
    else
        SetInterfaceMode(m_serial, driver_mode);
}

/*****************************************************************************
 * Output
 *****************************************************************************/
void FX5Device::openOutput()
{
    m_mode |= FX5_MODE_OUTPUT;
    updateMode();    
}

void FX5Device::closeOutput()
{
    m_mode &= ~FX5_MODE_OUTPUT;
    updateMode();    
}

void FX5Device::outputDMX(const QByteArray &universe)
{
    for (uint16_t i = 0; i<universe.size(); i++)
        m_dmx_out[i] = universe[i];
}

/*****************************************************************************
 * Input
 *****************************************************************************/

void FX5Device::openInput()
{
    m_mode |= FX5_MODE_INPUT;
    updateMode();    
}

void FX5Device::closeInput()
{
    m_mode &= ~FX5_MODE_INPUT;
    updateMode();    
}

void FX5Device::notifyInput(FX5 *sender)
{
    for (uint16_t chan = 0; chan<sizeof(m_dmx_instate); chan++)
    {
        if (m_dmx_instate[chan] != m_dmx_in[chan])
        {
            m_dmx_instate[chan] = m_dmx_in[chan];
            sender->emitChangeValue(m_port, chan, m_dmx_in[chan]);
        }
    }
}

