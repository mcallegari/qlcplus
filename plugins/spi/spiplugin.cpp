/*
  Q Light Controller Plus
  spiplugin.cpp

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

#include <QStringList>
#include <QSettings>
#include <QString>
#include <QDebug>
#include <QFile>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include "spiplugin.h"
#include "spiconfiguration.h"

#define SPI_DEFAULT_DEVICE  "/dev/spidev0.0"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

SPIPlugin::~SPIPlugin()
{
    if (m_spifd != -1)
        close(m_spifd);
}

void SPIPlugin::init()
{
    QSettings settings;
    m_spifd = -1;

    QVariant value = settings.value("SPIPlugin/frequency");
    if (value.isValid() == true)
        m_speed = value.toUInt();
    else
        m_speed = 1000000;
}

QString SPIPlugin::name()
{
    return QString("SPI");
}

int SPIPlugin::capabilities() const
{
    return QLCIOPlugin::Output;
}

/*****************************************************************************
 * Open/close
 *****************************************************************************/

void SPIPlugin::openOutput(quint32 output)
{
    int status = -1;

    if (output != 0)
        return;

    m_spifd = open(SPI_DEFAULT_DEVICE, O_RDWR);
    if(m_spifd < 0)
    {
        qWarning() << "Cannot open SPI device !";
        return;
    }

    int mode = SPI_MODE_0;
    m_bitsPerWord = 8;

    status = ioctl (m_spifd, SPI_IOC_WR_MODE, &mode);
    if(status < 0)
        qWarning() << "Could not set SPIMode (WR)...ioctl fail";

    status = ioctl (m_spifd, SPI_IOC_WR_BITS_PER_WORD, &m_bitsPerWord);
    if(status < 0)
        qWarning() << "Could not set SPI bitsPerWord (WR)...ioctl fail";

    status = ioctl (m_spifd, SPI_IOC_WR_MAX_SPEED_HZ, &m_speed);
    if(status < 0)
        qWarning() << "Could not set SPI speed (WR)...ioctl fail";
}

void SPIPlugin::closeOutput(quint32 output)
{
    if (output != 0)
        return;

    if (m_spifd != -1)
        close(m_spifd);
}

QStringList SPIPlugin::outputs()
{
    QStringList list;
    QFile file(QString(SPI_DEFAULT_DEVICE));
    if (file.exists() == true)
        list << QString("1: SPI0 CS0");
    return list;
}

QString SPIPlugin::pluginInfo()
{
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    str += QString("<P>");
    str += QString("<H3>%1</H3>").arg(name());
    str += tr("This plugin provides DMX output for SPI devices.");
    str += QString("</P>");

    return str;
    }

QString SPIPlugin::outputInfo(quint32 output)
{
    QString str;

    if (output != QLCIOPlugin::invalidLine() && output == 0)
    {
        str += QString("<H3>%1</H3>").arg(outputs()[output]);
    }

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void SPIPlugin::writeUniverse(quint32 universe, quint32 output, const QByteArray &data)
{
    Q_UNUSED(universe)

    if (output != 0 || m_spifd == -1)
        return;

    struct spi_ioc_transfer spi;
    int retVal = -1;

    memset(&spi, 0, sizeof(spi));
    spi.tx_buf        = reinterpret_cast<__u64>(data.data());
    spi.len           = data.size();
    spi.delay_usecs   = 0;
    spi.speed_hz      = m_speed;
    spi.bits_per_word = m_bitsPerWord;
    spi.cs_change = 0;

    retVal = ioctl(m_spifd, SPI_IOC_MESSAGE(1), &spi);

    if(retVal < 0)
        qWarning() << "Problem transmitting spi data..ioctl";
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void SPIPlugin::configure()
{
    SPIConfiguration conf(this);
    if (conf.exec() == QDialog::Accepted)
    {
        QSettings settings;
        settings.setValue("SPIPlugin/frequency", QVariant(conf.frequency()));
    }
}

bool SPIPlugin::canConfigure()
{
    return true;
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(spiplugin, SPIPlugin)
#endif
