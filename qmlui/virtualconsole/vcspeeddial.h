/*
  Q Light Controller Plus
  vcspeeddial.h

  Copyright (c) Massimo Callegari

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

#include "vcwidget.h"

#define KXMLQLCVCSpeedDial                  QStringLiteral("SpeedDial")
#define KXMLQLCVCSpeedDialAbsoluteValue     QStringLiteral("AbsoluteValue")
#define KXMLQLCVCSpeedDialAbsoluteValueMin  QStringLiteral("Minimum")
#define KXMLQLCVCSpeedDialAbsoluteValueMax  QStringLiteral("Maximum")
#define KXMLQLCVCSpeedDialTap               QStringLiteral("Tap")
#define KXMLQLCVCSpeedDialMult              QStringLiteral("Mult")
#define KXMLQLCVCSpeedDialDiv               QStringLiteral("Div")
#define KXMLQLCVCSpeedDialMultDivReset      QStringLiteral("MultDivReset")
#define KXMLQLCVCSpeedDialApply             QStringLiteral("Apply")
#define KXMLQLCVCSpeedDialTapKey            QStringLiteral("Key")
#define KXMLQLCVCSpeedDialMultKey           QStringLiteral("MultKey")
#define KXMLQLCVCSpeedDialDivKey            QStringLiteral("DivKey")
#define KXMLQLCVCSpeedDialMultDivResetKey   QStringLiteral("MultDivResetKey")
#define KXMLQLCVCSpeedDialApplyKey          QStringLiteral("ApplyKey")
#define KXMLQLCVCSpeedDialResetFactorOnDialChange QStringLiteral("ResetFactorOnDialChange")
#define KXMLQLCVCSpeedDialVisibilityMask    QStringLiteral("Visibility")
#define KXMLQLCVCSpeedDialTime              QStringLiteral("Time")
#define KXMLQLCVCSpeedDialFunction          QStringLiteral("Function")

class VCSpeedDial : public VCWidget
{
    Q_OBJECT

    Q_PROPERTY(quint32 visibilityMask READ visibilityMask WRITE setVisibilityMask NOTIFY visibilityMaskChanged)
    Q_PROPERTY(uint timeMinimumValue READ timeMinimumValue WRITE setTimeMinimumValue NOTIFY timeMinimumValueChanged FINAL)
    Q_PROPERTY(uint timeMaximumValue READ timeMaximumValue WRITE setTimeMaximumValue NOTIFY timeMaximumValueChanged FINAL)
    Q_PROPERTY(uint currentTime READ currentTime WRITE setCurrentTime NOTIFY currentTimeChanged FINAL)
    Q_PROPERTY(bool resetOnDialChange READ resetOnDialChange WRITE setResetOnDialChange NOTIFY resetOnDialChangeChanged FINAL)
    Q_PROPERTY(SpeedMultiplier currentFactor READ currentFactor WRITE setCurrentFactor NOTIFY currentFactorChanged FINAL)
    Q_PROPERTY(int tapTimeValue READ tapTimeValue NOTIFY tapTimeValueChanged FINAL)

    Q_PROPERTY(QVariant functionsList READ functionsList NOTIFY functionsListChanged)
    Q_PROPERTY(QVariantList presetsList READ presetsList NOTIFY presetsListChanged)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCSpeedDial(Doc* doc = nullptr, QObject *parent = nullptr);
    virtual ~VCSpeedDial();

    /** @reimp */
    QString defaultCaption() const override;

    /** @reimp */
    void setupLookAndFeel(qreal pixelDensity, int page) override;

    /** @reimp */
    void render(QQuickView *view, QQuickItem *parent) override;

    /** @reimp */
    QString propertiesResource() const override;
    QString presetsResource() const override;
    bool supportsPresets() const override;

    /** @reimp */
    VCWidget *createCopy(VCWidget *parent) const override;

    enum Visibility
    {
        Nothing      = 0,
        PlusMinus    = 1 << 0,
        Dial         = 1 << 1,
        Tap          = 1 << 2,
        Hours        = 1 << 3,
        Minutes      = 1 << 4,
        Seconds      = 1 << 5,
        Milliseconds = 1 << 6,
        Multipliers  = 1 << 7,
        Apply        = 1 << 8,
        Beats        = 1 << 9,
        XPad         = 1 << 10
    };
    Q_ENUM(Visibility)

    enum SpeedMultiplier
    {
        None = 0,
        Zero,
        OneSixteenth,
        OneEighth,
        OneFourth,
        Half,
        One,
        Two,
        Four,
        Eight,
        Sixteen
    };
    Q_ENUM(SpeedMultiplier)

    typedef struct
    {
        quint32 m_fId;
        SpeedMultiplier m_fadeInFactor;
        SpeedMultiplier m_fadeOutFactor;
        SpeedMultiplier m_durationFactor;
    } VCSpeedDialFunction;

protected:
    /** @reimp */
    bool copyFrom(const VCWidget* widget) override;

private:
    void cacheMultipliers();

private:
    QVector<float> m_multiplierCache;

    /*********************************************************************
     * UI elements visibility
     *********************************************************************/
public:
    /** Get/Set the widget's elements visibility bitmask */
    quint32 visibilityMask() const;
    void setVisibilityMask(quint32 mask);

signals:
    void visibilityMaskChanged();

private:
    quint32 m_visibilityMask;

    /*********************************************************************
     * Dial absolute time
     *********************************************************************/
public:
    /* Get/Set the time range minimum value */
    uint timeMinimumValue() const;
    void setTimeMinimumValue(uint newTimeMinimumValue);

    /* Get/Set the time range maximum value */
    uint timeMaximumValue() const;
    void setTimeMaximumValue(uint newTimeMaximumValue);

    /* Get/Set the current time value */
    uint currentTime() const;
    void setCurrentTime(uint newCurrentTime);

    /* Get/Set a flag to reset multipliers on dial change */
    bool resetOnDialChange() const;
    void setResetOnDialChange(bool newResetOnDialChange);

signals:
    void timeMinimumValueChanged();
    void timeMaximumValueChanged();
    void currentTimeChanged();
    void resetOnDialChangeChanged();

private:
    uint m_timeMinimumValue;
    uint m_timeMaximumValue;
    uint m_currentTime;
    bool m_resetOnDialChange;

    /*********************************************************************
     * Speed factor
     *********************************************************************/
public:
    /** Get/Set the speed factor to be applied to controlled Functions */
    SpeedMultiplier currentFactor() const;
    void setCurrentFactor(SpeedMultiplier factor);

    Q_INVOKABLE void increaseSpeedFactor();
    Q_INVOKABLE void decreaseSpeedFactor();

    /** This is where the speed magic happens. Current multiplier/divisor
     *  or absolute time is applied to the controlled Functions */
    Q_INVOKABLE void applyFunctionsTime();

signals:
    void currentFactorChanged();

private:
    SpeedMultiplier m_currentFactor;
    /** Index between OneSixteenth and Sixteen to be used by the UI */
    int m_factorIndex;

    /*********************************************************************
     * Functions
     *********************************************************************/
public:
    /**
     * Get/Set the list of functions that are controlled by the dial.
     */
    QMap<quint32, VCSpeedDialFunction> functions() const;
    void setFunctions(const QMap<quint32, VCSpeedDialFunction> &functions);

    /** Add a function to the speed control list */
    void addFunction(VCSpeedDialFunction function);

    /** Add/Remove a Function with the provided $functionID
     *  to be controlled by this widget */
    Q_INVOKABLE void addFunction(quint32 functionID);
    Q_INVOKABLE void removeFunction(quint32 functionID);

    /** Return a list suitable for the QML UI */
    QVariant functionsList();

    Q_INVOKABLE void setFunctionSpeed(quint32 fid, int speedType, SpeedMultiplier amount);

signals:
    void functionsListChanged();
    void presetsListChanged();

private:
    QMap<quint32, VCSpeedDialFunction> m_functions;

    /*********************************************************************
     * Presets
     *********************************************************************/
public:
    /** Return a list suitable for the QML UI */
    QVariantList presetsList() const;

    /** Add a preset and return its assigned ID */
    Q_INVOKABLE int addPreset(QString name, int value);
    Q_INVOKABLE void removePreset(quint8 presetId);
    Q_INVOKABLE void setPresetName(quint8 presetId, QString name);
    Q_INVOKABLE void setPresetValue(quint8 presetId, int value);

protected:
    QList<class VCSpeedDialPreset*> presets() const;
    void clearPresets();
    class VCSpeedDialPreset *findPreset(quint8 presetId) const;

private:
    void addPresetInternal(class VCSpeedDialPreset *preset);

private:
    quint8 m_lastAssignedPresetId;
    QList<class VCSpeedDialPreset*> m_presets;

    /*********************************************************************
     * Tap
     *********************************************************************/
public:
    int tapTimeValue() const;

    Q_INVOKABLE void tap();
    Q_INVOKABLE void resetTap();

signals:
    void tapTimeValueChanged();

private:
    static int calculateBPMByTapIntervals(QList<int> &tapHistory);

private:
    QList<int> m_tapHistory;
    qint64 m_lastTap;
    int m_tapTimeValue;

    /*********************************************************************
     * External input
     *********************************************************************/

public:
    /** @reimp */
    void updateFeedback() override;

public slots:
    /** @reimp */
    void slotInputValueChanged(quint8 id, uchar value) override;

    /*********************************************************************
     * Load & Save
     *********************************************************************/

public:
    bool loadXML(QXmlStreamReader &root) override;
    bool saveXML(QXmlStreamWriter *doc) const override;
};

#endif
