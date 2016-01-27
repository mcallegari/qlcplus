/*
  Q Light Controller Plus
  vcxypad.h

  Copyright (c) Stefan Krumm
                Heikki Junnila
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

#ifndef VCXYPAD_H
#define VCXYPAD_H

#include <QWidget>
#include <QPixmap>
#include <QString>
#include <QMutex>
#include <QList>

#include "vcxypadfixture.h"
#include "vcxypadpreset.h"
#include "dmxsource.h"
#include "vcwidget.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class ctkRangeSlider;
class QPaintEvent;
class QMouseEvent;
class MasterTimer;
class VCXYPadArea;
class QHBoxLayout;
class QVBoxLayout;
class FlowLayout;
class QByteArray;
class QSlider;
class EFX;
class Doc;

/** @addtogroup ui_vc_widgets
 * @{
 */

#define KXMLQLCVCXYPad "XYPad"
#define KXMLQLCVCXYPadPan "Pan"
#define KXMLQLCVCXYPadTilt "Tilt"
#define KXMLQLCVCXYPadWidth "Width"
#define KXMLQLCVCXYPadHeight "Height"
#define KXMLQLCVCXYPadPosition "Position"
#define KXMLQLCVCXYPadRangeWindow "Window"
#define KXMLQLCVCXYPadRangeHorizMin "hMin"
#define KXMLQLCVCXYPadRangeHorizMax "hMax"
#define KXMLQLCVCXYPadRangeVertMin "vMin"
#define KXMLQLCVCXYPadRangeVertMax "vMax"

#define KXMLQLCVCXYPadPositionX "X" // Legacy
#define KXMLQLCVCXYPadPositionY "Y" // Legacy

#define KXMLQLCVCXYPadInvertedAppearance "InvertedAppearance"

typedef struct
{
    quint32 m_universe;
    quint32 m_fixture;
    quint32 m_channel; // universe channel address
    QLCChannel::Group m_group;
    QLCChannel::ControlByte m_subType;
} SceneChannel;

class VCXYPad : public VCWidget, public DMXSource
{
    Q_OBJECT
    Q_DISABLE_COPY(VCXYPad)

public:
    static const quint8 panInputSourceId;
    static const quint8 tiltInputSourceId;
    static const quint8 widthInputSourceId;
    static const quint8 heightInputSourceId;

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    VCXYPad(QWidget* parent, Doc* doc);
    virtual ~VCXYPad();

    /** @reimp */
    void enableWidgetUI(bool enable);

private:
    QVBoxLayout* m_mainVbox;  // main vertical layout
    QHBoxLayout* m_padBox; // box containing sliders and XYPad
    QVBoxLayout* m_lvbox; // left vertical box (vertical ctkSlider)
    QVBoxLayout* m_cvbox; // center vertical box (horizontal ctkSlider + XYPad + horizontal slider)
    QVBoxLayout* m_rvbox; // right vertical box (vertical slider)
    QSlider* m_vSlider; // tilt slider
    QSlider* m_hSlider; // pan slider
    ctkRangeSlider *m_vRangeSlider; // range window height control
    ctkRangeSlider *m_hRangeSlider; // range window width control
    VCXYPadArea* m_area;
    FlowLayout *m_presetsLayout;

    /*************************************************************************
     * Clipboard
     *************************************************************************/
public:
    /** @reimp */
    VCWidget* createCopy(VCWidget* parent);

    /** @reimp */
    bool copyFrom(const VCWidget* widget);

    /*************************************************************************
     * Caption
     *************************************************************************/
public:
    /** @reimp */
    void setCaption(const QString& text);

    /*********************************************************************
     * Y-Axis Inverted appearance
     *********************************************************************/
public:
    bool invertedAppearance() const;
    void setInvertedAppearance(bool invert);

    /*************************************************************************
     * Properties
     *************************************************************************/
public:
    /** @reimp */
    void editProperties();

    /*************************************************************************
     * Fixtures
     *************************************************************************/
public:
    /**
     * Append a new fixture to the XY pad's list of controlled fixtures
     *
     * @param fxi The fixture to append
     */
    void appendFixture(const VCXYPadFixture& fxi);

    /**
     * Remove a fixture by its ID from the XY pad's control list
     */
    void removeFixture(GroupHead const & head);

    /**
     * Remove all currently controlled fixtures from the XY pad
     */
    void clearFixtures();

    /**
     * Get a list of the pad's currently controlled fixtures
     */
    QList <VCXYPadFixture> fixtures() const;

private:
    QRectF computeCommonDegreesRange() const;
    void updateDegreesRange();

private:
    QList <VCXYPadFixture> m_fixtures;

    /*************************************************************************
     * Current position
     *************************************************************************/
public:
    /** @reimp */
    void writeDMX(MasterTimer* timer, QList<Universe*> universes);

protected:
    void writeXYFixtures(MasterTimer* timer, QList<Universe*> universes);

public slots:
    void slotPositionChanged(const QPointF& pt);
    void slotSliderValueChanged();
    void slotRangeValueChanged();

signals:
    void fixturePositions(const QVariantList positions);

private:
    bool m_padInteraction;
    bool m_sliderInteraction;
    bool m_inputValueChanged;

    /*********************************************************************
     * Presets
     *********************************************************************/
public:
    void addPreset(VCXYPadPreset const& preset);
    void resetPresets();
    QList<VCXYPadPreset *> presets() const;

protected:
    void writeScenePositions(MasterTimer* timer, QList<Universe*> universes);

protected slots:
    void slotPresetClicked(bool checked);

private:
    Function::Source functionSource() const;

protected:
    QHash<QWidget *, VCXYPadPreset *> m_presets;
    EFX *m_efx;
    Scene *m_scene;
    QList<SceneChannel> m_sceneChannels;

    /*********************************************************************
     * External input
     *********************************************************************/
public:
    void updateFeedback();

protected slots:
    /** Called when an external input device produces input data */
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value);
    void slotKeyPressed(const QKeySequence& keySequence);

    /*************************************************************************
     * QLC+ mode
     *************************************************************************/
protected slots:
    /** @reimp */
    void slotModeChanged(Doc::Mode mode);

    /*************************************************************************
     * Load & Save
     *************************************************************************/
public:
    /** @reimp */
    bool loadXML(QXmlStreamReader &root);

    /** @reimp */
    bool saveXML(QXmlStreamWriter *doc);
};

/** @} */

#endif
