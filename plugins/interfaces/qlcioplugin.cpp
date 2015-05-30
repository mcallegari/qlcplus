/*
  Q Light Controller Plus
  qlcioplugin.cpp

  Copyright (c) Heikki Junnila
                Massimo Callegari

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

#include "qlcioplugin.h"

void QLCIOPlugin::setParameter(quint32 universe, QString name, QVariant &value)
{
    Q_UNUSED(universe)
    Q_UNUSED(name)
    Q_UNUSED(value)
}

/*************************************************************************
 * Outputs
 *************************************************************************/

bool QLCIOPlugin::openOutput(quint32 output)
{
    Q_UNUSED(output)
    return false;
}

void QLCIOPlugin::closeOutput(quint32 output)
{
    Q_UNUSED(output)
}

QStringList QLCIOPlugin::outputs()
{
    return QStringList();
}

QString QLCIOPlugin::outputInfo(quint32 output)
{
    Q_UNUSED(output)
    return QString();
}

void QLCIOPlugin::writeUniverse(quint32 universe, quint32 output, const QByteArray &data)
{
    Q_UNUSED(universe)
    Q_UNUSED(output)
    Q_UNUSED(data)
}

/*************************************************************************
 * Inputs
 *************************************************************************/

bool QLCIOPlugin::openInput(quint32 input, quint32 universe)
{
    Q_UNUSED(input)
    Q_UNUSED(universe)
    return false;
}

void QLCIOPlugin::closeInput(quint32 input)
{
    Q_UNUSED(input)
}

QStringList QLCIOPlugin::inputs()
{
    return QStringList();
}

QString QLCIOPlugin::inputInfo(quint32 input)
{
    Q_UNUSED(input)
    return QString();
}

void QLCIOPlugin::sendFeedBack(quint32 inputLine, quint32 channel, uchar value, const QString &key)
{
    Q_UNUSED(inputLine)
    Q_UNUSED(channel)
    Q_UNUSED(value)
    Q_UNUSED(key)
}

/*************************************************************************
 * Configure
 *************************************************************************/

void QLCIOPlugin::configure()
{
}

bool QLCIOPlugin::canConfigure()
{
    return false;
}









