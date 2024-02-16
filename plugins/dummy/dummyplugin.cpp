/*
  Q Light Controller Plus
  dummyplugin.cpp

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

#include "dummyplugin.h"
#include "dummyconfiguration.h"

/** Place here the global defines specific to this plugin */

/*****************************************************************************
 * Initialization
 *****************************************************************************/

DummyPlugin::~DummyPlugin()
{
    /** Clean up the plugin resources here.
     *  E.g. running threads, allocated memory, etc..
     */
}

void DummyPlugin::init()
{
    /** Initialize the plugin variables here */
}

QString DummyPlugin::name()
{
    return QString("Dummy");
}

int DummyPlugin::capabilities() const
{
    /** Return a mask of the plugin capabilities here.
     *  See the QLCIOPlugin Capability enum for usage
     */
    return QLCIOPlugin::Output | QLCIOPlugin::Input;
}

/*****************************************************************************
 * Outputs
 *****************************************************************************/

bool DummyPlugin::openOutput(quint32 output, quint32 universe)
{
    /** Check for output index validity and, in case, return false */

    addToMap(universe, output, Output);

    /** Do the plugin specific operations to
     *  open the requested output line */

    return true;
}

void DummyPlugin::closeOutput(quint32 output, quint32 universe)
{
    /** Check for output index validity and, in case, return */

    removeFromMap(output, universe, Output);

    /** Do the plugin specific operations to
     *  close the requested output line */
}

QStringList DummyPlugin::outputs()
{
    /**
     * Build a list of output line names. The names must be always in the
     * same order i.e. the first name is the name of output line number 0,
     * the next one is output line number 1, etc..
     */
    QStringList list;
    list << QString("Dummy line");
    return list;
}

QString DummyPlugin::pluginInfo()
{
    /** Return a description of the purpose of this plugin
     *  in HTML format */
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    str += QString("<P>");
    str += QString("<H3>%1</H3>").arg(name());
    str += tr("This plugin provides dummy input/output for dummy devices.");
    str += QString("</P>");

    return str;
}

QString DummyPlugin::outputInfo(quint32 output)
{
    /**
     * Provide an informational text regarding the specified output line.
     * This text is in HTML format and it is shown to the user.
     */
    QString str;

    if (output != QLCIOPlugin::invalidLine())
        str += QString("<H3>%1</H3>").arg(outputs()[output]);

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void DummyPlugin::writeUniverse(quint32 universe, quint32 output, const QByteArray &data, bool dataChanged)
{
    Q_UNUSED(universe)
    Q_UNUSED(output)
    Q_UNUSED(data)
    Q_UNUSED(dataChanged)

    /** Check for output index validity and, in case, return.
     *
     * This method is very important as it is the implementation of the plugin
     * data transmission. It is called at the rate of the QLC+ MasterTimer clock
     * and it should never block for more than 20ms.
     * If this plugin cannot predict the duration of a universe transmission,
     * it is then safer to exchange data with a thread, running indipendently
     * and not risking to hang QLC+
     */
}

/*************************************************************************
 * Inputs - If the plugin doesn't provide input
 * just remove this whole block
 *************************************************************************/

bool DummyPlugin::openInput(quint32 input, quint32 universe)
{
    /** Check for input index validity and, in case, return false */

    addToMap(universe, input, Input);

    /** Do the plugin specific operations to
     *  open the requested input line */

    return true;
}

void DummyPlugin::closeInput(quint32 input, quint32 universe)
{
    /** Check for input index validity and, in case, return */

    removeFromMap(input, universe, Input);

    /** Do the plugin specific operations to
     *  close the requested input line */
}

QStringList DummyPlugin::inputs()
{
    /**
     * Build a list of input line names. The names must be always in the
     * same order i.e. the first name is the name of input line number 0,
     * the next one is output line number 1, etc..
     */
    QStringList list;
    list << QString("Dummy line");
    return list;
}

QString DummyPlugin::inputInfo(quint32 input)
{
    /**
     * Provide an informational text regarding the specified input line.
     * This text is in HTML format and it is shown to the user.
     */
    QString str;

    if (input != QLCIOPlugin::invalidLine())
        str += QString("<H3>%1</H3>").arg(inputs()[input]);

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void DummyPlugin::sendFeedBack(quint32 universe, quint32 output, quint32 channel, uchar value, const QVariant &params)
{
    Q_UNUSED(universe)
    Q_UNUSED(output)
    Q_UNUSED(channel)
    Q_UNUSED(value)
    Q_UNUSED(params)

    /**
     * If the device support this feature, this is the method to send data back for
     * visual feedback.
     * To implement such method, the plugin must have an input line corresponding
     * to the specified output line.
     * Basically feedback data must return to the same line where it came from
     */
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void DummyPlugin::configure()
{
    DummyConfiguration conf(this);
    if (conf.exec() == QDialog::Accepted)
    {

    }
}

bool DummyPlugin::canConfigure()
{
    return true;
}

void DummyPlugin::setParameter(quint32 universe, quint32 line, Capability type,
                             QString name, QVariant value)
{
    Q_UNUSED(universe)
    Q_UNUSED(line)
    Q_UNUSED(type)
    Q_UNUSED(value)

    /** This method is provided to QLC+ to set the plugin specific settings.
     *  Those settings are saved in a project workspace and when it is loaded,
     *  this method is called after QLC+ has opened the input/output lines
     *  mapped in the project workspace as well.
     */

    if (name == "DummyParameter")
    {
        // do something smart :)
    }

    /** Remember to call the base QLCIOPlugin method to actually inform
     *  QLC+ to store the parameter in the project workspace XML */
    QLCIOPlugin::setParameter(universe, line, type, name, value);
}
