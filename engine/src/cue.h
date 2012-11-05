/*
  Q Light Controller
  cue.h

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef CUE_H
#define CUE_H

#include <QString>
#include <QHash>

#include "scenevalue.h"

#define KXMLQLCCue "Cue"
#define KXMLQLCCueName "Name"
#define KXMLQLCCueValue "Value"
#define KXMLQLCCueValueChannel "Channel"

#define KXMLQLCCueSpeed         "Speed"
#define KXMLQLCCueSpeedFadeIn   "FadeIn"
#define KXMLQLCCueSpeedFadeOut  "FadeOut"
#define KXMLQLCCueSpeedDuration "Duration"

class QDomDocument;
class QDomElement;

class Cue
{
public:
    Cue(const QString& name = QString());
    Cue(const QHash <uint,uchar> values);
    Cue(const Cue& cue);
    ~Cue();

    /************************************************************************
     * Name
     ************************************************************************/
public:
    void setName(const QString& str);
    QString name() const;

private:
    QString m_name;

    /************************************************************************
     * Values
     ************************************************************************/
public:
    void setValue(uint channel, uchar value);
    void unsetValue(uint channel);
    uchar value(uint channel) const;

    QHash <uint,uchar> values() const;

private:
    QHash <uint,uchar> m_values;

    /************************************************************************
     * Speed
     ************************************************************************/
public:
    void setFadeInSpeed(uint ms);
    uint fadeInSpeed() const;

    void setFadeOutSpeed(uint ms);
    uint fadeOutSpeed() const;

    void setDuration(uint ms);
    uint duration() const;

private:
    uint m_fadeInSpeed;
    uint m_fadeOutSpeed;
    uint m_duration;

    /************************************************************************
     * Load & Save
     ************************************************************************/
public:
    bool loadXML(const QDomElement& root);
    bool saveXML(QDomDocument* doc, QDomElement* stack_root) const;

private:
    bool loadXMLSpeed(const QDomElement& speedRoot);
    bool saveXMLSpeed(QDomDocument* doc, QDomElement* root) const;
};

#endif
