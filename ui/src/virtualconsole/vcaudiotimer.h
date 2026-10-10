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
    QTimer *m_timer;
    /** Last painted text, to avoid useless repaints */
    QString m_lastState;

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

protected:
    void mousePressEvent(QMouseEvent *e) override;
};

/** @} */

#endif
