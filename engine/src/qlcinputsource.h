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

#include <QVariant>
#include <QThread>
#include <QMutex>

#include "qlcinputfeedback.h"

/** @addtogroup engine Engine
 * @{
 */

class QLCInputSource: public QThread
{
    Q_OBJECT

public:
    static quint32 invalidUniverse;
    static quint32 invalidChannel;
    static quint32 invalidID;

public:
    QLCInputSource(QThread * parent = 0);
    QLCInputSource(quint32 universe, quint32 channel, QThread * parent = 0);
    virtual ~QLCInputSource();

    bool isValid() const;

    /** Get/set the input source universe */
    void setUniverse(quint32 uni);
    quint32 universe() const;

    /** Get/set the input source channel */
    void setChannel(quint32 ch);
    quint32 channel() const;

    /** Set the input source page by masking it to the
     *  existing input channel */
    void setPage(ushort pgNum);

    /** Return the input source page retrieve from a masked channel */
    ushort page() const;

    /** Get/set the input source target ID */
    void setID(quint32 id);
    quint32 id() const;

private:
    /** The universe from which this input source comes from */
    quint32 m_universe;

    /** The channel from which this input source comes from */
    quint32 m_channel;

    /** The target ID of this input source */
    quint32 m_id;

    /*********************************************************************
     * Custom feedback
     *********************************************************************/
public:
    uchar feedbackValue(QLCInputFeedback::FeedbackType type) const;
    void setFeedbackValue(QLCInputFeedback::FeedbackType type, uchar value);

    /** Get/set specific plugins params.
     *  OSC: a string with the command path
     *  MIDI: a channel modifier
     */
    QVariant feedbackExtraParams(QLCInputFeedback::FeedbackType type) const;
    void setFeedbackExtraParams(QLCInputFeedback::FeedbackType type, QVariant params);

protected:
    QLCInputFeedback m_lower;
    QLCInputFeedback m_upper;
    QLCInputFeedback m_monitor;

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
