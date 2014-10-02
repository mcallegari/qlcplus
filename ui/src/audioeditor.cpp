/*
  Q Light Controller Plus
  audioeditor.cpp

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

#include <QFileDialog>
#include <QLineEdit>
#include <QSettings>
#include <QLabel>
#include <QDebug>
#include <QUrl>

#include "speeddialwidget.h"
#include "audiodecoder.h"
#include "audioeditor.h"
#include "audio.h"
#include "doc.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
 #if defined( __APPLE__) || defined(Q_OS_MAC)
  #include "audiorenderer_portaudio.h"
 #elif defined(WIN32) || defined(Q_OS_WIN)
  #include "audiorenderer_waveout.h"
 #else
  #include "audiorenderer_alsa.h"
 #endif
#else
 #include "audiorenderer_qt.h"
#endif

AudioEditor::AudioEditor(QWidget* parent, Audio *audio, Doc* doc)
    : QWidget(parent)
    , m_doc(doc)
    , m_audio(audio)
    , m_speedDials(NULL)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(audio != NULL);

    setupUi(this);

    m_nameEdit->setText(m_audio->name());
    m_nameEdit->setSelection(0, m_nameEdit->text().length());

    m_fadeInEdit->setText(Function::speedToString(audio->fadeInSpeed()));
    m_fadeOutEdit->setText(Function::speedToString(audio->fadeOutSpeed()));

    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameEdited(const QString&)));
    connect(m_fileButton, SIGNAL(clicked()),
            this, SLOT(slotSourceFileClicked()));

    connect(m_speedDialButton, SIGNAL(toggled(bool)),
            this, SLOT(slotSpeedDialToggle(bool)));

    connect(m_fadeInEdit, SIGNAL(returnPressed()),
            this, SLOT(slotFadeInEdited()));
    connect(m_fadeOutEdit, SIGNAL(returnPressed()),
            this, SLOT(slotFadeOutEdited()));

    connect(m_previewButton, SIGNAL(toggled(bool)),
            this, SLOT(slotPreviewToggled(bool)));

    AudioDecoder *adec = m_audio->getAudioDecoder();

    m_filenameLabel->setText(m_audio->getSourceFileName());
    if (adec != NULL)
    {
        AudioParameters ap = adec->audioParameters();
        m_durationLabel->setText(Function::speedToString(m_audio->totalDuration()));
        m_srateLabel->setText(QString("%1 Hz").arg(ap.sampleRate()));
        m_channelsLabel->setText(QString("%1").arg(ap.channels()));
        m_bitrateLabel->setText(QString("%1 kb/s").arg(adec->bitrate()));
    }

    QList<AudioDeviceInfo> devList;
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
 #if defined( __APPLE__) || defined(Q_OS_MAC)
    devList = AudioRendererPortAudio::getDevicesInfo();
 #elif defined(WIN32) || defined(Q_OS_WIN)
    devList = AudioRendererWaveOut::getDevicesInfo();
 #else
    devList = AudioRendererAlsa::getDevicesInfo();
 #endif
#else
    devList = AudioRendererQt::getDevicesInfo();
#endif
    QSettings settings;
    QString outputName;
    int i = 0, selIdx = 0;

    m_audioDevCombo->addItem(tr("Default device"), "__qlcplusdefault__");
    if (m_audio->audioDevice().isEmpty())
    {
        QVariant var = settings.value(SETTINGS_AUDIO_OUTPUT_DEVICE);
        if (var.isValid() == true)
            outputName = var.toString();
    }
    else
        outputName = m_audio->audioDevice();

    foreach( AudioDeviceInfo info, devList)
    {
        if (info.capabilities & AUDIO_CAP_OUTPUT)
        {
            m_audioDevCombo->addItem(info.deviceName, info.privateName);

            if (info.privateName == outputName)
                selIdx = i;
            i++;
        }
    }
    m_audioDevCombo->setCurrentIndex(selIdx);
    connect(m_audioDevCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotAudioDeviceChanged(int)));

    // Set focus to the editor
    m_nameEdit->setFocus();
}

AudioEditor::~AudioEditor()
{
    if (m_audio->isRunning())
       m_audio->stop();
}

void AudioEditor::slotNameEdited(const QString& text)
{
    m_audio->setName(text);
    m_doc->setModified();
}

void AudioEditor::slotSourceFileClicked()
{
    QString fn;

    /* Create a file open dialog */
    QFileDialog dialog(this);
    dialog.setWindowTitle(tr("Open Audio File"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);

    /* Append file filters to the dialog */
    QStringList extList = Audio::getCapabilities();

    QStringList filters;
    qDebug() << Q_FUNC_INFO << "Extensions: " << extList.join(" ");
    filters << tr("Audio Files (%1)").arg(extList.join(" "));
#if defined(WIN32) || defined(Q_OS_WIN)
    filters << tr("All Files (*.*)");
#else
    filters << tr("All Files (*)");
#endif
    dialog.setNameFilters(filters);

    /* Append useful URLs to the dialog */
    QList <QUrl> sidebar;
    sidebar.append(QUrl::fromLocalFile(QDir::homePath()));
    sidebar.append(QUrl::fromLocalFile(QDir::rootPath()));
    dialog.setSidebarUrls(sidebar);

    /* Get file name */
    if (dialog.exec() != QDialog::Accepted)
        return;

    fn = dialog.selectedFiles().first();
    if (fn.isEmpty() == true)
        return;

    if (m_audio->isRunning())
        m_audio->stopAndWait();

    m_audio->setSourceFileName(fn);
    m_filenameLabel->setText(m_audio->getSourceFileName());

    AudioDecoder *adec = m_audio->getAudioDecoder();
    if (adec != NULL)
    {
        AudioParameters ap = adec->audioParameters();
        m_durationLabel->setText(Function::speedToString(m_audio->totalDuration()));
        m_srateLabel->setText(QString("%1 Hz").arg(ap.sampleRate()));
        m_channelsLabel->setText(QString("%1").arg(ap.channels()));
        m_bitrateLabel->setText(QString("%1 kb/s").arg(adec->bitrate()));
    }
}

void AudioEditor::slotFadeInEdited()
{
    uint newValue;
    QString text = m_fadeInEdit->text();
    if (text.contains(".") || text.contains("s") ||
        text.contains("m") || text.contains("h"))
            newValue = Function::stringToSpeed(text);
    else
    {
        newValue = (text.toDouble() * 1000);
        m_fadeInEdit->setText(Function::speedToString(newValue));
    }

    m_audio->setFadeInSpeed(newValue);
    m_doc->setModified();
}

void AudioEditor::slotFadeOutEdited()
{
    uint newValue;
    QString text = m_fadeOutEdit->text();
    if (text.contains(".") || text.contains("s") ||
        text.contains("m") || text.contains("h"))
            newValue = Function::stringToSpeed(text);
    else
    {
        newValue = (text.toDouble() * 1000);
        m_fadeOutEdit->setText(Function::speedToString(newValue));
    }

    m_audio->setFadeOutSpeed(newValue);
    m_doc->setModified();
}

void AudioEditor::slotAudioDeviceChanged(int idx)
{
    QString audioDev = m_audioDevCombo->itemData(idx).toString();
    qDebug() << "New audio device selected:" << audioDev;
    if (audioDev == "__qlcplusdefault__")
        m_audio->setAudioDevice(QString());
    else
        m_audio->setAudioDevice(audioDev);
}

void AudioEditor::slotPreviewToggled(bool state)
{
    if (state == true)
    {
        m_audio->start(m_doc->masterTimer());
        connect(m_audio, SIGNAL(stopped(quint32)),
                this, SLOT(slotPreviewStopped(quint32)));
    }
    else
        m_audio->stop();
}

void AudioEditor::slotPreviewStopped(quint32 id)
{
    if (id == m_audio->id())
        m_previewButton->setChecked(false);
}

/************************************************************************
 * Speed dials
 ************************************************************************/

void AudioEditor::createSpeedDials()
{
    if (m_speedDials != NULL)
        return;

    m_speedDials = new SpeedDialWidget(this);
    m_speedDials->setAttribute(Qt::WA_DeleteOnClose);
    m_speedDials->setWindowTitle(m_audio->name());
    m_speedDials->setFadeInSpeed(m_audio->fadeInSpeed());
    m_speedDials->setFadeOutSpeed(m_audio->fadeOutSpeed());
    m_speedDials->setDurationEnabled(false);
    m_speedDials->setDurationVisible(false);
    connect(m_speedDials, SIGNAL(fadeInChanged(int)), this, SLOT(slotFadeInDialChanged(int)));
    connect(m_speedDials, SIGNAL(fadeOutChanged(int)), this, SLOT(slotFadeOutDialChanged(int)));
    connect(m_speedDials, SIGNAL(destroyed(QObject*)), this, SLOT(slotDialDestroyed(QObject*)));
    m_speedDials->show();
}

void AudioEditor::slotSpeedDialToggle(bool state)
{
    if (state == true)
        createSpeedDials();
    else
    {
        if (m_speedDials != NULL)
            m_speedDials->deleteLater();
        m_speedDials = NULL;
    }
}

void AudioEditor::slotFadeInDialChanged(int ms)
{
    m_fadeInEdit->setText(Function::speedToString(ms));
    m_audio->setFadeInSpeed(ms);
}

void AudioEditor::slotFadeOutDialChanged(int ms)
{
    m_fadeOutEdit->setText(Function::speedToString(ms));
    m_audio->setFadeOutSpeed(ms);
}

void AudioEditor::slotDialDestroyed(QObject *)
{
    m_speedDialButton->setChecked(false);
}
