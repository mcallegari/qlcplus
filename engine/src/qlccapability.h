/*
  Q Light Controller Plus
  qlccapability.h

  Copyright (C) Heikki Junnila
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

#ifndef QLCCAPABILITY_H
#define QLCCAPABILITY_H

#include <QVariantList>
#include <QObject>
#include <climits>
#include <QColor>
#include <QList>

class QXmlStreamReader;
class QXmlStreamWriter;
class QLCCapability;
class QString;
class QFile;

/** @addtogroup engine Engine
 * @{
 */

#define KXMLQLCCapability       QString("Capability")
#define KXMLQLCCapabilityMin    QString("Min")
#define KXMLQLCCapabilityMax    QString("Max")
#define KXMLQLCCapabilityPreset QString("Preset")
#define KXMLQLCCapabilityRes1   QString("Res1")
#define KXMLQLCCapabilityRes2   QString("Res2")

#define KXMLQLCCapabilityAlias              QString("Alias")
#define KXMLQLCCapabilityAliasMode          QString("Mode")
#define KXMLQLCCapabilityAliasSourceName    QString("Channel")
#define KXMLQLCCapabilityAliasTargetName    QString("With")

/** ****************** LEGACY ***************** */
#define KXMLQLCCapabilityResource   QString("Res")
#define KXMLQLCCapabilityColor1     QString("Color")
#define KXMLQLCCapabilityColor2     QString("Color2")

typedef struct
{
    QString targetMode;     /** Name of the mode where this alias has effect */
    QString sourceChannel;  /** Name of the channel to be replaced by targetChannel */
    QString targetChannel;  /** Name of the channel that will replace sourceChannel */
} AliasInfo;

/**
 * QLCCapability represents one value range with a special meaning in a
 * QLCChannel. For example, a sunburst gobo might be set on a "gobo" channel
 * with any DMX value between 15 and 25. This is represented as a
 * QLCCapability, whose min == 15, max == 25 and name == "Sunburst". Single
 * values can be represented by setting the same value to both, for example:
 * min == 15 and max == 15.
 */
class QLCCapability: public QObject
{
    Q_OBJECT

    Q_PROPERTY(int min READ min WRITE setMin NOTIFY minChanged)
    Q_PROPERTY(int max READ max WRITE setMax NOTIFY maxChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(WarningType warning READ warning WRITE setWarning NOTIFY warningChanged)
    Q_PROPERTY(QVariantList resources READ resources CONSTANT)

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    /** Default constructor */
    QLCCapability(uchar min = 0, uchar max = UCHAR_MAX,
                  const QString& name = QString(), QObject *parent = 0);

    QLCCapability *createCopy();

    /** Destructor */
    ~QLCCapability();

    /** Comparing operator for qSort */
    bool operator<(const QLCCapability& capability) const;

    /********************************************************************
     * Presets
     *
     * please see
     * https://github.com/mcallegari/qlcplus/wiki/Fixture-definition-presets
     * when changing this list
     ********************************************************************/
public:
    enum Preset
    {
        Custom = 0,
        SlowToFast,
        FastToSlow,
        NearToFar,
        FarToNear,
        BigToSmall,
        SmallToBig,
        ShutterOpen,
        ShutterClose,
        StrobeSlowToFast,
        StrobeFastToSlow,
        StrobeRandom,
        StrobeRandomSlowToFast,
        StrobeRandomFastToSlow,
        StrobeFrequency,        /** precise frequency value in hertz specified in m_resources */
        StrobeFreqRange,        /** specified in m_resources as 0: min, 1: max hertz */
        PulseSlowToFast,
        PulseFastToSlow,
        PulseFrequency,
        PulseFreqRange,
        RampUpSlowToFast,
        RampUpFastToSlow,
        RampDownSlowToFast,
        RampDownFastToSlow,
        RampUpFrequency,
        RampUpFreqRange,
        RampDownFrequency,
        RampDownFreqRange,
        RotationStop,
        RotationIndexed,
        RotationClockwise,
        RotationClockwiseSlowToFast,
        RotationClockwiseFastToSlow,
        RotationCounterClockwise,
        RotationCounterClockwiseSlowToFast,
        RotationCounterClockwiseFastToSlow,
        ColorMacro,
        ColorDoubleMacro,
        ColorWheelIndex,
        GoboMacro,
        GoboShakeMacro,
        GenericPicture,
        PrismEffectOn,
        PrismEffectOff,
        LampOn,
        LampOff,
        ResetAll,
        ResetPanTilt,
        ResetPan,
        ResetTilt,
        ResetMotors,
        ResetGobo,
        ResetColor,
        ResetCMY,
        ResetCTO,
        ResetEffects,
        ResetPrism,
        ResetBlades,
        ResetIris,
        ResetFrost,
        ResetZoom,
        SilentModeOn,
        SilentModeOff,
        SilentModeAutomatic,
        Alias,
        LastPreset // dummy for cycles
    };
#if QT_VERSION >= 0x050500
    Q_ENUM(Preset)
#else
    Q_ENUMS(Preset)
#endif

    enum PresetType
    {
        None,
        SingleColor,
        DoubleColor,
        SingleValue,
        DoubleValue,
        Picture
    };

#if QT_VERSION >= 0x050500
    Q_ENUM(PresetType)
#else
    Q_ENUMS(PresetType)
#endif

    /** String <-> value preset conversion helpers */
    static QString presetToString(Preset preset);
    static Preset stringToPreset(const QString &preset);

    /** Get/Set the preset value for this capability */
    Preset preset() const;
    void setPreset(Preset preset);

    /** Return the type of the current preset.
     *  This is useful for the UI to understand the type
     *  of resources this capability is exposing */
    PresetType presetType() const;

    /** Returns the value unit type of a preset as string */
    QString presetUnits() const;

protected:
    Preset m_preset;

    /********************************************************************
     * Properties
     ********************************************************************/
public:
    enum WarningType
    {
        NoWarning,
        EmptyName,
        Overlapping
    };

#if QT_VERSION >= 0x050500
    Q_ENUM(WarningType)
#else
    Q_ENUMS(WarningType)
#endif

    /** Get/Set the capability range minimum value */
    uchar min() const;
    void setMin(uchar value);

    /** Get/Set the capability range maximum value */
    uchar max() const;
    void setMax(uchar value);

    /** Get the capability range middle value */
    uchar middle() const;

    /** Get/Set the capability display name */
    QString name() const;
    void setName(const QString& name);

    /** Get/Set a warning for this capability */
    WarningType warning() const;
    void setWarning(WarningType type);

    /** Get the resource at the provided index.
     *  Returns an empty QVariant on failure */
    QVariant resource(int index);

    /** Add or replace a resource value at the provided index */
    void setResource(int index, QVariant value);

    /** Get the complete list of resources for this capability */
    QVariantList resources();

    /** Check, whether the given capability overlaps with this */
    bool overlaps(const QLCCapability* cap);

signals:
    void minChanged();
    void maxChanged();
    void nameChanged();
    void warningChanged();

protected:
    uchar m_min;
    uchar m_max;
    QString m_name;
    WarningType m_warning;
    QVariantList m_resources;

    /********************************************************************
     * Aliases
     ********************************************************************/
public:
    /** Get the full list of aliases defined by this capability */
    QList<AliasInfo> aliasList();

    /** Add a new alias to the aliases list */
    void addAlias(AliasInfo alias);

    /** Remove an existing alias matching the provided structure */
    void removeAlias(AliasInfo alias);

    /** Replace all the current aliases with the ones in the provided list */
    void replaceAliases(QList<AliasInfo> list);

protected:
    QList<AliasInfo> m_aliases;

    /********************************************************************
     * Load & Save
     ********************************************************************/
public:
    /** Save the capability into a QXmlStreamWriter */
    bool saveXML(QXmlStreamWriter *doc);

    /** Load capability contents from an XML element */
    bool loadXML(QXmlStreamReader &doc);
};

/** @} */

#endif
