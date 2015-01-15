/*
  Q Light Controller
  rgbmatrix.h

  Copyright (c) Heikki Junnila

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

#ifndef RGBMATRIX_H
#define RGBMATRIX_H

#include <QVector>
#include <QColor>
#include <QList>
#include <QSize>
#include <QPair>
#include <QHash>
#include <QMap>
#include <QMutex>

#include "rgbscript.h"
#include "function.h"

class FixtureGroup;
class GenericFader;
class FadeChannel;
class QTime;
class QDir;

/** @addtogroup engine_functions Functions
 * @{
 */

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
     * Contents
     *********************************************************************/
public:
    /** Set the matrix total duration in milliseconds */
    void setTotalDuration(quint32 msec);

    /** Get the matrix total duration in milliseconds */
    quint32 totalDuration();

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
    quint32 m_fixtureGroupID;

    /************************************************************************
     * Algorithm
     ************************************************************************/
public:
    /** Set the current RGB Algorithm. RGBMatrix takes ownership of the pointer. */
    void setAlgorithm(RGBAlgorithm* algo);

    /** Get the current RGB Algorithm. */
    RGBAlgorithm* algorithm() const;

    /** Get the algorithm protection mutex */
    QMutex& algorithmMutex();

    /** Get the number of steps of the current algorithm */
    int stepsCount();

    /** Get the preview of the current algorithm at the given step */
    RGBMap previewMap(int step);

private:
    RGBAlgorithm* m_algorithm;
    QMutex m_algorithmMutex;

    /************************************************************************
     * Color
     ************************************************************************/
public:
    void setStartColor(const QColor& c);
    QColor startColor() const;

    void setEndColor(const QColor& c);
    QColor endColor() const;

    void calculateColorDelta();
    void setStepColor(QColor color);
    QColor stepColor();
    void updateStepColor(int step);

private:
    QColor m_startColor;
    QColor m_endColor;

    /************************************************************************
     * Properties
     ************************************************************************/
public:
    /** Set the value of the property with the given name */
    void setProperty(QString propName, QString value);

    /** Retrieve the value of the property with the given name */
    QString property(QString propName);

private:
    /** A map of the custom properties for this matrix */
    QHash<QString, QString>m_properties;

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
    void write(MasterTimer* timer, QList<Universe*> universes);

    /** @reimpl */
    void postRun(MasterTimer* timer, QList<Universe*> universes);

private:
    /** Check what should be done when elapsed() >= duration() */
    void roundCheck(const QSize& size);

    /** Update new FadeChannels to m_fader when $map has changed since last time */
    void updateMapChannels(const RGBMap& map, const FixtureGroup* grp);

    /** Grab starting values for a fade channel from $fader if available */
    void insertStartValues(FadeChannel& fc, uint fadeTime) const;

private:
    Function::Direction m_direction;
    GenericFader* m_fader;
    int m_step;
    QTime* m_roundTime;
    QColor m_stepColor;
    int m_crDelta, m_cgDelta, m_cbDelta;
    int m_stepCount;

    /*********************************************************************
     * Attributes
     *********************************************************************/
public:
    /** @reimpl */
    void adjustAttribute(qreal fraction, int attributeIndex);
};

/** @} */

#endif
