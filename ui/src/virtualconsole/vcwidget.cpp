/*
  Q Light Controller
  vcwidget.cpp

  Copyright (c) Heikki Junnila

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

#include <QStyleOptionFrame>
#include <QApplication>
#include <QInputDialog>
#include <QColorDialog>
#include <QFileDialog>
#include <QFontDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QMetaObject>
#include <QPainter>
#include <QPalette>
#include <QCursor>
#include <QPixmap>
#include <QDebug>
#include <QBrush>
#include <QPoint>
#include <QStyle>
#include <QSize>
#include <QMenu>
#include <QList>
#include <QtXml>
#include <cmath>

#include "qlcinputsource.h"
#include "qlcfile.h"

#include "qlcinputchannel.h"
#include "virtualconsole.h"
#include "vcproperties.h"
#include "inputpatch.h"
#include "vcwidget.h"
#include "doc.h"

#define GRID_RESOLUTION 5

VCWidget::VCWidget(QWidget* parent, Doc* doc)
    : QWidget(parent)
    , m_doc(doc)
    , m_id(invalidId())
    , m_disableState(false)
    , m_page(0)
    , m_allowChildren(false)
    , m_allowResize(true)
    , m_intensity(1.0)
    , m_liveEdit(VirtualConsole::instance()->liveEdit())
{
    Q_ASSERT(parent != NULL);
    Q_ASSERT(doc != NULL);

    /* Set the class name "VCWidget" as the object name as well */
    setObjectName(VCWidget::staticMetaObject.className());

    setMinimumSize(QSize(20, 20));

    m_type = UnknownWidget;
    m_hasCustomBackgroundColor = false;
    m_hasCustomForegroundColor = false;
    m_hasCustomFont = false;
    m_frameStyle = KVCFrameStyleNone;

    m_resizeMode = false;

    setBackgroundRole(QPalette::Window);
    setAutoFillBackground(true);
    setEnabled(true);

    connect(m_doc, SIGNAL(modeChanged(Doc::Mode)),
            this, SLOT(slotModeChanged(Doc::Mode)));

    /* Listen to parent's (only VCWidget-kind) key signals */
    if (parent->inherits(metaObject()->className()) == true)
    {
        connect(parent, SIGNAL(keyPressed(const QKeySequence&)),
                this, SLOT(slotKeyPressed(const QKeySequence&)));
        connect(parent,	SIGNAL(keyReleased(const QKeySequence&)),
                this, SLOT(slotKeyReleased(const QKeySequence&)));
    }
}

VCWidget::~VCWidget()
{
    qDeleteAll(m_inputs);
    m_inputs.clear();
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
        case XYPadWidget: return QString(tr("XYPad"));
        case FrameWidget: return QString(tr("Frame"));
        case SoloFrameWidget: return QString(tr("Solo frame"));
        case SpeedDialWidget: return QString(tr("Speed dial"));
        case CueListWidget: return QString(tr("Cue list"));
        case LabelWidget: return QString(tr("Label"));
        case AudioTriggersWidget: return QString(tr("Audio Triggers"));
        case AnimationWidget: return QString(tr("Animation"));
        case UnknownWidget:
        default:
             return QString(tr("Unknown"));
    }
    return QString(tr("Unknown"));
}

QIcon VCWidget::typeToIcon(int type)
{
    switch (type)
    {
        case ButtonWidget: return QIcon(":/button.png");
        case SliderWidget: return QIcon(":/slider.png");
        case XYPadWidget: return QIcon(":/xypad.png");
        case FrameWidget: return QIcon(":/frame.png");
        case SoloFrameWidget: return QIcon(":/soloframe.png");
        case SpeedDialWidget: return QIcon(":/speed.png");
        case CueListWidget: return QIcon(":/cuelist.png");
        case LabelWidget: return QIcon(":/label.png");
        case AudioTriggersWidget: return QIcon(":/audioinput.png");
        case AnimationWidget: return QIcon(":/rgbmatrix.png");
        case UnknownWidget:
        default:
             return QIcon(":/virtualconsole.png");
    }
    return QIcon(":/virtualconsole.png");
}

/*********************************************************************
 * Disable state
 *********************************************************************/

void VCWidget::setDisableState(bool disable)
{
    m_disableState = disable;
    if (mode() == Doc::Operate)
    {
        setEnabled(!disable);
        enableWidgetUI(!disable);
    }
}

void VCWidget::enableWidgetUI(bool enable)
{
    Q_UNUSED(enable)
}

bool VCWidget::isDisabled()
{
    return m_disableState;
}

/*********************************************************************
 * Page
 *********************************************************************/

void VCWidget::setPage(int pNum)
{
    m_page = pNum;
}

int VCWidget::page()
{
    return m_page;
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

bool VCWidget::copyFrom(const VCWidget* widget)
{
    if (widget == NULL)
        return false;

    m_backgroundImage = widget->m_backgroundImage;

    m_hasCustomBackgroundColor = widget->m_hasCustomBackgroundColor;
    if (m_hasCustomBackgroundColor == true)
        setBackgroundColor(widget->backgroundColor());

    m_hasCustomForegroundColor = widget->m_hasCustomForegroundColor;
    if (m_hasCustomForegroundColor == true)
        setForegroundColor(widget->foregroundColor());

    m_hasCustomFont = widget->m_hasCustomFont;
    if (m_hasCustomFont == true)
        setFont(widget->font());

    m_frameStyle = widget->m_frameStyle;

    setGeometry(widget->geometry());
    setCaption(widget->caption());

    m_allowChildren = widget->m_allowChildren;
    m_allowResize = widget->m_allowResize;

    QHashIterator <quint8, QLCInputSource*> it(widget->m_inputs);
    while (it.hasNext() == true)
    {
        it.next();
        quint8 id = it.key();
        QLCInputSource *src = new QLCInputSource(it.value()->universe(), it.value()->channel());
        setInputSource(src, id);
    }

    m_page = widget->m_page;

    return true;
}

/*****************************************************************************
 * Background image
 *****************************************************************************/

void VCWidget::setBackgroundImage(const QString& path)
{
    QPalette pal = palette();

    m_hasCustomBackgroundColor = false;
    m_backgroundImage = path;

    /* setAutoFillBackground(true); */
    pal.setBrush(QPalette::Window, QBrush(QPixmap(path)));
    setPalette(pal);

    m_doc->setModified();
}

QString VCWidget::backgroundImage() const
{
    return m_backgroundImage;
}


/*****************************************************************************
 * Background color
 *****************************************************************************/

void VCWidget::setBackgroundColor(const QColor& color)
{
    QPalette pal = palette();

    m_hasCustomBackgroundColor = true;
    m_backgroundImage = QString();

    pal.setColor(QPalette::Window, color);
    setPalette(pal);

    m_doc->setModified();
}

QColor VCWidget::backgroundColor() const
{
    return palette().color(QPalette::Window);
}

bool VCWidget::hasCustomBackgroundColor() const
{
    return m_hasCustomBackgroundColor;
}

void VCWidget::resetBackgroundColor()
{
    QColor fg;

    m_hasCustomBackgroundColor = false;
    m_backgroundImage = QString();

    /* Store foreground color */
    if (m_hasCustomForegroundColor == true)
        fg = palette().color(QPalette::WindowText);

    /* Reset the whole palette to application palette */
    setPalette(QApplication::palette());
    /* setAutoFillBackground(false); */

    /* Restore foreground color */
    if (fg.isValid() == true)
    {
        QPalette pal = palette();
        pal.setColor(QPalette::WindowText, fg);
        setPalette(pal);
    }

    m_doc->setModified();
}

/*****************************************************************************
 * Foreground color
 *****************************************************************************/

void VCWidget::setForegroundColor(const QColor& color)
{
    QPalette pal = palette();

    m_hasCustomForegroundColor = true;

    pal.setColor(QPalette::WindowText, color);
    setPalette(pal);

    m_doc->setModified();
}

QColor VCWidget::foregroundColor() const
{
    return palette().color(QPalette::WindowText);
}

bool VCWidget::hasCustomForegroundColor() const
{
    return m_hasCustomForegroundColor;
}

void VCWidget::resetForegroundColor()
{
    QColor bg;

    m_hasCustomForegroundColor = false;

    /* Store background color */
    if (hasCustomBackgroundColor() == true)
        bg = palette().color(QPalette::Window);

    /* Reset the whole palette to application palette */
    setPalette(QApplication::palette());

    /* Restore foreground color (the first two emit Doc::modified() signal) */
    if (bg.isValid() == true)
        setBackgroundColor(bg);
    else if (backgroundImage().isEmpty() == false)
        setBackgroundImage(backgroundImage());
    else
        m_doc->setModified();
}

/*****************************************************************************
 * Font
 *****************************************************************************/

void VCWidget::setFont(const QFont& font)
{
    m_hasCustomFont = true;
    QWidget::setFont(font);
    m_doc->setModified();
}

QFont VCWidget::font() const
{
    return QWidget::font();
}

bool VCWidget::hasCustomFont() const
{
    return m_hasCustomFont;
}

void VCWidget::resetFont()
{
    QWidget::setFont(QApplication::font());
    m_hasCustomFont = false;
    m_doc->setModified();
}

/*****************************************************************************
 * Caption
 *****************************************************************************/

void VCWidget::setCaption(const QString& text)
{
    setWindowTitle(text);
    update();
    m_doc->setModified();
}

QString VCWidget::caption() const
{
    return windowTitle();
}

/*****************************************************************************
 * Frame style
 *****************************************************************************/

void VCWidget::setFrameStyle(int style)
{
    m_frameStyle = style;
    update();
    m_doc->setModified();
}

int VCWidget::frameStyle() const
{
    return m_frameStyle;
}

void VCWidget::resetFrameStyle()
{
    setFrameStyle(KVCFrameStyleNone);
}

QString VCWidget::frameStyleToString(int style)
{
    if (style == KVCFrameStyleSunken)
        return "Sunken";
    else if (style == KVCFrameStyleRaised)
        return "Raised";
    else
        return "None";
}

int VCWidget::stringToFrameStyle(const QString& style)
{
    if (style == "Sunken")
        return KVCFrameStyleSunken;
    else if (style == "Raised")
        return KVCFrameStyleRaised;
    else
        return KVCFrameStyleNone;
}

/*****************************************************************************
 * Allow adding children
 *****************************************************************************/

void VCWidget::setAllowChildren(bool allow)
{
    m_allowChildren = allow;
}

bool VCWidget::allowChildren() const
{
    return m_allowChildren;
}

/*********************************************************************
 * Allow resizing
 *********************************************************************/

void VCWidget::setAllowResize(bool allow)
{
    m_allowResize = allow;
}

bool VCWidget::allowResize() const
{
    return m_allowResize;
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCWidget::editProperties()
{
    QMessageBox::information(this, staticMetaObject.className(),
                             tr("This widget has no properties"));
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

/*****************************************************************************
 * External input
 *****************************************************************************/

bool VCWidget::checkInputSource(quint32 universe, quint32 channel,
                                uchar value, QObject *sender, quint32 id)
{
    if (m_inputs.contains(id) == false)
        return false;

    QLCInputSource *src = m_inputs[id];
    if (src == NULL)
        return false;

    if (src->isValid() && src->universe() == universe && src->channel() == channel)
    {
        // if the event has been fired by an external controller
        // and this channel is set to relative mode, inform the input source
        // and don't allow the event to pass through as a synthetic event
        // will be generated by the input source itself
        if (src != sender && src->isRelative())
        {
            src->updateInputValue(value);
            return false;
        }
        return true;
    }

    return false;
}

void VCWidget::setInputSource(QLCInputSource* source, quint8 id)
{
    // Connect when the first valid input source is set
    if (m_inputs.isEmpty() == true && source != NULL && source->isValid() == true)
    {
        connect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotInputValueChanged(quint32,quint32,uchar)));
        connect(m_doc->inputOutputMap(), SIGNAL(profileChanged(quint32,QString)),
                this, SLOT(slotInputProfileChanged(quint32,QString)));
    }

    // Assign or clear
    if (source != NULL && source->isValid() == true)
    {
        m_inputs[id] = source;
        // now check if the source is defined in the associated universe
        // profile and if it is in relative mode
        InputPatch *ip = m_doc->inputOutputMap()->inputPatch(source->universe());
        if (ip != NULL)
        {
            if (ip->profile() != NULL)
            {
                QLCInputChannel *ich = ip->profile()->channel(source->channel());
                if (ich != NULL)
                {
                    if (ich->movementType() == QLCInputChannel::Relative)
                    {
                        source->setWorkingMode(QLCInputSource::Relative);
                        source->setSensitivity(ich->movementSensitivity());
                        connect(source, SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                                this, SLOT(slotInputValueChanged(quint32,quint32,uchar)));
                    }
                }
            }
        }
    }
    else
        m_inputs.remove(id);

    // Disconnect when there are no more input sources present
    if (m_inputs.isEmpty() == true)
    {
        disconnect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotInputValueChanged(quint32,quint32,uchar)));
        disconnect(m_doc->inputOutputMap(), SIGNAL(profileChanged(quint32,QString)),
                   this, SLOT(slotInputProfileChanged(quint32,QString)));
    }
}

QLCInputSource *VCWidget::inputSource(quint8 id) const
{
    if (m_inputs.contains(id) == false)
        return NULL;
    else
        return m_inputs[id];
}

void VCWidget::remapInputSources(int pgNum)
{
    foreach(quint8 s, m_inputs.keys())
    {
        QLCInputSource *src = m_inputs[s];
        src->setPage(pgNum);
        setInputSource(src, s);
    }
}

void VCWidget::sendFeedback(int value, quint8 id)
{
    /* Send input feedback */
    QLCInputSource *src = inputSource(id);
    if (src != NULL && src->isValid() == true)
    {
        // if in relative mode, send a "feedback" to this
        // input source so it can continue to emit values
        // from the right position
        if (src->isRelative())
            src->updateOuputValue(value);

        QString chName = QString();

        InputPatch* pat = m_doc->inputOutputMap()->inputPatch(src->universe());
        if (pat != NULL)
        {
            QLCInputProfile* profile = pat->profile();
            if (profile != NULL)
            {
                QLCInputChannel* ich = profile->channel(src->channel());
                if (ich != NULL)
                    chName = ich->name();
            }
        }

        m_doc->inputOutputMap()->sendFeedBack(src->universe(), src->channel(), value, chName);
    }
}

void VCWidget::slotInputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    Q_UNUSED(universe);
    Q_UNUSED(channel);
    Q_UNUSED(value);
}

void VCWidget::slotInputProfileChanged(quint32 universe, const QString &profileName)
{
    qDebug() << "[VCWdget] input profile changed" << profileName;

    QLCInputProfile *profile = m_doc->inputOutputMap()->profile(profileName);

    foreach(QLCInputSource *source, m_inputs.values())
    {
        if (source != NULL && source->universe() == universe)
        {
            // if the profile has been unset, reset all the valid
            // input sources to work in absolute mode
            if (profile == NULL)
            {
                source->setWorkingMode(QLCInputSource::Absolute);
            }
            else
            {
                QLCInputChannel *ich = profile->channel(source->channel());
                if (ich != NULL)
                {
                    if (ich->movementType() == QLCInputChannel::Absolute)
                        source->setWorkingMode(QLCInputSource::Absolute);
                    else
                    {
                        source->setWorkingMode(QLCInputSource::Relative);
                        source->setSensitivity(ich->movementSensitivity());
                    }
                }
            }
        }
    }
}

/*****************************************************************************
 * Key sequence handler
 *****************************************************************************/

QKeySequence VCWidget::stripKeySequence(const QKeySequence& seq)
{
    /* In QLC 3.2.x it is possible to set shortcuts like CTRL+X, but since
       CTRL is now the tap modifier, it must be stripped away. */
    int keys[4] = { 0, 0, 0, 0 };
    for (int i = 0; i < (int)seq.count() && i < 4; i++)
    {
        if ((seq[i] & Qt::ControlModifier) != 0)
            keys[i] = seq[i] & (~Qt::ControlModifier);
        else
            keys[i] = seq[i];
    }

    return QKeySequence(keys[0], keys[1], keys[2], keys[3]);
}

void VCWidget::slotKeyPressed(const QKeySequence& keySequence)
{
    emit keyPressed(keySequence);
}

void VCWidget::slotKeyReleased(const QKeySequence& keySequence)
{
    emit keyReleased(keySequence);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

void VCWidget::postLoad()
{
    /* NOP */
}

bool VCWidget::loadXMLCommon(const QDomElement *root)
{
    Q_ASSERT(root != NULL);

    /* ID */
    if (root->hasAttribute(KXMLQLCVCWidgetID))
        setID(root->attribute(KXMLQLCVCWidgetID).toUInt());

    /* Caption */
    if (root->hasAttribute(KXMLQLCVCCaption))
        setCaption(root->attribute(KXMLQLCVCCaption));

    /* Page */
    if (root->hasAttribute(KXMLQLCVCWidgetPage))
        setPage(root->attribute(KXMLQLCVCWidgetPage).toInt());

    return true;
}

bool VCWidget::loadXMLAppearance(const QDomElement* root)
{
    QDomNode node;
    QDomElement tag;
    QString str;

    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCVCWidgetAppearance)
    {
        qWarning() << Q_FUNC_INFO << "Appearance node not found!";
        return false;
    }

    /* Children */
    node = root->firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();
        if (tag.tagName() == KXMLQLCVCFrameStyle)
        {
            setFrameStyle(stringToFrameStyle(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCVCWidgetForegroundColor)
        {
            if (tag.text() != KXMLQLCVCWidgetColorDefault)
                setForegroundColor(QColor(tag.text().toUInt()));
            else if (hasCustomForegroundColor() == true)
                resetForegroundColor();
        }
        else if (tag.tagName() == KXMLQLCVCWidgetBackgroundColor)
        {
            if (tag.text() != KXMLQLCVCWidgetColorDefault)
                setBackgroundColor(QColor(tag.text().toUInt()));
        }
        else if (tag.tagName() == KXMLQLCVCWidgetBackgroundImage)
        {
            if (tag.text() != KXMLQLCVCWidgetBackgroundImageNone)
                setBackgroundImage(m_doc->denormalizeComponentPath(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCVCWidgetFont)
        {
            if (tag.text() != KXMLQLCVCWidgetFontDefault)
            {
                QFont font;
                font.fromString(tag.text());
                setFont(font);
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown appearance tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool VCWidget::loadXMLInput(const QDomElement* root)
{
    Q_ASSERT(root != NULL);

    quint32 uni = 0;
    quint32 ch = 0;
    if (loadXMLInput(*root, &uni, &ch) == true)
    {
        setInputSource(new QLCInputSource(uni, ch));
        return true;
    }
    else
    {
        return false;
    }
}

bool VCWidget::loadXMLInput(const QDomElement& root, quint32* uni, quint32* ch) const
{
    if (root.tagName() != KXMLQLCVCWidgetInput)
    {
        qWarning() << Q_FUNC_INFO << "Input node not found!";
        return false;
    }
    else
    {
        *uni = root.attribute(KXMLQLCVCWidgetInputUniverse).toUInt();
        *ch = root.attribute(KXMLQLCVCWidgetInputChannel).toUInt();
    }

    return true;
}

bool VCWidget::saveXMLCommon(QDomDocument *doc, QDomElement *widget_root)
{
    Q_UNUSED(doc)
    Q_ASSERT(widget_root != NULL);

    /* Caption */
    widget_root->setAttribute(KXMLQLCVCCaption, caption());

    /* ID */
    if (id() != VCWidget::invalidId())
        widget_root->setAttribute(KXMLQLCVCWidgetID, id());

    /* Page */
    if (page() != 0)
        widget_root->setAttribute(KXMLQLCVCWidgetPage, page());

    return true;
}

bool VCWidget::saveXMLAppearance(QDomDocument* doc, QDomElement* frame_root)
{
    QDomElement root;
    QDomElement tag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(frame_root != NULL);

    /* VC Label entry */
    root = doc->createElement(KXMLQLCVCWidgetAppearance);
    frame_root->appendChild(root);

    /* Frame style */
    tag = doc->createElement(KXMLQLCVCFrameStyle);
    root.appendChild(tag);
    text = doc->createTextNode(frameStyleToString(frameStyle()));
    tag.appendChild(text);

    /* Foreground color */
    tag = doc->createElement(KXMLQLCVCWidgetForegroundColor);
    root.appendChild(tag);
    if (hasCustomForegroundColor() == true)
        str.setNum(foregroundColor().rgb());
    else
        str = KXMLQLCVCWidgetColorDefault;
    text = doc->createTextNode(str);
    tag.appendChild(text);

    /* Background color */
    tag = doc->createElement(KXMLQLCVCWidgetBackgroundColor);
    root.appendChild(tag);
    if (hasCustomBackgroundColor() == true)
        str.setNum(backgroundColor().rgb());
    else
        str = KXMLQLCVCWidgetColorDefault;
    text = doc->createTextNode(str);
    tag.appendChild(text);

    /* Background image */
    tag = doc->createElement(KXMLQLCVCWidgetBackgroundImage);
    root.appendChild(tag);
    if (backgroundImage().isEmpty() == false)
        str = m_doc->normalizeComponentPath(m_backgroundImage);
    else
        str = KXMLQLCVCWidgetBackgroundImageNone;
    text = doc->createTextNode(str);
    tag.appendChild(text);

    /* Font */
    tag = doc->createElement(KXMLQLCVCWidgetFont);
    root.appendChild(tag);
    if (hasCustomFont() == true)
        str = font().toString();
    else
        str = KXMLQLCVCWidgetFontDefault;
    text = doc->createTextNode(str);
    tag.appendChild(text);

    return true;
}

bool VCWidget::saveXMLInput(QDomDocument* doc, QDomElement* root)
{
    return saveXMLInput(doc, root, inputSource());
}

bool VCWidget::saveXMLInput(QDomDocument* doc, QDomElement* root,
                            const QLCInputSource *src) const
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(root != NULL);

    if (src == NULL)
        return false;

    if (src->isValid() == true)
    {
        QDomElement tag = doc->createElement(KXMLQLCVCWidgetInput);
        tag.setAttribute(KXMLQLCVCWidgetInputUniverse, QString("%1").arg(src->universe()));
        tag.setAttribute(KXMLQLCVCWidgetInputChannel, QString("%1").arg(src->channel()));
        root->appendChild(tag);
    }

    return true;
}

bool VCWidget::saveXMLWindowState(QDomDocument* doc, QDomElement* root)
{
    QDomElement tag;
    QDomText text;
    QString str;

    if (doc == NULL || root == NULL)
        return false;

    /* Window state tag */
    tag = doc->createElement(KXMLQLCWindowState);
    root->appendChild(tag);

    /* Visible status */
    if (isVisible() == true)
        tag.setAttribute(KXMLQLCWindowStateVisible, KXMLQLCTrue);
    else
        tag.setAttribute(KXMLQLCWindowStateVisible, KXMLQLCFalse);

    tag.setAttribute(KXMLQLCWindowStateX, QString::number(x()));
    tag.setAttribute(KXMLQLCWindowStateY, QString::number(y()));
    tag.setAttribute(KXMLQLCWindowStateWidth, QString::number(width()));
    tag.setAttribute(KXMLQLCWindowStateHeight, QString::number(height()));

    return true;
}

bool VCWidget::loadXMLWindowState(const QDomElement* tag, int* x, int* y,
                                  int* w, int* h, bool* visible)
{
    if (tag == NULL || x == NULL || y == NULL || w == NULL || h == NULL ||
            visible == NULL)
        return false;

    if (tag->tagName() == KXMLQLCWindowState)
    {
        *x = tag->attribute(KXMLQLCWindowStateX).toInt();
        *y = tag->attribute(KXMLQLCWindowStateY).toInt();
        *w = tag->attribute(KXMLQLCWindowStateWidth).toInt();
        *h = tag->attribute(KXMLQLCWindowStateHeight).toInt();

        if (tag->attribute(KXMLQLCWindowStateVisible) == KXMLQLCTrue)
            *visible = true;
        else
            *visible = false;

        return true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "Window state not found";
        return false;
    }
}

/*****************************************************************************
 * QLC+ Mode change
 *****************************************************************************/

void VCWidget::setLiveEdit(bool liveEdit)
{
    if (m_doc->mode() == Doc::Design)
        return;

    m_liveEdit = liveEdit;

    if (m_disableState)
        setEnabled(m_liveEdit);
    else
        enableWidgetUI(!m_liveEdit);

    unsetCursor();
    update();
}

void VCWidget::cancelLiveEdit()
{
    m_liveEdit = false;
}

void VCWidget::slotModeChanged(Doc::Mode mode)
{
    // make sure to exit from a 'deep' disable state
    if (mode == Doc::Design)
        setEnabled(true);

    /* Reset mouse cursor */
    unsetCursor();

    /* Force an update to get rid of selection markers */
    update();
}

Doc::Mode VCWidget::mode() const
{
    Q_ASSERT(m_doc != NULL);
    if (m_liveEdit)
        return Doc::Design;
    return m_doc->mode();
}

/*****************************************************************************
 * Widget menu
 *****************************************************************************/

void VCWidget::invokeMenu(const QPoint& point)
{
    /* No point coming here if there is no VC instance */
    VirtualConsole* vc = VirtualConsole::instance();
    if (vc == NULL)
        return;

    QMenu* menu = vc->editMenu();
    Q_ASSERT(menu != NULL);
    menu->exec(point);
}

/*****************************************************************************
 * Custom menu
 *****************************************************************************/

QMenu* VCWidget::customMenu(QMenu* parentMenu)
{
    Q_UNUSED(parentMenu);
    return NULL;
}

/*****************************************************************************
 * Widget move & resize
 *****************************************************************************/

void VCWidget::resize(const QSize& size)
{
    QSize sz(size);

    // Force grid settings
    sz.setWidth(size.width() - (size.width() % GRID_RESOLUTION));
    sz.setHeight(size.height() - (size.height() % GRID_RESOLUTION));

    // Resize
    QWidget::resize(sz);
}

void VCWidget::move(const QPoint& point)
{
    QPoint pt(point);

    // Force grid settings
    pt.setX(point.x() - (point.x() % GRID_RESOLUTION));
    pt.setY(point.y() - (point.y() % GRID_RESOLUTION));

    // Don't move beyond left or right
    if (pt.x() < 0)
        pt.setX(0);
    else if (pt.x() + rect().width() > parentWidget()->width())
        pt.setX(parentWidget()->width() - rect().width());

    // Don't move beyond top or bottom
    if (pt.y() < 0)
        pt.setY(0);
    else if (pt.y() + rect().height() > parentWidget()->height())
        pt.setY(parentWidget()->height() - rect().height());

    // Move
    QWidget::move(pt);

    m_doc->setModified();
}

QPoint VCWidget::lastClickPoint() const
{
    return m_mousePressPoint;
}

/*****************************************************************************
 * Event handlers
 *****************************************************************************/

void VCWidget::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);

    /* No point coming here if there is no VC instance */
    VirtualConsole* vc = VirtualConsole::instance();
    if (vc == NULL)
        return;

    QPainter painter(this);

    /* Draw frame according to style */
    QStyleOptionFrame option;
    option.initFrom(this);

    if (frameStyle() == KVCFrameStyleSunken)
        option.state = QStyle::State_Sunken;
    else if (frameStyle() == KVCFrameStyleRaised)
        option.state = QStyle::State_Raised;
    else
        option.state = QStyle::State_None;

    if (mode() == Doc::Design)
        option.state |= QStyle::State_Enabled;

    /* Draw a frame border if such is specified for this widget */
    if (option.state != QStyle::State_None)
        style()->drawPrimitive(QStyle::PE_Frame, &option, &painter, this);

    QWidget::paintEvent(e);

    /* Draw selection frame */
    if (mode() == Doc::Design && vc->isWidgetSelected(this) == true)
    {
        /* Draw a dotted line around the widget */
        QPen pen(Qt::DashLine);
        pen.setColor(Qt::blue);
        pen.setCapStyle(Qt::RoundCap);
        pen.setWidth(0);
        painter.setPen(pen);
        painter.drawRect(0, 0, rect().width() - 1, rect().height() - 1);

        /* Draw a resize handle */
        if (allowResize() == true)
        {
            QIcon icon(":/resize.png");
            painter.drawPixmap(rect().width() - 16, rect().height() - 16,
                               icon.pixmap(QSize(16, 16), QIcon::Normal, QIcon::On));
        }
    }
}

void VCWidget::mousePressEvent(QMouseEvent* e)
{
    Q_ASSERT(e != NULL);

    if (mode() == Doc::Operate)
    {
        QWidget::mousePressEvent(e);
        return;
    }

    /* Perform widget de/selection in virtualconsole's selection buffer */
    handleWidgetSelection(e);

    /* Resize mode */
    if (m_resizeMode == true)
    {
        setMouseTracking(false);
        m_resizeMode = false;
    }

    /* Move, resize or context menu invocation */
    if (e->button() & Qt::LeftButton || e->button() & Qt::MidButton)
    {
        /* Start moving or resizing based on where the click landed */
        if (e->x() > rect().width() - 10 && e->y() > rect().height() - 10 && allowResize())
        {
            m_resizeMode = true;
            setMouseTracking(true);
            setCursor(QCursor(Qt::SizeFDiagCursor));
        }
        else
        {
            m_mousePressPoint = QPoint(e->x(), e->y());
            setCursor(QCursor(Qt::SizeAllCursor));
        }
    }
    else if (e->button() & Qt::RightButton)
    {
        /* Menu invocation */
        m_mousePressPoint = QPoint(e->x(), e->y());
        invokeMenu(mapToGlobal(e->pos()));
    }
}

void VCWidget::handleWidgetSelection(QMouseEvent* e)
{
    /* No point coming here if there is no VC */
    VirtualConsole* vc = VirtualConsole::instance();
    if (vc == NULL)
        return;

    /* Widget selection logic (like in Qt Designer) */
    if (e->button() == Qt::LeftButton)
    {
        if (e->modifiers() & Qt::ShiftModifier)
        {
            /* Toggle selection with LMB when shift is pressed */
            bool selected = vc->isWidgetSelected(this);
            vc->setWidgetSelected(this, !selected);
        }
        else
        {
            if (vc->isWidgetSelected(this) == false)
            {
                /* Select only this */
                vc->clearWidgetSelection();
                vc->setWidgetSelected(this, true);
            }
        }
    }
    else if (e->button() == Qt::RightButton)
    {
        if (vc->isWidgetSelected(this) == false)
        {
            /* Select only this */
            vc->clearWidgetSelection();
            vc->setWidgetSelected(this, true);
        }
    }
}

void VCWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (mode() == Doc::Design)
    {
        unsetCursor();
        m_resizeMode = false;
        setMouseTracking(false);
    }
    else
    {
        QWidget::mouseReleaseEvent(e);
    }
}

void VCWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (mode() == Doc::Design)
        editProperties();
    else
        QWidget::mouseDoubleClickEvent(e);
}

void VCWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (mode() == Doc::Design)
    {
        if (m_resizeMode == true)
        {
            QPoint p = mapToParent(e->pos());
            resize(QSize(p.x() - x(), p.y() - y()));
            m_doc->setModified();
        }
        else if (e->buttons() & Qt::LeftButton || e->buttons() & Qt::MidButton)
        {
            QPoint p = mapToParent(e->pos());
            p.setX(p.x() - m_mousePressPoint.x());
            p.setY(p.y() - m_mousePressPoint.y());

            move(p);
            m_doc->setModified();
        }
    }
    else
    {
        QWidget::mouseMoveEvent(e);
    }
}

