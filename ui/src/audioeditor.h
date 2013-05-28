/*
  Q Light Controller
  audioeditor.h

  Copyright (c) Massimo Callegari

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
};

#endif
