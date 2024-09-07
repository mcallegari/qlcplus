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
#include <QSettings>
#include <QDebug>

#include <gpiod.hpp>

#include "gpioplugin.h"
#include "gpioreaderthread.h"
#include "gpioconfiguration.h"

#define SETTINGS_CHIP_NAME "GPIOPlugin/chipname"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

GPIOPlugin::~GPIOPlugin()
{
    int linesNum = m_gpioList.count();
    for (int i = 0; i < linesNum; i++)
    {
        GPIOLineInfo *gpio = m_gpioList.takeLast();
        delete gpio;
    }
}

void GPIOPlugin::init()
{
    m_readerThread = NULL;
    m_inputUniverse = UINT_MAX;
    m_outputUniverse = UINT_MAX;

    QSettings settings;
    QVariant value = settings.value(SETTINGS_CHIP_NAME);
    if (value.isValid() == true)
    {
        m_chipName = value.toString().toStdString();
    }
    else
    {
        // autodetect chips and use first
        for (auto& it: ::gpiod::make_chip_iter())
        {
            qDebug() << "GPIO chip found" << QString::fromStdString(it.name());
            if (m_chipName.empty())
                m_chipName = it.name();
        }
    }

    updateLinesList();
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

std::string GPIOPlugin::chipName() const
{
    return m_chipName;
}

void GPIOPlugin::setChipName(QString name)
{
    if (name == QString::fromStdString(m_chipName))
        return;

    m_chipName = name.toStdString();

    QSettings settings;
    settings.setValue(SETTINGS_CHIP_NAME, name);

    updateLinesList();
}

void GPIOPlugin::updateLinesList()
{
    ::gpiod::chip gChip(m_chipName);
    if (!gChip)
        return;

    int linesNum = m_gpioList.count();
    for (int i = 0; i < linesNum; i++)
    {
        GPIOLineInfo *gpio = m_gpioList.takeLast();
        delete gpio;
    }

    for (unsigned int i = 0; i < gChip.num_lines(); i++)
    {
        auto line = gChip.get_line(i);

        GPIOLineInfo *gpio = new GPIOLineInfo;
        gpio->m_line = i;
        gpio->m_name = QString::fromStdString(line.name());
        gpio->m_enabled = false;
        gpio->m_value = 1;
        gpio->m_count = 0;

        /*
        int direction = line.direction();

        if (direction == ::gpiod::line::DIRECTION_INPUT)
            gpio->m_direction = InputDirection;
        else if (direction == ::gpiod::line::DIRECTION_OUTPUT)
            gpio->m_direction = OutputDirection;
        else
        */
        gpio->m_direction = NoDirection;

        m_gpioList.append(gpio);
    }
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
    list << QString("GPIO lines");
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

void GPIOPlugin::writeUniverse(quint32 universe, quint32 output, const QByteArray &data, bool dataChanged)
{
    Q_UNUSED(universe)
    Q_UNUSED(dataChanged)

    if (output != 0)
        return;

    for (int i = 0; i < qMin(data.size(), m_gpioList.count()); i++)
    {
        GPIOLineInfo *gpio = m_gpioList.at(i);
        if (gpio->m_enabled == false || gpio->m_direction != OutputDirection)
            continue;

        //qDebug() << "[GPIO] writing GPIO number:" << i;
        uchar boolVal = uchar(data.at(i)) < 128 ? 1 : 0;
        if (gpio->m_value != boolVal)
            setLineValue(i, boolVal);
    }
}

/*********************************************************************
 * GPIO PIN methods
 *********************************************************************/
QList<GPIOLineInfo *> GPIOPlugin::gpioList() const
{
    return m_gpioList;
}

QString GPIOPlugin::lineDirectionToString(GPIOPlugin::LineDirection usage)
{
    switch(usage)
    {
        case OutputDirection: return QString("Output"); break;
        case InputDirection: return QString("Input"); break;
        default: break;
    }
    return QString("NotUsed");
}

GPIOPlugin::LineDirection GPIOPlugin::stringToLineDirection(QString usage)
{
    if (usage == "Output") return OutputDirection;
    else if (usage == "Input") return InputDirection;

    return NoDirection;
}

void GPIOPlugin::setLineStatus(int lineNumber, bool enable)
{
    if (lineNumber < 0 || lineNumber >= m_gpioList.count())
        return;

    if (m_gpioList.at(lineNumber)->m_enabled == enable)
        return;

    m_gpioList[lineNumber]->m_enabled = enable;
}

void GPIOPlugin::setLineDirection(int lineNumber, GPIOPlugin::LineDirection direction)
{
    if (lineNumber < 0 || lineNumber >= m_gpioList.count())
        return;

    qDebug() << "[GPIO] setLineDirection" << lineNumber << direction;
    GPIOLineInfo *gpio = m_gpioList.at(lineNumber);

    if (gpio->m_direction == direction)
        return;

    if (gpio->m_direction == InputDirection || direction == InputDirection)
    {
        if (m_readerThread != NULL && m_readerThread->isRunning())
            m_readerThread->pause(true);
    }

    if (direction == NoDirection)
    {
        m_gpioList[lineNumber]->m_direction = direction;

        setLineStatus(lineNumber, false);

        if (m_readerThread != NULL)
            m_readerThread->pause(false);

        return;
    }
    else
        setLineStatus(lineNumber, true);

    m_gpioList[lineNumber]->m_direction = direction;

    if (m_readerThread != NULL)
    {
        m_readerThread->pause(false);
    }
    else
    {
        if (direction == InputDirection)
        {
            m_readerThread = new ReadThread(this);
            connect(m_readerThread, SIGNAL(valueChanged(quint32,uchar)),
                    this, SLOT(slotValueChanged(quint32,uchar)));
        }
    }
}

void GPIOPlugin::setLineValue(int lineNumber, uchar value)
{
    if (lineNumber < 0 || lineNumber >= m_gpioList.count())
        return;

    qDebug() << "[GPIO] writing line" << lineNumber << "with value" << value;

    ::gpiod::chip gChip(chipName());
    ::gpiod::line gLine = gChip.get_line(lineNumber);
    gLine.request({"set_value", gpiod::line_request::DIRECTION_OUTPUT, 0}, value);
    gLine.release();

    m_gpioList[lineNumber]->m_value = value;
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
    list << QString("GPIO lines");
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
    // rewrite the target universe based on the type...
    universe = (type == QLCIOPlugin::Input) ? m_inputUniverse : m_outputUniverse;

    //qDebug() << "[SetParameter]" << universe << line << name << value.toString();
    QStringList param = name.split("-");
    if (param.count() < 2)
    {
        qDebug() << "[GPIO] invalid parameter name!" << name;
        return;
    }
    int gpioNumber = param.at(1).toInt();
    if (gpioNumber < 0 || gpioNumber >= m_gpioList.count())
    {
        qDebug() << "[GPIO] invalid PIN number!" << gpioNumber;
        return;
    }

    if (param.at(0) == GPIO_PARAM_USAGE)
    {
        LineDirection usage = stringToLineDirection(value.toString());
        LineDirection prevUsage = LineDirection(m_gpioList.at(gpioNumber)->m_direction);

        setLineDirection(gpioNumber, usage);

        if (usage == NoDirection)
        {
            if (prevUsage == InputDirection)
                QLCIOPlugin::unSetParameter(m_inputUniverse, 0, Input, name);
            else
                QLCIOPlugin::unSetParameter(m_outputUniverse, 0, Output, name);
            return;
        }
    }

    QLCIOPlugin::setParameter(universe, line, type, name, value);
}
