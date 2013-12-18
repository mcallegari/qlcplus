/*
  Q Light Controller
  audioeditor.h

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

#ifndef AUDIOEDITOR_H
#define AUDIOEDITOR_H

#include "ui_audioeditor.h"

class Audio;
class Doc;

class AudioEditor : public QWidget, public Ui_AudioEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(AudioEditor)

public:
    AudioEditor(QWidget* parent, Audio* audio, Doc* doc);
    ~AudioEditor();

private:
    Doc* m_doc;
    Audio* m_audio; // The Audio function being edited

private slots:
    void slotNameEdited(const QString& text);
    void slotFadeInEdited();
    void slotFadeOutEdited();
};

#endif
