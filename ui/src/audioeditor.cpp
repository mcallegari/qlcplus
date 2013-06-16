/*
  Q Light Controller
  audioeditor.cpp

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

#include <QLineEdit>
#include <QLabel>
#include <QDebug>


#include "audiodecoder.h"
#include "audioeditor.h"
#include "audio.h"
#include "doc.h"

AudioEditor::AudioEditor(QWidget* parent, Audio *audio, Doc* doc)
    : QWidget(parent)
    , m_doc(doc)
    , m_audio(audio)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(audio != NULL);

    setupUi(this);

    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameEdited(const QString&)));

    m_nameEdit->setText(m_audio->name());
    m_nameEdit->setSelection(0, m_nameEdit->text().length());

    AudioDecoder *adec = m_audio->getAudioDecoder();

    m_filenameLabel->setText(m_audio->getSourceFileName());
    if (adec != NULL)
    {
        AudioParameters ap = adec->audioParameters();
        m_durationLabel->setText(Function::speedToString(m_audio->getDuration()));
        m_srateLabel->setText(QString("%1 Hz").arg(ap.sampleRate()));
        m_channelsLabel->setText(QString("%1").arg(ap.channels()));
        m_bitrateLabel->setText(QString("%1 kb/s").arg(adec->bitrate()));
    }

    // Set focus to the editor
    m_nameEdit->setFocus();
}

AudioEditor::~AudioEditor()
{
}

void AudioEditor::slotNameEdited(const QString& text)
{
    m_audio->setName(text);
}
