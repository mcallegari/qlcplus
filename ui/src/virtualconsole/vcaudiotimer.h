/*
  Q Light Controller Plus
  vcaudiotimer.h

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

#ifndef VCAUDIOTIMER_H
#define VCAUDIOTIMER_H

#include <QElapsedTimer>
#include <QPixmap>
#include <QHash>

#include "vcwidget.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class QPaintEvent;
class QMouseEvent;
class QPainter;
class QTimer;
class Doc;

/** @addtogroup ui_vc_widgets
 * @{
 */

#define KXMLQLCVCAudioTimer QStringLiteral("AudioTimer")

/**
 * Shows elapsed, total and remaining time of every Audio function
 * that is currently playing, regardless of what started it.
 */
class VCAudioTimer final : public VCWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(VCAudioTimer)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCAudioTimer(QWidget* parent, Doc* doc);
    ~VCAudioTimer();

private slots:
    void slotUpdate();

private:
    struct Entry
    {
        quint32 id;
        bool paused;
        QString name;
        QString file;
        quint32 elapsed;   // smoothed position shown on screen
        quint32 total;
    };

    /** Collect the running audio functions, with a smoothed elapsed time */
    void refreshEntries();

    QTimer *m_timer;
    /** Running audio functions, refreshed by slotUpdate() */
    QList<Entry> m_entries;
    /** Last painted state when nothing changes continuously, to avoid useless repaints */
    QString m_lastState;

    /** The engine updates the elapsed time in ticks: between them the position
     *  is extrapolated with a local clock, so that the playhead moves smoothly */
    struct Smoothing
    {
        quint32 engineElapsed;
        qint64 stamp;
        quint32 shown;
    };
    QHash<quint32, Smoothing> m_smoothing;
    QElapsedTimer m_clock;

    /** Seek requested from the waveform and not applied by the engine yet:
     *  function ID -> (target position, time of the request) */
    QHash<quint32, QPair<quint32, qint64> > m_pendingSeeks;

    /** Waveforms rendered once per size and color, then just copied when painting */
    struct WaveCache
    {
        QString file;
        QSize size;
        QColor unplayedColor;
        QColor playedColor;
        QPixmap unplayed;
        QPixmap played;
    };
    QHash<quint32, WaveCache> m_waveCache;

    /*********************************************************************
     * Clipboard
     *********************************************************************/
public:
    VCWidget* createCopy(VCWidget* parent) const override;

    /*********************************************************************
     * Properties
     *********************************************************************/
public:
    void editProperties() override;

    /*****************************************************************************
     * External input
     *****************************************************************************/
    /** @reimp */
    void updateFeedback() override { }

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    bool loadXML(QXmlStreamReader &root) override;
    bool saveXML(QXmlStreamWriter *doc) override;

    /*********************************************************************
     * Painting
     *********************************************************************/
protected:
    void paintEvent(QPaintEvent* e) override;

    /*********************************************************************
     * Transport buttons
     *********************************************************************/
private:
    enum ButtonType { PlayButton, PauseButton, StopButton };

    struct ButtonArea
    {
        ButtonArea(quint32 id = 0, ButtonType t = StopButton, QRect r = QRect())
            : functionID(id), type(t), rect(r) { }
        quint32 functionID;
        ButtonType type;
        QRect rect;
    };

    /** Clickable areas of the buttons painted in the last paintEvent */
    QList<ButtonArea> m_buttons;

    void drawButton(QPainter &painter, const QRect &r, ButtonType type, const QColor &color);

    /** Waveform areas painted in the last paintEvent, where clicking seeks the track */
    struct SeekArea
    {
        quint32 functionID;
        QRect rect;
        quint32 total;
    };
    QList<SeekArea> m_seekAreas;

    /** Track being dragged on its waveform (invalid ID when none) and the dragged position */
    quint32 m_dragID;
    quint32 m_dragPosition;

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
};

/** @} */

#endif
