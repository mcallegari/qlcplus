/*
  Q Light Controller Plus
  audioitem.h

  Copyright (C) Massimo Callegari

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

#ifndef AUDIOITEM_H
#define AUDIOITEM_H

#include <QGraphicsItem>
#include <QObject>
#include <QAction>
#include <QFont>

#include "showitem.h"
#include "audio.h"

/** @addtogroup ui_functions
 * @{
 */

/**
 *
 * Audio Item. Clickable and draggable object identifying an Audio object
 *
 */
class AudioItem : public ShowItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    AudioItem(Audio *aud, ShowFunction *func);

    /** @reimp */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    /** @reimp */
    void setTimeScale(int val);

    /** @reimp */
    void setDuration(quint32 msec, bool stretch);

    /** @reimp */
    QString functionName();

    /** Return a pointer to a Audio Function associated to this item */
    Audio *getAudio();

protected:
    /** @reimp */
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

protected slots:
    void slotAudioChanged(quint32);

    void slotAudioPreviewLeft();
    void slotAudioPreviewRight();
    void slotAudioPreviewStereo();

private:
    /** Calculate sequence width for paint() and boundingRect() */
    void calculateWidth();

    /** Start a thread to elapse a waveform preview over the item */
    void updateWaveformPreview();

public:
    /** Reference to the actual Audio Function */
    Audio *m_audio;

    /** Context menu actions */
    QAction *m_previewLeftAction;
    QAction *m_previewRightAction;
    QAction *m_previewStereoAction;

    /** Pixmap holding the waveform (if enabled) */
    QPixmap *m_preview;
};

class PreviewThread : public QThread
{
public:
    void setAudioItem(AudioItem *item);

private:
    /** Retrieve a sample value from an audio buffer, given the sample size */
    qint32 getSample(unsigned char *data, quint32 idx, int sampleSize);
    void run();

    AudioItem *m_item;
};

/** @} */

#endif
