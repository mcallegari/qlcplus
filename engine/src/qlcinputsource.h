/*;
  Q Light Controller
  qlcinputsource.h

  Copyright (C) Heikki Junnila

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

#ifndef QLCINPUTSOURCE_H
#define QLCINPUTSOURCE_H

#include <QThread>
#include <QMutex>

/** @addtogroup engine Engine
 * @{
 */

class QLCInputSource: public QThread
{
    Q_OBJECT

public:
    static quint32 invalidUniverse;
    static quint32 invalidChannel;

public:
    QLCInputSource(QThread * parent = 0);
    QLCInputSource(quint32 universe, quint32 channel, QThread * parent = 0);
    virtual ~QLCInputSource();

    bool isValid() const;

    void setUniverse(quint32 uni);
    quint32 universe() const;

    void setChannel(quint32 ch);
    quint32 channel() const;

    void setPage(ushort pgNum);
    ushort page() const;

private:
    quint32 m_universe;
    quint32 m_channel;

    /*********************************************************************
     * Custom feedback
     *********************************************************************/
public:
    void setRange(uchar lower, uchar upper);
    uchar lowerValue() const;
    uchar upperValue() const;

protected:
    uchar m_lower, m_upper;

    /*********************************************************************
     * Working mode
     *********************************************************************/
public:
    /** Movement behaviour */
    enum WorkingMode {
        Absolute = 0,
        Relative = 1,
        Encoder = 2
    };

    WorkingMode workingMode() const;
    void setWorkingMode(WorkingMode mode);

    bool needsUpdate();

    int sensitivity() const;
    void setSensitivity(int value);

    bool sendExtraPressRelease() const;
    void setSendExtraPressRelease(bool enable);

    void updateInputValue(uchar value);
    void updateOuputValue(uchar value);

private:
    /** @reimp */
    void run();

protected:
    /** The input source mode: absolute or relative */
    WorkingMode m_workingMode;

    /** When in relative mode, this defines the sensitivity
     *  of synthetic emitted values against the external input value */
    int m_sensitivity;

    /** When enabled, this flag will emit an extra synthetic signal
     *  to simulate a press or release event */
    bool m_emitExtraPressRelease;

    /** The input value received from an external controller */
    uchar m_inputValue;

    /** The DMX value of a QLC+ item using this source.
     *  It is used to keep in sync this source and QLC+ */
    uchar m_outputValue;

    /** Thread running state condition variable.
     *  Used only in relative working mode */
    bool m_running;

    /** Mutex to syncronize input/output value updates */
    QMutex m_mutex;

signals:
    /** Signal emitted when the input source is in relative
     *  working mode. It emits synthetic generated values */
    void inputValueChanged(quint32 universe, quint32 channel,
                           uchar value, const QString& key = 0);
};

/** @} */

#endif
