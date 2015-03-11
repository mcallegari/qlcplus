/*
  Q Light Controller
  vcspeeddial.h

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

#ifndef VCSPEEDDIAL_H
#define VCSPEEDDIAL_H

#include <QSet>

#include "vcwidget.h"

class QDomDocument;
class QDomElement;
class SpeedDial;
class VCSpeedDialFunction;

/** @addtogroup ui_vc_props
 * @{
 */

#define KXMLQLCVCSpeedDial "SpeedDial"
#define KXMLQLCVCSpeedDialSpeedTypes "SpeedTypes"
#define KXMLQLCVCSpeedDialAbsoluteValue "AbsoluteValue"
#define KXMLQLCVCSpeedDialAbsoluteValueMin "Minimum"
#define KXMLQLCVCSpeedDialAbsoluteValueMax "Maximum"
#define KXMLQLCVCSpeedDialTap "Tap"
#define KXMLQLCVCSpeedDialTapKey "Key"
#define KXMLQLCVCSpeedDialVisibilityMask "Visibility"

class VCSpeedDial : public VCWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(VCSpeedDial)

public:
    static const quint8 absoluteInputSourceId;
    static const quint8 tapInputSourceId;
    static const QSize defaultSize;

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    VCSpeedDial(QWidget* parent, Doc* doc);
    ~VCSpeedDial();

    /** @reimp */
    virtual void enableWidgetUI(bool enable);

    /*************************************************************************
     * Clipboard
     *************************************************************************/
public:
    /** @reimp */
    VCWidget* createCopy(VCWidget* parent);

protected:
    /** @reimp */
    bool copyFrom(const VCWidget* widget);

    /*************************************************************************
     * Caption
     *************************************************************************/
public:
    /** @reimp */
    void setCaption(const QString& text);

    /*************************************************************************
     * QLC Mode
     *************************************************************************/
public slots:
    /** @reimp */
    void slotModeChanged(Doc::Mode mode);

    /*************************************************************************
     * Properties
     *************************************************************************/
public:
    /** @reimp */
    void editProperties();

     /************************************************************************
     * Speed Type
     ************************************************************************/
public:
    enum SpeedType
    {
        FadeIn   = 1 << 0,
        FadeOut  = 1 << 1,
        Duration = 1 << 2
    };
    Q_DECLARE_FLAGS(SpeedTypes, SpeedType)

private:
    SpeedTypes m_speedTypes;

    /************************************************************************
     * Functions
     ************************************************************************/
public:
    /**
     * Set the list of functions that are controlled by the dial.
     */
    void setFunctions(const QList <VCSpeedDialFunction> functions);

    /**
     * Get the list of functions that are controlled by the dial.
     */
    QList <VCSpeedDialFunction> functions() const;

    /** Simulate tap button press
     */
    void tap();

private slots:
    /** Catch dial value changes and patch them to controlled functions */
    void slotDialValueChanged(int ms);

    /** Catch dial tap button clicks and patch them to controlled functions */
    void slotDialTapped();

private:
    QList <VCSpeedDialFunction> m_functions;
    SpeedDial* m_dial;

    /*********************************************************************
     * External input
     *********************************************************************/
public:
    /** @reimp */
    void updateFeedback();

protected slots:
    /** @reimp */
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value);

    /*********************************************************************
     * Tap key sequence handler
     *********************************************************************/
public:
    void setKeySequence(const QKeySequence& keySequence);
    QKeySequence keySequence() const;

protected slots:
    void slotKeyPressed(const QKeySequence& keySequence);

protected:
    QKeySequence m_tapKeySequence;

    /************************************************************************
     * Absolute value range
     ************************************************************************/
public:
    void setAbsoluteValueRange(uint min, uint max);
    uint absoluteValueMin() const;
    uint absoluteValueMax() const;

private:
    uint m_absoluteValueMin;
    uint m_absoluteValueMax;

    /*************************************************************************
     * Elements visibility
     *************************************************************************/
public:
    /** Return the widget's elements visibility bitmask */
    ushort visibilityMask();

    /** Set the visibility of the widget's elements
      * according to the provided bitmask */
    void setVisibilityMask(ushort mask);

private:
    ushort m_visibilityMask;

    /*************************************************************************
     * Load & Save
     *************************************************************************/
public:
    /** @reimp */
    bool loadXML(const QDomElement* root);

    /** @reimp */
    bool saveXML(QDomDocument* doc, QDomElement* vc_root);

    /** @reimp */
    void postLoad();
};
// Deprecated: used for loading old workspace files
Q_DECLARE_OPERATORS_FOR_FLAGS(VCSpeedDial::SpeedTypes)

/** @} */

#endif
