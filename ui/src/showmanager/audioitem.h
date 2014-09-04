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
    AudioItem(Audio *aud);

    /** @reimp */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    /** @reimp */
    void setTimeScale(int val);

    /** @reimp */
    void setStartTime(quint32 time);

    /** @reimp */
    quint32 getStartTime();

    /** @reimp */
    QString functionName();

    /** @reimp */
    void setLocked(bool locked);

    /** Return a pointer to a Audio object associated to this item */
    Audio *getAudio();

public slots:
    void updateDuration();

protected:
    /** @reimp */
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

protected slots:
    void slotAudioChanged(quint32);

    void slotAudioPreviewLeft(bool active);
    void slotAudioPreviewRight(bool active);
    void slotAudioPreviewStereo(bool active);
    void slotAlignToCursorClicked();
    void slotLockItemClicked();

private:
    /** Calculate sequence width for paint() and boundingRect() */
    void calculateWidth();
    /** Retrieve a sample value from an audio buffer, given the sample size */
    qint32 getSample(unsigned char *data, quint32 *idx, int sampleSize);
    /** Routine that decode the whole and create the waveform QPixmap */
    void createWaveform(bool left, bool right);

private:
    /** Reference to the actual Audio object */
    Audio *m_audio;

    /** Context menu actions */
    QAction *m_previewLeftAction;
    QAction *m_previewRightAction;
    QAction *m_previewStereoAction;

    /** Pixmap holding the waveform (if enabled) */
    QPixmap *m_preview;
};

/** @} */

#endif
