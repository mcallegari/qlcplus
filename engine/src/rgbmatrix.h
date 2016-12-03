/*
  Q Light Controller Plus
  rgbmatrix.h

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

#ifdef QT_QML_LIB
  #include "rgbscriptv4.h"
#else
  #include "rgbscript.h"
#endif
#include "function.h"

class QElapsedTimer;
class FixtureGroup;
class GenericFader;
class FadeChannel;
class QDir;

/** @addtogroup engine_functions Functions
 * @{
 */

class RGBMatrixStep
{
public:
    RGBMatrixStep();
    ~RGBMatrixStep() { }

public:
    /** Set/Get the current step index */
    void setCurrentStepIndex(int index);
    int currentStepIndex() const;

    /** Calculate the RGB components delta between $startColor and $endColor */
    void calculateColorDelta(QColor startColor, QColor endColor);

    /** Set/Get the final color of the next step to be reproduced */
    void setStepColor(QColor color);
    QColor stepColor();

    /** Update the color of the next step to be reproduced, considering the step index,
     *  the start color and the steps count */
    void updateStepColor(int step, QColor startColor, int stepsCount);

    /** Initialize the playback direction and set the initial step index and
      * color based on $startColor and $endColor */
    void initializeDirection(Function::Direction direction, QColor startColor, QColor endColor, int stepsCount);

    /** Check the steps progression based on $order and the internal m_direction.
     *  This method returns true if the RGBMatrix can continue to run, otherwise
     *  false is returned and the caller should stop the RGBMatrix */
    bool checkNextStep(Function::RunOrder order, QColor startColor, QColor endColor, int stepsNumber);

private:
    /** The current direction of the steps playback */
    Function::Direction m_direction;
    /** The index of the algorithm step currently being reproduced */
    int m_currentStepIndex;
    /** The RGB color passed to the currently loaded algorithm */
    QColor m_stepColor;
    /** Color delta values of the RGB components between each step */
    int m_crDelta, m_cgDelta, m_cbDelta;
};

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
    /** @reimpl */
    void setTotalDuration(quint32 msec);

    /** @reimpl */
    quint32 totalDuration();

    /** Set the matrix to control or not the dimmer channel */
    void setDimmerControl(bool dimmerControl);

    /** Get the matrix ability to control the dimmer channel */
    bool dimmerControl() const;

private:
    bool m_dimmerControl;

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
    FixtureGroup *m_group;

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
    RGBMap previewMap(int step, RGBMatrixStep *handler);

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

    void updateColorDelta();

private:
    QColor m_startColor;
    QColor m_endColor;
    RGBMatrixStep *m_stepHandler;

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
    bool loadXML(QXmlStreamReader &root);

    /** @reimpl */
    bool saveXML(QXmlStreamWriter *doc);

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
    void roundCheck();

    /** Update new FadeChannels to m_fader when $map has changed since last time */
    void updateMapChannels(const RGBMap& map, const FixtureGroup* grp);

    /** Grab starting values for a fade channel from $fader if available */
    void insertStartValues(FadeChannel& fc, uint fadeTime) const;

private:
    /** Reference of a GenericFader in charge of actually sending DMX data
     *  of the current RGB Matrix step, including fade transitions */
    GenericFader* m_fader;

    /** Reference to a timer counting the time in ms between steps */
    QElapsedTimer* m_roundTime;

    /** The number of steps returned by the currently loaded algorithm */
    int m_stepsCount;

    /** The duration of a step based on the current BPM (Beats tempo only) */
    uint m_stepBeatDuration;

    /*********************************************************************
     * Attributes
     *********************************************************************/
public:
    /** @reimpl */
    void adjustAttribute(qreal fraction, int attributeIndex);

    /*************************************************************************
     * Blend mode
     *************************************************************************/
public:
    /** @reimpl */
    void setBlendMode(Universe::BlendMode mode);
};

/** @} */

#endif
