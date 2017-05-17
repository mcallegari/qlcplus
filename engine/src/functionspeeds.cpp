/*
  Q Light Controller Plus
  functionspeeds.cpp

  Copyright (C) 2016 Massimo Callegari
                     David Garyga

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

#include <cmath>
#include <QXmlStreamReader>

#include "functionspeeds.h"
#include "function.h"
#include "qlcmacros.h"

FunctionSpeeds::FunctionSpeeds(quint32 fadeIn, quint32 hold, quint32 fadeOut)
    : m_fadeIn(fadeIn)
    , m_hold(hold)
    , m_fadeOut(fadeOut)
{
}

void FunctionSpeeds::setTempoType(Speed::TempoType tempoType, float beatTime)
{
    m_fadeIn.switchTempoType(tempoType, beatTime);
    m_hold.switchTempoType(tempoType, beatTime);
    m_fadeOut.switchTempoType(tempoType, beatTime);
}

Speed::TempoType FunctionSpeeds::tempoType() const
{
    return m_fadeIn.tempoType;
}

quint32 FunctionSpeeds::fadeIn() const
{
    return m_fadeIn.value;
}

void FunctionSpeeds::setFadeIn(quint32 fadeIn)
{
    m_fadeIn.value = fadeIn;
}

quint32 FunctionSpeeds::hold() const
{
    return m_hold.value;
}

void FunctionSpeeds::setHold(quint32 hold)
{
    m_hold.value = hold;
}

quint32 FunctionSpeeds::fadeOut() const
{
    return m_fadeOut.value;
}

void FunctionSpeeds::setFadeOut(quint32 fadeOut)
{
    m_fadeOut.value = fadeOut;
}

quint32 FunctionSpeeds::duration() const
{
    if (m_fadeIn.value == Speed::originalValue() || m_hold.value == Speed::originalValue())
        return m_hold.value;
    return Speed::add(m_fadeIn.value, m_hold.value);
}

void FunctionSpeeds::setDuration(quint32 duration)
{
    if (m_fadeIn.value == Speed::originalValue() || duration == Speed::originalValue())
        m_hold.value = duration;
    else
    {
        m_hold.value = Speed::sub(duration, m_fadeIn.value);
        if (m_hold.value == 0)
            m_fadeIn.value = duration;
    }
}

quint32 FunctionSpeeds::msFadeIn(float beatTime) const
{
    if (tempoType() == Speed::Ms)
        return fadeIn();

    return Speed::msToBeats(fadeIn(), beatTime);
}

void FunctionSpeeds::setMsFadeIn(quint32 fadeIn, float beatTime)
{
    if (tempoType() == Speed::Ms)
        setFadeIn(fadeIn);

    setFadeIn(Speed::msToBeats(fadeIn, beatTime));
}

quint32 FunctionSpeeds::msHold(float beatTime) const
{
    if (tempoType() == Speed::Ms)
        return hold();

    return Speed::msToBeats(hold(), beatTime);
}

void FunctionSpeeds::setMsHold(quint32 hold, float beatTime)
{
    if (tempoType() == Speed::Ms)
        setHold(hold);

    setHold(Speed::msToBeats(hold, beatTime));
}

quint32 FunctionSpeeds::msFadeOut(float beatTime) const
{
    if (tempoType() == Speed::Ms)
        return fadeOut();

    return Speed::msToBeats(fadeOut(), beatTime);
}

void FunctionSpeeds::setMsFadeOut(quint32 fadeOut, float beatTime)
{
    if (tempoType() == Speed::Ms)
        setFadeOut(fadeOut);

    setFadeOut(Speed::msToBeats(fadeOut, beatTime));
}

quint32 FunctionSpeeds::msDuration(float beatTime) const
{
    if (tempoType() == Speed::Ms)
        return duration();

    return Speed::msToBeats(duration(), beatTime);
}

void FunctionSpeeds::setMsDuration(quint32 duration, float beatTime)
{
    if (tempoType() == Speed::Ms)
        setDuration(fadeOut());

    setDuration(Speed::msToBeats(duration, beatTime));
}

quint32 FunctionSpeeds::beatsFadeIn(float beatTime) const
{
    if (tempoType() == Speed::Beats)
        return fadeIn();

    return Speed::beatsToMs(fadeIn(), beatTime);
}

void FunctionSpeeds::setBeatsFadeIn(quint32 fadeIn, float beatTime)
{
    if (tempoType() == Speed::Beats)
        setFadeIn(fadeIn);

    setFadeIn(Speed::beatsToMs(fadeIn, beatTime));
}

quint32 FunctionSpeeds::beatsHold(float beatTime) const
{
    if (tempoType() == Speed::Beats)
        return hold();

    return Speed::beatsToMs(hold(), beatTime);
}

void FunctionSpeeds::setBeatsHold(quint32 hold, float beatTime)
{
    if (tempoType() == Speed::Beats)
        setHold(hold);

    setHold(Speed::beatsToMs(hold, beatTime));
}

quint32 FunctionSpeeds::beatsFadeOut(float beatTime) const
{
    if (tempoType() == Speed::Beats)
        return fadeOut();

    return Speed::beatsToMs(fadeOut(), beatTime);
}

void FunctionSpeeds::setBeatsFadeOut(quint32 fadeOut, float beatTime)
{
    if (tempoType() == Speed::Beats)
        setFadeOut(fadeOut);

    setFadeOut(Speed::beatsToMs(fadeOut, beatTime));
}

quint32 FunctionSpeeds::beatsDuration(float beatTime) const
{
    if (tempoType() == Speed::Beats)
        return duration();

    return Speed::beatsToMs(duration(), beatTime);
}

void FunctionSpeeds::setBeatsDuration(quint32 duration, float beatTime)
{
    if (tempoType() == Speed::Beats)
        setDuration(fadeOut());

    setDuration(Speed::beatsToMs(duration, beatTime));
}

bool FunctionSpeeds::loadXML(QXmlStreamReader &speedsRoot, QString const& nodeName)
{
    if (speedsRoot.name() != nodeName)
        return false;

    QXmlStreamAttributes attrs = speedsRoot.attributes();

    if (attrs.hasAttribute(KXMLQLCFunctionSpeedsType))
    {
        Speed::TempoType tempoType = Speed::stringToTempoType(attrs.value(KXMLQLCFunctionSpeedsFadeIn).toString());
        m_fadeIn.tempoType = tempoType;
        m_hold.tempoType = tempoType;
        m_fadeOut.tempoType = tempoType;
    }
    m_fadeIn.value = attrs.value(KXMLQLCFunctionSpeedsFadeIn).toString().toUInt();
    m_hold.value = attrs.value(KXMLQLCFunctionSpeedsHold).toString().toUInt();
    m_fadeOut.value = attrs.value(KXMLQLCFunctionSpeedsFadeOut).toString().toUInt();

    // Keep legacy workspaces compatibility
    if (attrs.hasAttribute(KXMLQLCFunctionSpeedsDuration))
        setDuration(attrs.value(KXMLQLCFunctionSpeedsDuration).toString().toUInt());

    speedsRoot.skipCurrentElement();

    return true;
}

bool FunctionSpeeds::saveXML(QXmlStreamWriter *doc, QString const& nodeName) const
{
    doc->writeStartElement(nodeName);
    doc->writeAttribute(KXMLQLCFunctionSpeedsType, Speed::tempoTypeToString(m_fadeIn.tempoType));
    doc->writeAttribute(KXMLQLCFunctionSpeedsFadeIn, QString::number(m_fadeIn.value));
    doc->writeAttribute(KXMLQLCFunctionSpeedsHold, QString::number(m_hold.value));
    doc->writeAttribute(KXMLQLCFunctionSpeedsFadeOut, QString::number(m_fadeOut.value));
    doc->writeEndElement();

    return true;
}
