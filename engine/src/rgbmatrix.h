/*
  Q Light Controller
  rgbmatrix.h

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

#ifndef RGBMATRIX_H
#define RGBMATRIX_H

#include <QVector>
#include <QColor>
#include <QList>
#include <QSize>
#include <QPair>
#include <QMap>

#include "rgbscript.h"
#include "function.h"

class FixtureGroup;
class GenericFader;
class FadeChannel;
class QTime;
class QDir;

class RGBMatrix : public Function
{
    Q_OBJECT
    Q_DISABLE_COPY(RGBMatrix)

   /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    RGBMatrix(Doc* parent);
    ~RGBMatrix();

    /*********************************************************************
     * Copying
     *********************************************************************/
public:
    /** @reimpl */
    virtual Function* createCopy(Doc* doc, bool addToDoc = true);

    /** @reimpl */
    virtual bool copyFrom(const Function* function);

    /************************************************************************
     * Fixture Group
     ************************************************************************/
public:
    void setFixtureGroup(quint32 id);
    quint32 fixtureGroup() const;

private:
    quint32 m_fixtureGroup;

    /************************************************************************
     * Algorithm
     ************************************************************************/
public:
    /** Set the current RGB Algorithm. RGBMatrix takes ownership of the pointer. */
    void setAlgorithm(RGBAlgorithm* algo);

    /** Get the current RGB Algorithm. */
    RGBAlgorithm* algorithm() const;

    /** Get a list of RGBMap steps for preview purposes, using the current algorithm. */
    QList <RGBMap> previewMaps();

private:
    RGBAlgorithm* m_algorithm;

    /************************************************************************
     * Colour
     ************************************************************************/
public:
    void setMonoColor(const QColor& c);
    QColor monoColor() const;

private:
    QColor m_monoColor;

    /************************************************************************
     * Load & Save
     ************************************************************************/
public:
    /** @reimpl */
    bool loadXML(const QDomElement& root);

    /** @reimpl */
    bool saveXML(QDomDocument* doc, QDomElement* root);

    /************************************************************************
     * Running
     ************************************************************************/
public:
    /** @reimpl */
    void tap();

    /** @reimpl */
    void preRun(MasterTimer* timer);

    /** @reimpl */
    void write(MasterTimer* timer, UniverseArray* universes);

    /** @reimpl */
    void postRun(MasterTimer* timer, UniverseArray* universes);

private:
    /** Check what should be done when elapsed() >= duration() */
    void roundCheck(const QSize& size);

    /** Update new FadeChannels to m_fader when $map has changed since last time */
    void updateMapChannels(const RGBMap& map, const FixtureGroup* grp);

    /** Grab starting values for a fade channel from $fader if available */
    void insertStartValues(FadeChannel& fc) const;

private:
    Function::Direction m_direction;
    GenericFader* m_fader;
    int m_step;
    QTime* m_roundTime;
};

#endif
