/*
  Q Light Controller
  vcxypad.h

  Copyright (c) Stefan Krumm, Heikki Junnila

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
#include "dmxsource.h"
#include "vcwidget.h"

class ctkRangeSlider;
class QDomDocument;
class QDomElement;
class QPaintEvent;
class QMouseEvent;
class MasterTimer;
class VCXYPadArea;
class QHBoxLayout;
class QVBoxLayout;
class QByteArray;
class QSlider;
class Doc;

#define KXMLQLCVCXYPad "XYPad"
#define KXMLQLCVCXYPadPan "Pan"
#define KXMLQLCVCXYPadTilt "Tilt"
#define KXMLQLCVCXYPadPosition "Position"
#define KXMLQLCVCXYPadRangeWindow "Window"
#define KXMLQLCVCXYPadRangeHorizMin "hMin"
#define KXMLQLCVCXYPadRangeHorizMax "hMax"
#define KXMLQLCVCXYPadRangeVertMin "vMin"
#define KXMLQLCVCXYPadRangeVertMax "vMax"

#define KXMLQLCVCXYPadPositionX "X" // Legacy
#define KXMLQLCVCXYPadPositionY "Y" // Legacy

#define KXMLQLCVCXYPadInvertedAppearance "InvertedAppearance"

class VCXYPad : public VCWidget, public DMXSource
{
    Q_OBJECT
    Q_DISABLE_COPY(VCXYPad)

public:
    static const quint8 panInputSourceId;
    static const quint8 tiltInputSourceId;

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    VCXYPad(QWidget* parent, Doc* doc);
    virtual ~VCXYPad();

private:
    QHBoxLayout* m_hbox;
    QVBoxLayout* m_lvbox; // left vertical box
    QVBoxLayout* m_cvbox; // center vertical box
    QVBoxLayout* m_rvbox; // right vertical box
    QSlider* m_vSlider;
    QSlider* m_hSlider;
    ctkRangeSlider *m_vRangeSlider;
    ctkRangeSlider *m_hRangeSlider;
    VCXYPadArea* m_area;

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
    QList <VCXYPadFixture> m_fixtures;

    /*************************************************************************
     * Current position
     *************************************************************************/
public:
    /** @reimp */
    void writeDMX(MasterTimer* timer, UniverseArray* universes);

public slots:
    void slotPositionChanged(const QPoint& pt);
    void slotSliderValueChanged();
    void slotRangeValueChanged();

private:
    bool m_padInteraction;
    bool m_sliderInteraction;
    bool m_inputValueChanged;

    /*********************************************************************
     * External input
     *********************************************************************/
public:
    void updateFeedback();

protected slots:
    /** Called when an external input device produces input data */
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value);

    /*************************************************************************
     * QLC mode
     *************************************************************************/
protected slots:
    /** @reimp */
    void slotModeChanged(Doc::Mode mode);

    /*************************************************************************
     * Load & Save
     *************************************************************************/
public:
    /** @reimp */
    bool loadXML(const QDomElement* root);

    /** @reimp */
    bool saveXML(QDomDocument* doc, QDomElement* root);
};

#endif
