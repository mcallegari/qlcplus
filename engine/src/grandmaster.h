/*
  Q Light Controller Plus
  grandmaster.h

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

#ifndef GRANDMASTER_H
#define GRANDMASTER_H

#include <QObject>
#include <QString>
#include <QSet>

/** @addtogroup engine Engine
 * @{
 */

/** Contains settings for Grand Master
 *
 *  Changing properties in Operate mode is not supported.
 *  If the need arises, Universe::slotGMValueChanged() needs to be fixed (recompute
 *  all channels, for the case when mode is changed from AllChannels to Intensity)
 */
class GrandMaster: public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(GrandMaster)

public:
    /** Construct a new GrandMaster */
    GrandMaster(QObject* parent = 0);

    /** Destructor */
    virtual ~GrandMaster();

public:
    enum ValueMode
    {
        Limit, /** Limit maximum values to current GM value */
        Reduce /** Reduce channel values by a fraction (0-100%) */
    };
#if QT_VERSION >= 0x050500
    Q_ENUM(ValueMode)
#endif

    enum ChannelMode
    {
        Intensity,  /** GM applied only for Intensity channels */
        AllChannels /** GM applied for all channels */
    };
#if QT_VERSION >= 0x050500
    Q_ENUM(ChannelMode)
#endif

    enum SliderMode
    {
        Normal,     /** GM slider in normal mode 0-255 */
        Inverted    /** GM Slider inverted mode 255-0 */
    };

    static ValueMode stringToValueMode(const QString& str);
    static QString valueModeToString(ValueMode mode);

    static ChannelMode stringToChannelMode(const QString& str);
    static QString channelModeToString(ChannelMode mode);

    static SliderMode stringToSliderMode(const QString& str);
    static QString sliderModeToString(SliderMode mode);

    /**
     * Set the way how Grand Master should treat its value. @See enum
     * GMValueMode for more info on the modes.
     *
     * @param mode The mode to set
     */
    void setValueMode(ValueMode mode);

    /**
     * Get the Grand Master value mode.
     * @See setValueMode() and enum GMValueMode.
     *
     * @return Current value mode
     */
    ValueMode valueMode() const;

    /**
     * Set the way how Grand Master should treat channels. @See enum
     * GrandMasterChannelMode for more info on the modes.
     *
     * @param mode The mode to set
     */
    void setChannelMode(ChannelMode mode);

    /**
     * Get the Grand Master channel mode.
     * @See setChannelMode() and enum GMChannelMode.
     *
     * @return Current channel mode
     */
    ChannelMode channelMode() const;

    /**
     * Set the Grand Master value as a DMX value 0-255. This value is
     * converted to a fraction according to the current mode.
     */
    void setValue(uchar value);

    /**
     * Get the current Grand Master value as a DMX value (0 - 255)
     *
     * @return Current Grand Master value in DMX
     */
    uchar value() const;

    /**
     * Get the current Grand Master value as a fraction 0.0 - 1.0
     *
     * @return Current Grand Master value as a fraction
     */
    double fraction() const;

signals:
    void valueChanged(uchar value);

protected:
    ValueMode m_valueMode;
    ChannelMode m_channelMode;
    uchar m_value;
    double m_fraction;
};

/** @} */

#endif
