/*
  Q Light Controller Plus
  vcwidget.h

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

#ifndef VCWIDGET_H
#define VCWIDGET_H

#include <QQuickView>
#include <QQuickItem>
#include <QObject>
#include <QColor>
#include <QRect>

#include "qlcfile.h"

class QDomElement;
class Doc;

#define KXMLQLCVCCaption "Caption"
#define KXMLQLCVCFrameStyle "FrameStyle"

#define KXMLQLCVCWidgetID "ID"
#define KXMLQLCVCWidgetPage "Page"
#define KXMLQLCVCWidgetAppearance "Appearance"

#define KXMLQLCVCWidgetForegroundColor "ForegroundColor"
#define KXMLQLCVCWidgetBackgroundColor "BackgroundColor"
#define KXMLQLCVCWidgetColorDefault "Default"

#define KXMLQLCVCWidgetFont "Font"
#define KXMLQLCVCWidgetFontDefault "Default"

#define KXMLQLCVCWidgetBackgroundImage "BackgroundImage"
#define KXMLQLCVCWidgetBackgroundImageNone "None"

#define KXMLQLCVCWidgetInput "Input"
#define KXMLQLCVCWidgetInputUniverse "Universe"
#define KXMLQLCVCWidgetInputChannel "Channel"

#define KXMLQLCWindowState "WindowState"
#define KXMLQLCWindowStateVisible "Visible"
#define KXMLQLCWindowStateX "X"
#define KXMLQLCWindowStateY "Y"
#define KXMLQLCWindowStateWidth "Width"
#define KXMLQLCWindowStateHeight "Height"

class VCWidget : public QObject
{
    Q_OBJECT

    Q_PROPERTY(quint32 id READ id CONSTANT)
    Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry NOTIFY geometryChanged)
    Q_PROPERTY(bool isDisabled READ isDisabled WRITE setDisabled NOTIFY disabledStateChanged)
    Q_PROPERTY(QString caption READ caption WRITE setCaption NOTIFY captionChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(QColor foregroundColor READ foregroundColor WRITE setForegroundColor NOTIFY foregroundColorChanged)
    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)
    Q_PROPERTY(int page READ page WRITE setPage NOTIFY pageChanged)

    /*********************************************************************
     * Initialization
     *********************************************************************/

public:
    VCWidget(Doc* doc = NULL, QObject* parent = 0);
    virtual ~VCWidget();

    void setDocModified();

    virtual void render(QQuickView *view, QQuickItem *parent);

protected:
    Doc* m_doc;

    /*********************************************************************
     * ID
     *********************************************************************/
public:
    /**
     * Set this widget's unique ID
     *
     * @param id This widget's unique ID
     */
    virtual void setID(quint32 id);

    /**
     * Get this widget's unique ID
     */
    quint32 id() const;

    /**
     * Get the value for an invalid widget ID (for comparison etc.)
     */
    static quint32 invalidId();

private:
    quint32 m_id;

    /*********************************************************************
     * Type
     *********************************************************************/
public:
    enum WidgetType
    {
        UnknownWidget,
        ButtonWidget,
        SliderWidget,
        XYPadWidget,
        FrameWidget,
        SoloFrameWidget,
        SpeedDialWidget,
        CueListWidget,
        LabelWidget,
        AudioTriggersWidget,
        AnimationWidget
    };

public:
    /** Set the widget's type */
    void setType(int type);

    /** Get the widget's type */
    int type();

    /** Convert a widget's type to a string */
    static QString typeToString(int type);

    /** Convert a widget's type to a SVG icon */
    static QString typeToIcon(int type);

protected:
    int m_type;

    /*********************************************************************
     * Geometry
     *********************************************************************/
public:
    /** Get this widget's geometry */
    QRect geometry() const;

    /** Set this widget's geometry. x/y position is relative to
     *  the widget's parent */
    void setGeometry(QRect rect);

signals:
    void geometryChanged(QRect rect);

protected:
    QRect m_geometry;

    /*********************************************************************
     * Disable state
     *********************************************************************/
    /**
     * A widget in disable state cannot be clicked and won't accept external
     * input signals.
     * Only VC Frames can set/unset the disable state of themselves and
     * their children widgets.
     */
public:
    /** Get the widget's disable state */
    bool isDisabled();

    /** Set the widget's disable state flag */
    void setDisabled(bool disable);

signals:
    void disabledStateChanged(bool state);

protected:
    bool m_isDisabled;

    /*********************************************************************
     * Caption
     *********************************************************************/
public:
    /** Get this widget's caption text */
    QString caption() const;

    /** Set this widget's caption text */
    void setCaption(QString caption);

signals:
    void captionChanged(QString caption);

protected:
    QString m_caption;

    /*********************************************************************
     * Background color
     *********************************************************************/
public:

    /** Get the widget's background color. The color is invalid if the
        widget has a background image. */
    QColor backgroundColor() const;

    /** Set the widget's background color and invalidate background image */
    void setBackgroundColor(QColor backgroundColor);

    /** Check, whether the widget has a custom background color */
    bool hasCustomBackgroundColor() const;

    /** Reset the widget's background color to default */
    void resetBackgroundColor();

signals:
    void backgroundColorChanged(QColor backgroundColor);

protected:
    QColor m_backgroundColor;
    bool m_hasCustomBackgroundColor;

    /*********************************************************************
     * Foreground color
     *********************************************************************/
public:
    /** Get the widget's foreground color */
    QColor foregroundColor() const;

    /** Set the widget's foreground color */
    void setForegroundColor(QColor foregroundColor);

    /** Check, whether the widget has a custom foreground color */
    bool hasCustomForegroundColor() const;

    /** Reset the widget's foreground color to whatever the platform uses */
    void resetForegroundColor();

signals:
    void foregroundColorChanged(QColor foregroundColor);

protected:
    QColor m_foregroundColor;
    bool m_hasCustomForegroundColor;

    /*********************************************************************
     * Font
     *********************************************************************/
public:
    /** Set the font used for the widget's caption */
    void setFont(const QFont& font);

    /** Get the font used for the widget's caption */
    QFont font() const;

    /** Check, whether the widget has a custom font */
    bool hasCustomFont() const;

    /** Reset the font used for the widget's caption to the default */
    void resetFont();

signals:
    void fontChanged();

protected:
    QFont m_font;
    bool m_hasCustomFont;

    /*********************************************************************
     * Page
     *********************************************************************/
public:
    void setPage(int pNum);
    int page();

signals:
    void pageChanged(int page);

protected:
    int m_page;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    virtual bool loadXML(const QDomElement* vc_root);

protected:
    bool loadXMLCommon(const QDomElement* root);

    /**
     * Read this widget's appearance XML tag, to load properties
     * such as background and foreground color, font, etc..
     */
    bool loadXMLAppearance(const QDomElement* appearance_root);

    /**
     * Read this widget's geometry and visibility from an XML tag.
     *
     * @param tag A QDomElement under which the window state is saved
     * @param x Loaded x position
     * @param y Loaded y position
     * @param w Loaded w position
     * @param h Loaded h position
     * @param visible Loaded visible status
     *
     * @return true if succesful, otherwise false
     */
    bool loadXMLWindowState(const QDomElement* tag, int* x, int* y,
                            int* w, int* h, bool* visible);

};

#endif
