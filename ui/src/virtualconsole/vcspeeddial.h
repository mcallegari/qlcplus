/*
  Q Light Controller Plus
  vcspeeddial.h

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

#ifndef VCSPEEDDIAL_H
#define VCSPEEDDIAL_H

#include <QSet>

#include "vcwidget.h"

class VCSpeedDialFunction;
class VCSpeedDialPreset;
class QXmlStreamReader;
class QXmlStreamWriter;
class QPushButton;
class QToolButton;
class FlowLayout;
class SpeedDial;
class QLabel;

/** @addtogroup ui_vc_props
 * @{
 */

#define KXMLQLCVCSpeedDial                  QString("SpeedDial")
#define KXMLQLCVCSpeedDialSpeedTypes        QString("SpeedTypes")
#define KXMLQLCVCSpeedDialAbsoluteValue     QString("AbsoluteValue")
#define KXMLQLCVCSpeedDialAbsoluteValueMin  QString("Minimum")
#define KXMLQLCVCSpeedDialAbsoluteValueMax  QString("Maximum")
#define KXMLQLCVCSpeedDialTap               QString("Tap")
#define KXMLQLCVCSpeedDialMult              QString("Mult")
#define KXMLQLCVCSpeedDialDiv               QString("Div")
#define KXMLQLCVCSpeedDialMultDivReset      QString("MultDivReset")
#define KXMLQLCVCSpeedDialApply             QString("Apply")
#define KXMLQLCVCSpeedDialTapKey            QString("Key")
#define KXMLQLCVCSpeedDialMultKey           QString("MultKey")
#define KXMLQLCVCSpeedDialDivKey            QString("DivKey")
#define KXMLQLCVCSpeedDialMultDivResetKey   QString("MultDivResetKey")
#define KXMLQLCVCSpeedDialApplyKey          QString("ApplyKey")
#define KXMLQLCVCSpeedDialResetFactorOnDialChange QString("ResetFactorOnDialChange")
#define KXMLQLCVCSpeedDialVisibilityMask    QString("Visibility")
#define KXMLQLCVCSpeedDialTime              QString("Time")

// Legacy: infinite checkbox
#define KXMLQLCVCSpeedDialInfinite      QString("Infinite")
#define KXMLQLCVCSpeedDialInfiniteKey   QString("InfiniteKey")

class VCSpeedDial : public VCWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(VCSpeedDial)

public:
    // 0xffff mask is used by SpeedDial.
    // VCSpeedDial uses the 0xffff0000 mask.
    enum Visibility
    {
        MultDiv    = 0x10000 << 0,
        Apply      = 0x10000 << 1,
    };

    static const quint8 absoluteInputSourceId;
    static const quint8 tapInputSourceId;
    static const quint8 multInputSourceId;
    static const quint8 divInputSourceId;
    static const quint8 multDivResetInputSourceId;
    static const quint8 applyInputSourceId;
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

    /*********************************************************************
     * Background/Foreground color
     *********************************************************************/
public:
    /** @reimp */
    void setFont(const QFont& font);

    /** @reimp */
    void setBackgroundColor(const QColor& color);

    /** @reimp */
    void setForegroundColor(const QColor& color);

    /** @reimp */
    QColor foregroundColor() const;

private:
    QColor m_foregroundColor;
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
    void setFunctions(const QList <VCSpeedDialFunction> & functions);

    /**
     * Get the list of functions that are controlled by the dial.
     */
    QList <VCSpeedDialFunction> functions() const;

    /** Simulate tap button press
     */
    void tap();

private slots:
    /** Catch dial value changes and patch them to controlled functions */
    void slotDialValueChanged();

    /** Catch dial tap button clicks and patch them to controlled functions */
    void slotDialTapped();

    /** Catch dial tap timeouts to send feedback */
    void slotTapTimeout();

private:
    QList <VCSpeedDialFunction> m_functions;
    SpeedDial* m_dial;
    QToolButton* m_multButton;
    QLabel* m_multDivLabel;
    QToolButton* m_divButton;
    QToolButton* m_multDivResetButton;
    QLabel* m_multDivResultLabel;
    QPushButton* m_applyButton;
    FlowLayout* m_presetsLayout;

protected slots:
    void slotMult();
    void slotDiv();
    void slotMultDivReset();
    void slotMultDivChanged();
    void slotFactoredValueChanged();

private:
    qint32 m_currentFactor;
    qint32 m_factoredValue;
    bool m_resetFactorOnDialChange;

public:
    void setResetFactorOnDialChange(bool value);
    bool resetFactorOnDialChange() const;

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
     * Tap & presets key sequence handler
     *********************************************************************/
public:
    void setTapKeySequence(const QKeySequence& keySequence);
    QKeySequence tapKeySequence() const;
    void setMultKeySequence(const QKeySequence& keySequence);
    QKeySequence multKeySequence() const;
    void setDivKeySequence(const QKeySequence& keySequence);
    QKeySequence divKeySequence() const;
    void setMultDivResetKeySequence(const QKeySequence& keySequence);
    QKeySequence multDivResetKeySequence() const;
    void setApplyKeySequence(const QKeySequence& keySequence);
    QKeySequence applyKeySequence() const;

protected slots:
    void slotKeyPressed(const QKeySequence& keySequence);

protected:
    QKeySequence m_tapKeySequence;
    QKeySequence m_multKeySequence;
    QKeySequence m_divKeySequence;
    QKeySequence m_multDivResetKeySequence;
    QKeySequence m_applyKeySequence;

    /************************************************************************
     * Absolute value range
     ************************************************************************/
public:
    void setAbsoluteValueRange(uint min, uint max);
    uint absoluteValueMin() const;
    uint absoluteValueMax() const;

private:
    quint32 m_absoluteValueMin;
    quint32 m_absoluteValueMax;

    /*************************************************************************
     * Elements visibility
     *************************************************************************/
public:
    /** Return the widget's elements visibility bitmask */
    quint32 visibilityMask() const;

    /** Set the visibility of the widget's elements
      * according to the provided bitmask */
    void setVisibilityMask(quint32 mask);

private:
    quint32 m_visibilityMask;

    /*********************************************************************
     * Presets
     *********************************************************************/
public:
    void addPreset(VCSpeedDialPreset const& preset);
    void resetPresets();
    QList<VCSpeedDialPreset *> presets() const;

protected slots:
    void slotPresetClicked();

protected:
    QHash<QWidget*, VCSpeedDialPreset*> m_presets;

private slots:
    void slotUpdate();

private:
    /** timer for updating the preset buttons */
    QTimer* m_updateTimer;

    /*************************************************************************
     * Load & Save
     *************************************************************************/
public:
    /** @reimp */
    bool loadXML(QXmlStreamReader &root);

    bool loadXMLInfiniteLegacy(QXmlStreamReader &root, QSharedPointer<VCSpeedDialPreset> preset);

    /** @reimp */
    bool saveXML(QXmlStreamWriter *doc);

    /** @reimp */
    void postLoad();
};
// Deprecated: used for loading old workspace files
Q_DECLARE_OPERATORS_FOR_FLAGS(VCSpeedDial::SpeedTypes)

/** @} */

#endif
