/*
  Q Light Controller Plus
  videoeditor.h

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

#ifndef VIDEOEDITOR_H
#define VIDEOEDITOR_H

#include "ui_videoeditor.h"

class Video;
class Doc;

/** @addtogroup ui_shows
 * @{
 */

class VideoEditor : public QWidget, public Ui_VideoEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(VideoEditor)

public:
    VideoEditor(QWidget* parent, Video* video, Doc* doc);
    ~VideoEditor();

private:
    Doc* m_doc;
    Video* m_video; // The Video function being edited

private slots:
    void slotNameEdited(const QString& text);
    void slotSourceFileClicked();
    void slotSourceUrlClicked();
    void slotScreenIndexChanged(int idx);
    void slotWindowedCheckClicked();
    void slotFullscreenCheckClicked();
    void slotPreviewToggled(bool state);
    void slotPreviewStopped(quint32 id);
    void slotDurationChanged(qint64 duration);
    void slotMetaDataChanged(QString key, QVariant data);
};

/** @} */

#endif
