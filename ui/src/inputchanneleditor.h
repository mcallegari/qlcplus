/*
  Q Light Controller
  inputchanneleditor.h

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

#ifndef INPUTCHANNELEDITOR_H
#define INPUTCHANNELEDITOR_H

#include <QDialog>

#include "qlcinputchannel.h"
#include "qlcinputprofile.h"
#include "ui_inputchanneleditor.h"

class QLCInputChannel;

/** @addtogroup ui_io
 * @{
 */

class InputChannelEditor : public QDialog, public Ui_InputChannelEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(InputChannelEditor)

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    InputChannelEditor(QWidget* parent,
                       const QLCInputProfile* profile,
                       const QLCInputChannel* channel,
                       QLCInputProfile::Type profileType);
    virtual ~InputChannelEditor();

    /********************************************************************
     * Properties
     ********************************************************************/
public:
    quint32 channel() const;
    QString name() const;
    QLCInputChannel::Type type() const;

protected slots:
    void slotNumberChanged(int number);
    void slotNameEdited(const QString& text);
    void slotTypeActivated(int index);

protected:
    quint32 m_channel;
    QString m_name;
    QLCInputChannel::Type m_type;

    /********************************************************************
     * MIDI
     ********************************************************************/
protected slots:
    void slotMidiChanged();

private:
    static void numberToMidi(int number, int & channel, int & message, int & param);
    static int midiToNumber(int channel, int message, int param);

    void enableMidiParam(int midiMessage, int midiParam);
    static QString noteToString(int note);
};

/** @} */

#endif
