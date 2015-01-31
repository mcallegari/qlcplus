/*
  Q Light Controller Plus
  clickandgowidget.h

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

#ifndef CLICKANDGOWIDGET_H
#define CLICKANDGOWIDGET_H

#include <QWidget>

#include "qlcchannel.h"

/** @addtogroup ui UI
 * @{
 */

class ClickAndGoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ClickAndGoWidget(QWidget *parent = 0);

    enum ClickAndGo
    {
        None,
        Red,
        Green,
        Blue,
        Cyan,
        Magenta,
        Yellow,
        Amber,
        White,
        UV,
        RGB,
        CMY,
        Preset
    };

    /**
     * Set the widget type. This is fundamental
     * for the whole widget behaviour
     */
    void setType(int type, const QLCChannel *chan = NULL);

    /**
     * Returns the widget type
     */
    int getType();

    /**
     * Returns the color at pos position.
     * Used with primary colors linear gradient
     */
    QColor getColorAt(uchar pos);

    /**
     * Return a QImage to be displayed on a Click & Go button
     *
     * @param value the slider position value
     */
    QImage getImageFromValue(uchar value);

    /** Returns a human readable string of a Click And Go type */
    static QString clickAndGoTypeToString(ClickAndGoWidget::ClickAndGo type);

    /** Returns a Click And Go type from the given string */
    static ClickAndGoWidget::ClickAndGo stringToClickAndGoType(QString str);

protected:
    /**
     * Prepare the widget to display a linear gradient
     * from either black or white (begin) to a primary color (end)
     */
    void setupGradient(QColor begin, QColor end);

    /**
     * Helper function to draw a vertical gradient from
     * black to white to a given X position
     */
    void fillWithGradient(int r, int g, int b, QPainter *painter, int x);

    /**
     * Prepare the widget to display a full color picker
     * with all gradients of colors from black to white
     * and 16 default colors
     */
    void setupColorPicker();

    /**
     * Prepare the list of gobos/effects to be used
     * when the widget is in Preset mode
     */
    void createPresetList(const QLCChannel *chan);

    /**
     * Prepare the widget to display a grid of the
     * channel(s) preset functions
     */
    void setupPresetPicker();

    /*************************************************************************
     * PresetResource Class
     *************************************************************************/
private:
    class PresetResource
    {
    public:
        PresetResource(QString path, QString text, uchar min, uchar max);
        PresetResource(QColor color1, QColor color2, QString text, uchar min, uchar max);
        PresetResource(int index, QString text, uchar min, uchar max);

    public:
        QImage m_thumbnail;
        QString m_descr;
        uchar m_min;
        uchar m_max;
    };

protected:
    /** The Click And Go type. Essential for the whole widget behaviour */
    int m_type;

    /** Geometry parameters of the widget */
    int m_width;
    int m_height;
    int m_cols;
    int m_rows;
    int m_cellWidth;
    int m_hoverCellIdx;
    int m_cellBarXpos;
    int m_cellBarYpos;
    int m_cellBarWidth;

    QList<ClickAndGoWidget::PresetResource> m_resources;

    /** Used to group all the primary colors */
    bool m_linearColor;

    /** Pre-rendered bitmap with the UI displayed by ClickAndGoAction */
    QImage m_image;

protected:
    /** @reimp */
    QSize sizeHint() const;

    /** @reimp */
    void mousePressEvent(QMouseEvent *event);

    /** @reimp */
    void mouseMoveEvent(QMouseEvent *event);

    /** @reimp */
    void paintEvent(QPaintEvent *event);

signals:
    void levelChanged(uchar level);
    void colorChanged(QRgb color);
    void levelAndPresetChanged(uchar level, QImage img);
    
public slots:
    
};

/** @} */

#endif // CLICKANDGOWIDGET_H
