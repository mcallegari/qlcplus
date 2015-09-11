/*
  Q Light Controller Plus
  gpioplugin.cpp

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
#include <QDebug>

#include "gpioplugin.h"
#include "gpioreaderthread.h"
#include "gpioconfiguration.h"

#define MAX_GPIO_PINS   30

/*****************************************************************************
 * Initialization
 *****************************************************************************/

GPIOPlugin::~GPIOPlugin()
{
    for (int i = 0; i < MAX_GPIO_PINS; i++)
        setPinStatus(i, false);
}

void GPIOPlugin::init()
{
    m_readerThread = NULL;
    m_inputUniverse = UINT_MAX;
    m_outputUniverse = UINT_MAX;

    for (int i = 0; i < MAX_GPIO_PINS; i++)
    {
        GPIOPinInfo *gpio = new GPIOPinInfo;
        gpio->m_number = i;
        gpio->m_usage = NoUsage;
        gpio->m_value = 1;

        QString pinPath = QString("/sys/class/gpio/gpio%1/value").arg(i);
        gpio->m_file = new QFile(pinPath);

        m_gpioList.append(gpio);
    }
}

QString GPIOPlugin::name()
{
    return QString("GPIO");
}

int GPIOPlugin::capabilities() const
{
    /** Return a mask of the plugin capabilities here.
     *  See the QLCIOPlugin Capability enum for usage
     */
    return QLCIOPlugin::Output | QLCIOPlugin::Input;
}

QString GPIOPlugin::pluginInfo()
{
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    str += QString("<P>");
    str += QString("<H3>%1</H3>").arg(name());
    str += tr("This plugin provides input/output on GPIO PINs.");
    str += QString("</P>");

    return str;
}

/*****************************************************************************
 * Outputs
 *****************************************************************************/

bool GPIOPlugin::openOutput(quint32 output, quint32 universe)
{
    if (output != 0)
        return false;

    m_outputUniverse = universe;

    addToMap(universe, output, Output);

    return true;
}

void GPIOPlugin::closeOutput(quint32 output, quint32 universe)
{
    if (output != 0)
        return;

    m_outputUniverse = UINT_MAX;

    removeFromMap(output, universe, Output);
}

QStringList GPIOPlugin::outputs()
{
    QStringList list;
    list << QString("1: GPIO lines");
    return list;
}

QString GPIOPlugin::outputInfo(quint32 output)
{
    QString str;

    if (output == 0)
        str += QString("<H3>%1</H3>").arg(outputs()[output]);

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void GPIOPlugin::writeUniverse(quint32 universe, quint32 output, const QByteArray &data)
{
    Q_UNUSED(universe)

    if (output != 0)
        return;

    for (int i = 0; i < qMin(data.size(), MAX_GPIO_PINS); i++)
    {
        GPIOPinInfo *gpio = m_gpioList.at(i);
        if (gpio->m_enabled == false || gpio->m_usage != OutputUsage)
            continue;

        //qDebug() << "[GPIO] writing GPIO number:" << i;
        uchar boolVal = uchar(data.at(i)) < 128 ? 1 : 0;
        if (gpio->m_value != boolVal)
            setPinValue(i, boolVal);
    }
}

/*********************************************************************
 * GPIO PIN methods
 *********************************************************************/
QList<GPIOPinInfo *> GPIOPlugin::gpioList() const
{
    return m_gpioList;
}

QString GPIOPlugin::pinUsageToString(GPIOPlugin::PinUsage usage)
{
    switch(usage)
    {
        case OutputUsage: return QString("Output"); break;
        case InputUsage: return QString("Input"); break;
        default: break;
    }
    return QString("NotUsed");
}

GPIOPlugin::PinUsage GPIOPlugin::stringToPinUsage(QString usage)
{
    if (usage == "Output") return OutputUsage;
    else if (usage == "Input") return InputUsage;

    return NoUsage;
}

void GPIOPlugin::setPinStatus(int gpioNumber, bool enable)
{
    if (gpioNumber < 0 || gpioNumber >= m_gpioList.count())
        return;

    if (m_gpioList.at(gpioNumber)->m_enabled == enable)
        return;

    QString sysPath = QString("/sys/class/gpio/%1").arg(enable ? "export" : "unexport");
    QString gpioStr = QString("%1").arg(gpioNumber);
    QFile file(sysPath);
    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "[GPIO] error in opening export file" << sysPath;
        return;
    }
    file.write(gpioStr.toLatin1());
    file.close();

    m_gpioList[gpioNumber]->m_enabled = enable;
}

void GPIOPlugin::setPinUsage(int gpioNumber, GPIOPlugin::PinUsage usage)
{
    if (gpioNumber < 0 || gpioNumber >= m_gpioList.count())
        return;

    qDebug() << "[GPIO] setPinUsage" << gpioNumber << usage;
    GPIOPinInfo *gpio = m_gpioList.at(gpioNumber);

    if (gpio->m_usage == usage)
        return;

    if (gpio->m_usage == InputUsage || usage == InputUsage)
    {
        if (m_readerThread != NULL && m_readerThread->isRunning())
            m_readerThread->pause(true);
    }

    if (usage == NoUsage)
    {
        if (gpio->m_usage == InputUsage)
            m_gpioList[gpioNumber]->m_file->close();

        m_gpioList[gpioNumber]->m_usage = usage;

        setPinStatus(gpioNumber, false);

        if (m_readerThread != NULL)
        {
            m_readerThread->updateReadPINs();
            m_readerThread->pause(false);
        }
        return;
    }
    else
        setPinStatus(gpioNumber, true);

    QString pinPath = QString("/sys/class/gpio/gpio%1/direction").arg(gpioNumber);
    QFile file(pinPath);
    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "[GPIO] error in opening direction file" << pinPath;
        return;
    }
    if (usage == OutputUsage)
        file.write("out");
    else if (usage == InputUsage)
        file.write("in");
    file.close();

    m_gpioList[gpioNumber]->m_usage = usage;

    if (m_readerThread != NULL)
    {
        m_readerThread->updateReadPINs();
        m_readerThread->pause(false);
    }
    else
    {
        if (usage == InputUsage)
        {
            m_readerThread = new ReadThread(this);
            connect(m_readerThread, SIGNAL(valueChanged(quint32,uchar)),
                    this, SLOT(slotValueChanged(quint32,uchar)));
        }
    }
}

void GPIOPlugin::setPinValue(int gpioNumber, uchar value)
{
    if (gpioNumber < 0 || gpioNumber >= m_gpioList.count())
        return;

    GPIOPinInfo *gpio = m_gpioList.at(gpioNumber);
    if (gpio->m_file->isOpen() == false)
    {
        //qDebug() << "[GPIO] Opening value file of PIN" << gpioNumber;
        if (!gpio->m_file->open(QIODevice::WriteOnly))
        {
            qDebug() << "[GPIO] Error, cannot open PIN" << gpioNumber << "for writing";
            return;
        }
    }

    qDebug() << "[GPIO] writing PIN" << gpioNumber << "with value" << value;
    //gpio->m_file->reset();
    //gpio->m_file->write(QString::number(value).toLatin1());
    gpio->m_file->putChar(value + 48);
    gpio->m_file->close();
    //gpio->m_file->write("\n");
    m_gpioList[gpioNumber]->m_value = value;
}

/*************************************************************************
 * Inputs
 *************************************************************************/

bool GPIOPlugin::openInput(quint32 input, quint32 universe)
{
    if (input != 0)
        return false;

    m_inputUniverse = universe;

    addToMap(universe, input, Input);

    // we do not start the reader thread here,
    // as there might be no PINs configure as
    // InputUsage yet.
    // At some point a setParameter will be called
    // so the reader thread will be started

    return true;
}

void GPIOPlugin::closeInput(quint32 input, quint32 universe)
{
    if (input != 0)
        return;

    m_inputUniverse = UINT_MAX;

    removeFromMap(input, universe, Input);
}

QStringList GPIOPlugin::inputs()
{
    QStringList list;
    list << QString("1: GPIO lines");
    return list;
}

QString GPIOPlugin::inputInfo(quint32 input)
{
    QString str;

    if (input == 0)
        str += QString("<H3>%1</H3>").arg(inputs()[input]);

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void GPIOPlugin::slotValueChanged(quint32 channel, uchar value)
{
    emit valueChanged(m_inputUniverse, 0, channel, value);
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void GPIOPlugin::configure()
{
    GPIOConfiguration conf(this);
    if (conf.exec() == QDialog::Accepted)
    {

    }
}

bool GPIOPlugin::canConfigure()
{
    return true;
}

void GPIOPlugin::setParameter(quint32 universe, quint32 line, Capability type,
                             QString name, QVariant value)
{
    QStringList param = name.split("-");
    if (param.count() < 2)
    {
        qDebug() << "[GPIO] invalid parameter name !" << name;
        return;
    }
    int gpioNumber = param.at(1).toInt();
    if (gpioNumber < 0 || gpioNumber >= m_gpioList.count())
    {
        qDebug() << "[GPIO] invalid PIN number !" << gpioNumber;
        return;
    }

    if (param.at(0) == GPIO_PARAM_USAGE)
    {
        PinUsage usage = stringToPinUsage(value.toString());
        PinUsage prevUsage = PinUsage(m_gpioList.at(gpioNumber)->m_usage);

        setPinUsage(gpioNumber, usage);

        if (usage == NoUsage)
        {
            if (prevUsage == InputUsage)
                QLCIOPlugin::unSetParameter(m_inputUniverse, 0, Input, name);
            else
                QLCIOPlugin::unSetParameter(m_outputUniverse, 0, Output, name);
            return;
        }
    }

    QLCIOPlugin::setParameter(universe, line, type, name, value);
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(gpioplugin, GPIOPlugin)
#endif
