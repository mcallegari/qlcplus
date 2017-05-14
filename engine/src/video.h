/*
  Q Light Controller Plus
  video.h

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

#ifndef VIDEO_H
#define VIDEO_H

#include <QColor>
#include <QVariant>

#include "function.h"

class QXmlStreamReader;

/** @addtogroup engine_functions Functions
 * @{
 */

class Video : public Function
{
    Q_OBJECT
    Q_DISABLE_COPY(Video)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    Video(Doc* doc);
    virtual ~Video();

    /** @reimp */
    QIcon getIcon() const;

private:
    Doc *m_doc;
    /*********************************************************************
     * Copying
     *********************************************************************/
public:
    /** @reimp */
    Function* createCopy(Doc* doc, bool addToDoc = true);

    /** Copy the contents for this function from another function */
    bool copyFrom(const Function* function);

public slots:
    /** Catches Doc::functionRemoved() so that destroyed members can be
        removed immediately. */
    void slotFunctionRemoved(quint32 function);

    /*********************************************************************
     * Capabilities
     *********************************************************************/
public:
    /** Get the list of the extensions supported by the video decoding system */
    static QStringList getCapabilities();

    /*********************************************************************
     * Properties
     *********************************************************************/
public:
    /** @reimpl */
    void setTotalDuration(quint32 duration);

    /** @reimpl */
    quint32 totalDuration();

    /**
     * Sets the video resolution as a QSize variable
     */
    void setResolution(QSize size);

    /**
     * Returns the video resolution as a QSize variable
     */
    QSize resolution();

    /**
     * Sets the audio codec for this Video Function
     */
    void setAudioCodec(QString codec);

    /**
     * Returns the audio codec detected from the media source
     */
    QString audioCodec();

    /**
     * Sets the video codec for this Video Function
     */
    void setVideoCodec(QString codec);

    /**
     * Returns the video codec detected from the media source
     */
    QString videoCodec();

    /**
     * Set the source file name used by this Video object
     */
    bool setSourceUrl(QString filename);

    /**
     * Retrieve the source file name used by this Video object
     */
    QString sourceUrl();

    /**
     * Set the screen index where to render the video
     */
    void setScreen(int index);

    /**
     * Retrieve the screen index where the video is rendered
     */
    int screen();

    /**
     * Set the video to be rendered in windowed or fullscreen mode
     */
    void setFullscreen(bool enable);

    /**
     * Retrieves if the video has to be rendered in windowed or fullscreen mode
     */
    bool fullscreen();

    void adjustAttribute(qreal fraction, int attributeIndex);

signals:
    void sourceChanged(QString url);
    void totalTimeChanged(qint64);
    void metaDataChanged(QString key, QVariant data);
    void requestPlayback();
    void requestPause(bool enable);
    void requestStop();
    void requestBrightnessAdjust(int value);

private:
    /** URL of the video media source */
    QString m_sourceUrl;
    /** Duration of the video content */
    qint64 m_videoDuration;
    /** The video codec as strings */
    QString m_audioCodec, m_videoCodec;
    /** Resolution of the video content */
    QSize m_resolution;
    /** Index of the screen where to render the video */
    int m_screen;
    /** Flag that indicates if the video has to go fullscreen */
    bool m_fullscreen;

    /*********************************************************************
     * Save & Load
     *********************************************************************/
public:
    /** Save function's contents to an XML document */
    bool saveXML(QXmlStreamWriter *doc);

    /** Load function's contents from an XML document */
    bool loadXML(QXmlStreamReader &root);

    /** @reimp */
    void postLoad();

    /*********************************************************************
     * Running
     *********************************************************************/
public:
    /** @reimpl */
    void preRun(MasterTimer*);

    /** @reimpl */
    void setPause(bool enable);

    /** @reimpl */
    void write(MasterTimer* timer, QList<Universe*> universes);

    /** @reimpl */
    void postRun(MasterTimer* timer, QList<Universe *> universes);

};

/** @} */

#endif
