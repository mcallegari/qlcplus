/*
  Q Light Controller Plus
  vcwidget.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "vcwidget.h"
#include "doc.h"

VCWidget::VCWidget(Doc *doc, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_id(invalidId())
    , m_type(UnknownWidget)
    , m_geometry(QRect(0,0,0,0))
    , m_allowResize(true)
    , m_isDisabled(false)
    , m_isVisible(true)
    , m_caption(QString())
    , m_backgroundColor(QColor("#333"))
    , m_hasCustomBackgroundColor(false)
    , m_foregroundColor(QColor(Qt::white))
    , m_hasCustomForegroundColor(false)
    , m_hasCustomFont(false)
    , m_page(0)
    , m_intensity(1.0)
    , m_isEditing(false)
{
    m_font = QFont("Roboto Condensed");
    m_font.setPointSize(10);
}

VCWidget::~VCWidget()
{
}

void VCWidget::setDocModified()
{
    if (m_doc != NULL)
        m_doc->setModified();
}

void VCWidget::render(QQuickView *, QQuickItem *)
{
}

/*****************************************************************************
 * ID
 *****************************************************************************/

void VCWidget::setID(quint32 id)
{
    /* Don't set doc modified status or emit changed signal, because this
       function is called only once during widget creation. */
    m_id = id;
}

quint32 VCWidget::id() const
{
    return m_id;
}

quint32 VCWidget::invalidId()
{
    return UINT_MAX;
}

/*********************************************************************
 * Type
 *********************************************************************/

void VCWidget::setType(int type)
{
    m_type = type;
}

int VCWidget::type()
{
    return m_type;
}

QString VCWidget::typeToString(int type)
{
    switch (type)
    {
        case ButtonWidget: return QString(tr("Button"));
        case SliderWidget: return QString(tr("Slider"));
        case FrameWidget: return QString(tr("Frame"));
        case SoloFrameWidget: return QString(tr("Solo Frame"));
        case SpeedDialWidget: return QString(tr("Speed Dial"));
        case XYPadWidget: return QString(tr("XY Pad"));
        case CueListWidget: return QString(tr("Cue list"));
        case LabelWidget: return QString(tr("Label"));
        case AudioTriggersWidget: return QString(tr("Audio Triggers"));
        case AnimationWidget: return QString(tr("Animation"));
        case ClockWidget: return QString(tr("Clock"));
        case UnknownWidget:
        default:
             return QString(tr("Unknown"));
    }
    return QString(tr("Unknown"));
}

QString VCWidget::typeToIcon(int type)
{
    switch (type)
    {
        case ButtonWidget: return QString("qrc:/button.svg");
        case SliderWidget: return QString("qrc:/slider.svg");
        case FrameWidget: return QString("qrc:/frame.svg");
        case SoloFrameWidget: return QString("qrc:/soloframe.svg");
        case SpeedDialWidget: return QString("qrc:/speed.svg");
        case XYPadWidget: return QString("qrc:/xypad.svg");
        case CueListWidget: return QString("qrc:/cuelist.svg");
        case LabelWidget: return QString("qrc:/label.svg");
        case AudioTriggersWidget: return QString("qrc:/audioinput.svg");
        case AnimationWidget: return QString("qrc:/rgbmatrix.svg");
        case ClockWidget: return QString("qrc:/clock.svg");
        case UnknownWidget:
        default:
             return QString("qrc:/virtualconsole.svg");
    }
    return QString("qrc:/virtualconsole.svg");
}

VCWidget::WidgetType VCWidget::stringToType(QString str)
{
    if (str == "Button") return ButtonWidget;
    else if (str == "Slider") return SliderWidget;
    else if (str == "XYPad") return XYPadWidget;
    else if (str == "Frame") return FrameWidget;
    else if (str == "Solo frame") return SoloFrameWidget;
    else if (str == "Speed dial") return SpeedDialWidget;
    else if (str == "Cue list") return CueListWidget;
    else if (str == "Label") return LabelWidget;
    else if (str == "Audio Triggers") return AudioTriggersWidget;
    else if (str == "Animation") return AnimationWidget;
    else if (str == "Clock") return ClockWidget;

    return UnknownWidget;
}

/*********************************************************************
 * Geometry
 *********************************************************************/

QRect VCWidget::geometry() const
{
    return m_geometry;
}

void VCWidget::setGeometry(QRect rect)
{
    if (m_geometry == rect)
        return;

    m_geometry = rect;
    setDocModified();
    emit geometryChanged(rect);
}

/*********************************************************************
 * Allow resize
 *********************************************************************/

bool VCWidget::allowResize() const
{
    return m_allowResize;
}

void VCWidget::setAllowResize(bool allowResize)
{
    if (m_allowResize == allowResize)
        return;

    m_allowResize = allowResize;
    emit allowResizeChanged(allowResize);
}

/*********************************************************************
 * Disable state
 *********************************************************************/

bool VCWidget::isDisabled()
{
    return m_isDisabled;
}

void VCWidget::setDisabled(bool disable)
{
    if (m_isDisabled == disable)
        return;

    m_isDisabled = disable;
    setDocModified();
    emit disabledStateChanged(disable);
}

/*********************************************************************
 * Visibility state
 *********************************************************************/

void VCWidget::setVisible(bool isVisible)
{
    if (m_isVisible == isVisible)
        return;

    m_isVisible = isVisible;
    emit isVisibleChanged(isVisible);
}

bool VCWidget::isVisible() const
{
    return m_isVisible;
}

/*****************************************************************************
 * Caption
 *****************************************************************************/

QString VCWidget::caption() const
{
    return m_caption;
}

void VCWidget::setCaption(QString caption)
{
    if (m_caption == caption)
        return;

    m_caption = caption;
    setDocModified();
    emit captionChanged(caption);
}

/*****************************************************************************
 * Background color
 *****************************************************************************/

QColor VCWidget::backgroundColor() const
{
    return m_backgroundColor;
}

void VCWidget::setBackgroundColor(QColor backgroundColor)
{
    if (m_backgroundColor == backgroundColor)
        return;

    m_backgroundColor = backgroundColor;
    m_hasCustomBackgroundColor = true;
    setDocModified();
    emit backgroundColorChanged(backgroundColor);
}

bool VCWidget::hasCustomBackgroundColor() const
{
    return m_hasCustomBackgroundColor;
}

void VCWidget::resetBackgroundColor()
{
    m_hasCustomBackgroundColor = false;
    m_backgroundColor = Qt::gray;
    setDocModified();
    emit backgroundColorChanged(m_backgroundColor);
}

/*****************************************************************************
 * Foreground color
 *****************************************************************************/

QColor VCWidget::foregroundColor() const
{
    return m_foregroundColor;
}

void VCWidget::setForegroundColor(QColor foregroundColor)
{
    if (m_foregroundColor == foregroundColor)
        return;

    m_foregroundColor = foregroundColor;
    m_hasCustomForegroundColor = true;
    setDocModified();
    emit foregroundColorChanged(foregroundColor);
}

bool VCWidget::hasCustomForegroundColor() const
{
    return m_hasCustomForegroundColor;
}

void VCWidget::resetForegroundColor()
{
    m_hasCustomForegroundColor = false;
    m_foregroundColor = Qt::white;
    setDocModified();
    emit foregroundColorChanged(m_foregroundColor);
}

/*********************************************************************
 * Font
 *********************************************************************/

void VCWidget::setFont(const QFont& font)
{
    m_hasCustomFont = true;
    m_font = font;
    setDocModified();
    emit fontChanged();
}

QFont VCWidget::font() const
{
    return m_font;
}

bool VCWidget::hasCustomFont() const
{
    return m_hasCustomFont;
}

void VCWidget::resetFont()
{
    m_font = QFont("Roboto Condensed");
    m_font.setPointSize(16);
    m_hasCustomFont = false;
    setDocModified();
    emit fontChanged();
}

/*********************************************************************
 * Page
 *********************************************************************/

void VCWidget::setPage(int pNum)
{
    if (pNum == m_page)
        return;

    m_page = pNum;
    emit pageChanged(pNum);
}

int VCWidget::page()
{
    return m_page;
}

void VCWidget::notifyFunctionStarting(VCWidget *widget, quint32 fid, qreal fIntensity)
{
    Q_UNUSED(widget)
    Q_UNUSED(fid);
    Q_UNUSED(fIntensity);
}

/*********************************************************************
 * Intensity
 *********************************************************************/

void VCWidget::adjustIntensity(qreal val)
{
    m_intensity = val;
}

qreal VCWidget::intensity()
{
    return m_intensity;
}

/*********************************************************************
 * QML Properties Component
 *********************************************************************/

bool VCWidget::isEditing() const
{
    return m_isEditing;
}

void VCWidget::setIsEditing(bool edit)
{
    if (edit == m_isEditing)
        return;

    m_isEditing = edit;
    emit isEditingChanged();
}

QString VCWidget::propertiesResource() const
{
    return QString();
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCWidget::loadXML(QXmlStreamReader &root)
{
    Q_UNUSED(root)
    return false;
}

bool VCWidget::saveXML(QXmlStreamWriter *doc)
{
    Q_UNUSED(doc)
    return false;
}

bool VCWidget::loadXMLCommon(QXmlStreamReader &root)
{
    if (root.device() == NULL || root.hasError())
        return false;

    QXmlStreamAttributes attrs = root.attributes();

    /* ID */
    if (attrs.hasAttribute(KXMLQLCVCWidgetID))
        setID(attrs.value(KXMLQLCVCWidgetID).toUInt());

    /* Caption */
    if (attrs.hasAttribute(KXMLQLCVCCaption))
        setCaption(attrs.value(KXMLQLCVCCaption).toString());

    /* Page */
    if (attrs.hasAttribute(KXMLQLCVCWidgetPage))
        setPage(attrs.value(KXMLQLCVCWidgetPage).toInt());

    return true;
}

bool VCWidget::loadXMLAppearance(QXmlStreamReader &root)
{
    if (root.device() == NULL || root.hasError())
        return false;

    if (root.name() != KXMLQLCVCWidgetAppearance)
    {
        qWarning() << Q_FUNC_INFO << "Appearance node not found!";
        return false;
    }

    /* Children */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCVCWidgetForegroundColor)
        {
            QString str = root.readElementText();
            if (str != KXMLQLCVCWidgetColorDefault)
                setForegroundColor(QColor(str.toUInt()));
            else if (hasCustomForegroundColor() == true)
                resetForegroundColor();
        }
        else if (root.name() == KXMLQLCVCWidgetBackgroundColor)
        {
            QString str = root.readElementText();
            if (str != KXMLQLCVCWidgetColorDefault)
                setBackgroundColor(QColor(str.toUInt()));
        }
/*
        else if (root.name() == KXMLQLCVCWidgetBackgroundImage)
        {
            QString str = root.readElementText();
            if (str != KXMLQLCVCWidgetBackgroundImageNone)
                setBackgroundImage(m_doc->denormalizeComponentPath(str));
        }
*/
        else if (root.name() == KXMLQLCVCWidgetFont)
        {
            QString str = root.readElementText();
            if (str != KXMLQLCVCWidgetFontDefault)
            {
                QFont font;
                font.fromString(str);
                setFont(font);
            }
        }
        else if (root.name() == KXMLQLCVCFrameStyle)
        {
            /** LEGACY: no more supported/needed */
            root.skipCurrentElement();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown appearance tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool VCWidget::loadXMLWindowState(QXmlStreamReader &root, int* x, int* y,
                                  int* w, int* h, bool* visible)
{
    if (root.device() == NULL || x == NULL || y == NULL || w == NULL || h == NULL ||
            visible == NULL)
        return false;

    if (root.name() == KXMLQLCWindowState)
    {
        QXmlStreamAttributes attrs = root.attributes();
        *x = attrs.value(KXMLQLCWindowStateX).toInt();
        *y = attrs.value(KXMLQLCWindowStateY).toInt();
        *w = attrs.value(KXMLQLCWindowStateWidth).toInt();
        *h = attrs.value(KXMLQLCWindowStateHeight).toInt();

        if (attrs.value(KXMLQLCWindowStateVisible).toString() == KXMLQLCTrue)
            *visible = true;
        else
            *visible = false;
        root.skipCurrentElement();

        return true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "Window state not found";
        return false;
    }
}

bool VCWidget::saveXMLCommon(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* Caption */
    doc->writeAttribute(KXMLQLCVCCaption, caption());

    /* ID */
    if (id() != VCWidget::invalidId())
        doc->writeAttribute(KXMLQLCVCWidgetID, QString::number(id()));

    /* Page */
    if (page() != 0)
        doc->writeAttribute(KXMLQLCVCWidgetPage, QString::number(page()));

    return true;
}

bool VCWidget::saveXMLAppearance(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    QString str;

    if (hasCustomForegroundColor() == false &&
        hasCustomBackgroundColor() == false &&
        //backgroundImage().isEmpty() &&
        hasCustomFont() == false)
            return true;

    /* VC widget appearance entry */
    doc->writeStartElement(KXMLQLCVCWidgetAppearance);

    /* Foreground color */
    if (hasCustomForegroundColor() == true)
    {
        str.setNum(foregroundColor().rgb());
    //else
    //    str = KXMLQLCVCWidgetColorDefault;
        doc->writeTextElement(KXMLQLCVCWidgetForegroundColor, str);
    }

    /* Background color */
    if (hasCustomBackgroundColor() == true)
    {
        str.setNum(backgroundColor().rgb());
    //else
    //    str = KXMLQLCVCWidgetColorDefault;
        doc->writeTextElement(KXMLQLCVCWidgetBackgroundColor, str);
    }

#if 0 // TODO
    /* Background image */
    if (backgroundImage().isEmpty() == false)
    {
        str = m_doc->normalizeComponentPath(m_backgroundImage);
    //else
    //    str = KXMLQLCVCWidgetBackgroundImageNone;
        doc->writeTextElement(KXMLQLCVCWidgetBackgroundImage, str);
    }
#endif

    /* Font */
    if (hasCustomFont() == true)
    {
        str = font().toString();
    //else
    //    str = KXMLQLCVCWidgetFontDefault;
        doc->writeTextElement(KXMLQLCVCWidgetFont, str);
    }

    /* End the <Appearance> tag */
    doc->writeEndElement();

    return true;
}

bool VCWidget::saveXMLWindowState(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    QRect r = geometry();

    /* Window state tag */
    doc->writeStartElement(KXMLQLCWindowState);

    /* Visible status */
    if (isVisible() == true)
        doc->writeAttribute(KXMLQLCWindowStateVisible, KXMLQLCTrue);
    else
        doc->writeAttribute(KXMLQLCWindowStateVisible, KXMLQLCFalse);

    doc->writeAttribute(KXMLQLCWindowStateX, QString::number(r.x()));
    doc->writeAttribute(KXMLQLCWindowStateY, QString::number(r.y()));
    doc->writeAttribute(KXMLQLCWindowStateWidth, QString::number(r.width()));
    doc->writeAttribute(KXMLQLCWindowStateHeight, QString::number(r.height()));

    doc->writeEndElement();

    return true;
}

