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
    void calculateColorDelta(QColor startColor, QColor endColor, RGBAlgorithm *algorithm);

    /** Set/Get the final color of the next step to be reproduced */
    void setStepColor(QColor color);
    QColor stepColor();

    /** Update the color of the next step to be reproduced, considering the step index,
     *  the start color and the steps count */
    void updateStepColor(int step, QColor startColor, int stepsCount);

    /** Initialize the playback direction and set the initial step index and
      * color based on $startColor and $endColor */
    void initializeDirection(Function::Direction direction, QColor startColor, QColor endColor, int stepsCount, RGBAlgorithm *algorithm);

    /** Check the steps progression based on $order and the internal m_direction.
     *  This method returns true if the RGBMatrix can continue to run, otherwise
     *  false is returned and the caller should stop the RGBMatrix */
    bool checkNextStep(Function::RunOrder order, QColor startColor, QColor endColor, int stepsNumber);

public:
    /** Matrix RGB data of the current step */
    RGBMap m_map;

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

    /** @reimp */
    QIcon getIcon() const;

    /*********************************************************************
     * Contents
     *********************************************************************/
public:
    /** @reimp */
    void setTotalDuration(quint32 msec);

    /** @reimp */
    quint32 totalDuration();

    /** Set the matrix to control or not the dimmer channel */
    void setDimmerControl(bool dimmerControl);

    /** Get the matrix ability to control the dimmer channel */
    bool dimmerControl() const;

private:
    // LEGACY: replaced by ControlModeDimmer
    bool m_dimmerControl;

    /*********************************************************************
     * Copying
     *********************************************************************/
public:
    /** @reimp */
    virtual Function* createCopy(Doc* doc, bool addToDoc = true);

    /** @reimp */
    virtual bool copyFrom(const Function* function);

    /************************************************************************
     * Fixture Group
     ************************************************************************/
public:
    /** Get/Set the Fixture Group associated to this RGBMatrix */
    quint32 fixtureGroup() const;
    void setFixtureGroup(quint32 id);

    /** @reimp */
    QList<quint32> components();

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
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QMutex& algorithmMutex();
#else
    QRecursiveMutex& algorithmMutex();
#endif

    /** Get the number of steps of the current algorithm */
    int stepsCount();

    /** Get the preview of the current algorithm at the given step */
    void previewMap(int step, RGBMatrixStep *handler);

private:
    RGBAlgorithm *m_algorithm;
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QMutex m_algorithmMutex;
#else
    QRecursiveMutex m_algorithmMutex;
#endif

    /************************************************************************
     * Color
     ************************************************************************/
public:
    void setColor(int i, QColor c);
    QColor getColor(int i) const;
    QVector <QColor> getColors() const;

    void updateColorDelta();

    /** Set the colors of the current algorithm */
    void setMapColors();

private:
    QVector<QColor> m_rgbColors;
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
    /** @reimp */
    bool loadXML(QXmlStreamReader &root);

    /** @reimp */
    bool saveXML(QXmlStreamWriter *doc);

    /************************************************************************
     * Running
     ************************************************************************/
public:
    /** @reimp */
    void tap();

    /** @reimp */
    void preRun(MasterTimer *timer);

    /** @reimp */
    void write(MasterTimer *timer, QList<Universe*> universes);

    /** @reimp */
    void postRun(MasterTimer *timer, QList<Universe*> universes);

private:
    /** Check what should be done when elapsed() >= duration() */
    void roundCheck();

    FadeChannel *getFader(Universe *universe, quint32 fixtureID, quint32 channel);
    void updateFaderValues(FadeChannel *fc, uchar value, uint fadeTime);

    /** Update FadeChannels when $map has changed since last time */
    void updateMapChannels(const RGBMap& map, const FixtureGroup* grp, QList<Universe *> universes);

public:
    /** Convert color values to fader value */
    static uchar rgbToGrey(uint col);

private:
    /** Reference to a timer counting the time in ms between steps */
    QElapsedTimer *m_roundTime;

    /** The number of steps returned by the currently loaded algorithm */
    int m_stepsCount;

    /** The duration of a step based on the current BPM (Beats tempo only) */
    uint m_stepBeatDuration;

    /*********************************************************************
     * Attributes
     *********************************************************************/
public:
    /** @reimp */
    int adjustAttribute(qreal fraction, int attributeId);

    /*************************************************************************
     * Blend mode
     *************************************************************************/
public:
    /** @reimp */
    void setBlendMode(Universe::BlendMode mode);

    /*************************************************************************
     * Control Mode
     *************************************************************************/
public:
    /** Control modes for the RGB Matrix */
    enum ControlMode
    {
        ControlModeRgb = 0,
        ControlModeWhite,
        ControlModeAmber,
        ControlModeUV,
        ControlModeDimmer,
        ControlModeShutter
    };

    /** Get/Set the control mode associated to this RGBMatrix */
    ControlMode controlMode() const;
    void setControlMode(ControlMode mode);

    /** Return a control mode from a string */
    static ControlMode stringToControlMode(QString mode);

    /** Return a string from a control mode, to be saved into a XML */
    static QString controlModeToString(ControlMode mode);

private:
    ControlMode m_controlMode;
};

/** @} */

#endif
