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
#include <QRect>
#include <QVector3D>
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

    Q_PROPERTY(QString sourceUrl READ sourceUrl WRITE setSourceUrl NOTIFY sourceChanged)
    Q_PROPERTY(qreal intensity READ intensity NOTIFY intensityChanged)
    Q_PROPERTY(QRect customGeometry READ customGeometry WRITE setCustomGeometry NOTIFY customGeometryChanged)
    Q_PROPERTY(QVector3D rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
    Q_PROPERTY(int zIndex READ zIndex WRITE setZIndex NOTIFY zIndexChanged)
    Q_PROPERTY(bool fullscreen READ fullscreen WRITE setFullscreen)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    enum VideoAttr
    {
        Intensity = Function::Intensity,
        Volume,
        XRotation,
        YRotation,
        ZRotation,
        XPosition,
        YPosition,
        WidthScale,
        HeightScale
    };

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
    static QStringList getVideoCapabilities();

    /** Get the list of the extensions supported for picture rendering */
    static QStringList getPictureCapabilities();

    static const QStringList m_defaultVideoCaps;
    static const QStringList m_defaultPictureCaps;

    /*********************************************************************
     * Properties
     *********************************************************************/
public:
    /** @reimpl */
    void setTotalDuration(quint32 duration);

    /** @reimpl */
    quint32 totalDuration();

    /** Get/Set the video resolution as a QSize variable */
    QSize resolution();
    void setResolution(QSize size);

    /** Get/Set the video custom geometry as a QRect variable */
    QRect customGeometry();
    void setCustomGeometry(QRect rect);

    /** Get/Set the video XYZ rotation as a QVector3D variable */
    QVector3D rotation() const;
    void setRotation(QVector3D rotation);

    /** Get/Set the video Z-Index used for layering */
    int zIndex() const;
    void setZIndex(int idx);

    /** Get/Set the audio codec for this Video Function */
    QString audioCodec();
    void setAudioCodec(QString codec);

    /** Get/Set the video codec for this Video Function */
    QString videoCodec();
    void setVideoCodec(QString codec);

    /** Get/Set the source URL used by this Video object */
    QString sourceUrl();
    bool setSourceUrl(QString filename);

    /** Return if the loaded source is a picture */
    bool isPicture() const;

    /** Get/Set the screen index where to render the video */
    int screen();
    void setScreen(int index);

    /** Get/Set the video to be rendered in windowed or fullscreen mode */
    bool fullscreen();
    void setFullscreen(bool enable);

    /** Get the current Video intensity */
    qreal intensity();

    /** @reimp */
    int adjustAttribute(qreal fraction, int attributeId);

signals:
    void sourceChanged(QString url);
    void intensityChanged();
    void customGeometryChanged(QRect rect);
    void rotationChanged(QVector3D rotation);
    void zIndexChanged(int index);
    void totalTimeChanged(qint64);
    void metaDataChanged(QString key, QVariant data);
    void requestPlayback();
    void requestPause(bool enable);
    void requestStop();
    void requestBrightnessAdjust(int value);

private:
    /** URL of the video media source */
    QString m_sourceUrl;
    /** Flag that indicates if the loaded source is a picture (or a video) */
    bool m_isPicture;
    /** Duration of the video content */
    qint64 m_videoDuration;
    /** The audio and video codec as strings */
    QString m_audioCodec, m_videoCodec;
    /** Resolution of the video content */
    QSize m_resolution;
    /** If set, specifies the custom geometry (position and size)
     *  to be used when rendering the video */
    QRect m_customGeometry;
    /** The video XYZ rotation as a 3D vector */
    QVector3D m_rotation;
    /** The video Z-Index */
    int m_zIndex;
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
