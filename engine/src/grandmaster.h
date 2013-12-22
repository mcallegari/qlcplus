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
    enum GMValueMode
    {
        GMLimit, /** Limit maximum values to current GM value */
        GMReduce /** Reduce channel values by a fraction (0-100%) */
    };

    enum GMChannelMode
    {
        GMIntensity,  /** GM applied only for Intensity channels */
        GMAllChannels /** GM applied for all channels */
    };

    enum GMSliderMode
    {
        GMNormal,     /** GM slider in normal mode 0-255 */
        GMInverted    /** GM Slider inverted mode 255-0 */
    };

    static GMValueMode stringToGMValueMode(const QString& str);
    static QString gMValueModeToString(GMValueMode mode);

    static GMChannelMode stringToGMChannelMode(const QString& str);
    static QString gMChannelModeToString(GMChannelMode mode);

    static GMSliderMode stringToGMSliderMode(const QString& str);
    static QString gMSliderModeToString(GMSliderMode mode);

    /**
     * Set the way how Grand Master should treat its value. @See enum
     * GMValueMode for more info on the modes.
     *
     * @param mode The mode to set
     */
    void setGMValueMode(GMValueMode mode);

    /**
     * Get the Grand Master value mode.
     * @See setGMValueMode() and enum GMValueMode.
     *
     * @return Current value mode
     */
    GMValueMode gMValueMode() const;

    /**
     * Set the way how Grand Master should treat channels. @See enum
     * GrandMasterChannelMode for more info on the modes.
     *
     * @param mode The mode to set
     */
    void setGMChannelMode(GMChannelMode mode);

    /**
     * Get the Grand Master channel mode.
     * @See setGMChannelMode() and enum GMChannelMode.
     *
     * @return Current channel mode
     */
    GMChannelMode gMChannelMode() const;

    /**
     * Set the Grand Master value as a DMX value 0-255. This value is
     * converted to a fraction according to the current mode.
     */
    void setGMValue(uchar value);

    /**
     * Get the current Grand Master value as a DMX value (0 - 255)
     *
     * @return Current Grand Master value in DMX
     */
    uchar gMValue() const;

    /**
     * Get the current Grand Master value as a fraction 0.0 - 1.0
     *
     * @return Current Grand Master value as a fraction
     */
    double gMFraction() const;

signals:
    void valueChanged(uchar value);
    
protected:
    GMValueMode m_gMValueMode;
    GMChannelMode m_gMChannelMode;
    uchar m_gMValue;
    double m_gMFraction;
    
};

#endif
