/*
  Q Light Controller Plus
  videoeditor.cpp

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

#include <QDesktopWidget>
#include <QInputDialog>
#include <QFileDialog>
#include <QLineEdit>
#include <QLabel>
#include <QDebug>

#include "videoeditor.h"
#include "video.h"
#include "doc.h"

VideoEditor::VideoEditor(QWidget* parent, Video *video, Doc* doc)
    : QWidget(parent)
    , m_doc(doc)
    , m_video(video)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(video != NULL);

    setupUi(this);

    m_nameEdit->setText(m_video->name());
    m_nameEdit->setSelection(0, m_nameEdit->text().length());

    connect(m_video, SIGNAL(totalTimeChanged(qint64)),
            this, SLOT(slotDurationChanged(qint64)));
    connect(m_video, SIGNAL(metaDataChanged(QString,QVariant)),
            this, SLOT(slotMetaDataChanged(QString,QVariant)));

    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameEdited(const QString&)));
    connect(m_fileButton, SIGNAL(clicked()),
            this, SLOT(slotSourceFileClicked()));
    connect(m_urlButton, SIGNAL(clicked()),
            this, SLOT(slotSourceUrlClicked()));

    connect(m_previewButton, SIGNAL(toggled(bool)),
            this, SLOT(slotPreviewToggled(bool)));

    m_filenameLabel->setText(m_video->sourceUrl());
    m_durationLabel->setText(Function::speedToString(m_video->totalDuration()));
    QSize res = video->resolution();
    m_resolutionLabel->setText(QString("%1x%2").arg(res.width()).arg(res.height()));
    m_vcodecLabel->setText(video->videoCodec());
    m_acodecLabel->setText(video->audioCodec());


    int screenCount = 0;
    QDesktopWidget *desktop = qApp->desktop();
    if (desktop != NULL)
        screenCount = desktop->screenCount();

    if (screenCount > 0)
    {
        for (int i = 0; i < screenCount; i++)
            m_screenCombo->addItem(QString("Screen %1").arg(i + 1));
    }

    m_screenCombo->setCurrentIndex(m_video->screen());

    if (m_video->fullscreen() == true)
        m_fullCheck->setChecked(true);
    else
        m_winCheck->setChecked(true);

    connect(m_screenCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotScreenIndexChanged(int)));
    connect(m_winCheck, SIGNAL(clicked()),
            this, SLOT(slotWindowedCheckClicked()));
    connect(m_fullCheck, SIGNAL(clicked()),
            this, SLOT(slotFullscreenCheckClicked()));

    // Set focus to the editor
    m_nameEdit->setFocus();
}

VideoEditor::~VideoEditor()
{
    if (m_video->isRunning())
       m_video->stop();
/*
    disconnect(m_video, SIGNAL(totalTimeChanged(qint64)),
               this, SLOT(slotDurationChanged(qint64)));
    disconnect(m_video, SIGNAL(metaDataChanged(QString,QVariant)),
               this, SLOT(slotMetaDataChanged(QString,QVariant)));
*/
}

void VideoEditor::slotNameEdited(const QString& text)
{
    m_video->setName(text);
    m_doc->setModified();
}

void VideoEditor::slotSourceFileClicked()
{
    QString fn;

    /* Create a file open dialog */
    QFileDialog dialog(this);
    dialog.setWindowTitle(tr("Open Video File"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);

    /* Append file filters to the dialog */
    QStringList extList = Video::getCapabilities();

    QStringList filters;
    qDebug() << Q_FUNC_INFO << "Extensions: " << extList.join(" ");
    filters << tr("Video Files (%1)").arg(extList.join(" "));
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

    if (m_video->isRunning())
        m_video->stopAndWait();

    m_video->setSourceUrl(fn);
    m_filenameLabel->setText(m_video->sourceUrl());
    m_durationLabel->setText(Function::speedToString(m_video->totalDuration()));
}

void VideoEditor::slotSourceUrlClicked()
{
    bool ok;
    QString videoURL = QInputDialog::getText(this, tr("Video source URL"),
                                         tr("Enter a URL:"), QLineEdit::Normal,
                                         "http://", &ok);

    if (ok == true)
    {
        m_video->setSourceUrl(videoURL);
        m_filenameLabel->setText(m_video->sourceUrl());
    }
}

void VideoEditor::slotScreenIndexChanged(int idx)
{
    m_video->setScreen(idx);
}

void VideoEditor::slotWindowedCheckClicked()
{
    m_video->setFullscreen(false);
}

void VideoEditor::slotFullscreenCheckClicked()
{
    m_video->setFullscreen(true);
}

void VideoEditor::slotPreviewToggled(bool state)
{
    if (state == true)
    {
        m_video->start(m_doc->masterTimer());
        connect(m_video, SIGNAL(stopped(quint32)),
                this, SLOT(slotPreviewStopped(quint32)));
    }
    else
        m_video->stop();
}

void VideoEditor::slotPreviewStopped(quint32 id)
{
    if (id == m_video->id())
    {
        m_previewButton->blockSignals(true);
        m_previewButton->setChecked(false);
        m_previewButton->blockSignals(false);
    }
}

void VideoEditor::slotDurationChanged(qint64 duration)
{
    m_durationLabel->setText(Function::speedToString(duration));
}

void VideoEditor::slotMetaDataChanged(QString key, QVariant data)
{
    qDebug() << "Got meta data:" << key;
    if (key == "Resolution")
    {
        QSize res = data.toSize();
        m_resolutionLabel->setText(QString("%1x%2").arg(res.width()).arg(res.height()));
    }
    else if (key == "VideoCodec")
        m_vcodecLabel->setText(data.toString());
    else if (key == "AudioCodec")
        m_acodecLabel->setText(data.toString());
}

