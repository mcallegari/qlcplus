/*
  Q Light Controller
  vcxypad.cpp

  Copyright (c) Heikki Junnila, Stefan Krumm

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

#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QMouseEvent>
#include <QMessageBox>
#include <QGridLayout>
#include <QByteArray>
#include <QPainter>
#include <QPixmap>
#include <QCursor>
#include <QSlider>
#include <QDebug>
#include <QPoint>
#include <QMenu>
#include <QList>
#include <QtXml>

#include "qlcmacros.h"
#include "qlcfile.h"

#include "vcpropertieseditor.h"
#include "vcxypadproperties.h"
#include "qlcinputchannel.h"
#include "virtualconsole.h"
#include "ctkrangeslider.h"
#include "mastertimer.h"
#include "vcxypadarea.h"
#include "inputpatch.h"
#include "vcxypad.h"
#include "fixture.h"
#include "apputil.h"
#include "doc.h"

const quint8 VCXYPad::panInputSourceId = 0;
const quint8 VCXYPad::tiltInputSourceId = 1;

/*****************************************************************************
 * VCXYPad Initialization
 *****************************************************************************/

VCXYPad::VCXYPad(QWidget* parent, Doc* doc) : VCWidget(parent, doc)
{
    /* Set the class name "VCXYPad" as the object name as well */
    setObjectName(VCXYPad::staticMetaObject.className());

    m_hbox = new QHBoxLayout(this);
    m_lvbox = new QVBoxLayout;
    m_lvbox->addSpacing(20);
    m_vRangeSlider = new ctkRangeSlider(this);
    m_lvbox->addWidget(m_vRangeSlider);
    m_lvbox->addSpacing(25);

    m_hbox->addLayout(m_lvbox);

    m_cvbox = new QVBoxLayout;
    m_hbox->addLayout(m_cvbox);

    m_hRangeSlider = new ctkRangeSlider(Qt::Horizontal, this);
    m_cvbox->addWidget(m_hRangeSlider);

    m_area = new VCXYPadArea(this);
    m_cvbox->addWidget(m_area);

    m_hSlider = new QSlider(Qt::Horizontal, this);
    m_cvbox->addWidget(m_hSlider);

    m_rvbox = new QVBoxLayout;
    m_hbox->addLayout(m_rvbox);
    m_rvbox->addSpacing(20);
    m_vSlider = new QSlider(this);
    m_rvbox->addWidget(m_vSlider);
    m_rvbox->addSpacing(25);

    m_vSlider->setRange(0, 255);
    m_hSlider->setRange(0, 255);
    m_vSlider->setInvertedAppearance(true);
    m_vSlider->setTickPosition(QSlider::TicksLeft);
    m_vSlider->setTickInterval(16);
    m_hSlider->setTickPosition(QSlider::TicksAbove);
    m_hSlider->setTickInterval(16);
    m_vSlider->setStyle(AppUtil::saneStyle());
    m_hSlider->setStyle(AppUtil::saneStyle());

    m_hRangeSlider->setRange(0, 255);
    m_vRangeSlider->setInvertedAppearance(true);
    m_vRangeSlider->setRange(0, 255);
    m_hRangeSlider->setMaximumPosition(255);
    m_vRangeSlider->setMaximumPosition(255);

    connect(m_area, SIGNAL(positionChanged(const QPoint&)),
            this, SLOT(slotPositionChanged(const QPoint&)));
    connect(m_vSlider, SIGNAL(valueChanged(int)),
            this, SLOT(slotSliderValueChanged()));
    connect(m_hSlider, SIGNAL(valueChanged(int)),
            this, SLOT(slotSliderValueChanged()));
    connect(m_hRangeSlider, SIGNAL(positionsChanged(int,int)),
            this, SLOT(slotRangeValueChanged()));
    connect(m_vRangeSlider, SIGNAL(positionsChanged(int,int)),
            this, SLOT(slotRangeValueChanged()));

    setFrameStyle(KVCFrameStyleSunken);
    setType(VCWidget::XYPadWidget);
    setCaption("XY Pad");
    setMinimumSize(20, 20);

    QSettings settings;
    QVariant var = settings.value(SETTINGS_XYPAD_SIZE);
    if (var.isValid() == true)
        resize(var.toSize());
    else
        resize(QSize(230, 230));
    m_padInteraction = false;
    m_sliderInteraction = false;
    m_inputValueChanged = false;

    slotModeChanged(Doc::Design);
}

VCXYPad::~VCXYPad()
{
    m_doc->masterTimer()->unregisterDMXSource(this);
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

VCWidget* VCXYPad::createCopy(VCWidget* parent)
{
    Q_ASSERT(parent != NULL);

    VCXYPad* xypad = new VCXYPad(parent, m_doc);
    if (xypad->copyFrom(this) == false)
    {
        delete xypad;
        xypad = NULL;
    }

    return xypad;
}

bool VCXYPad::copyFrom(VCWidget* widget)
{
    VCXYPad* xypad = qobject_cast <VCXYPad*> (widget);
    if (xypad == NULL)
        return false;
    resize(xypad->size());

    /* Get rid of existing channels */
    m_fixtures.clear();

    /* Copy the other widget's fixtures */
    m_fixtures = xypad->fixtures();

    /* Copy the current position */
    m_area->setPosition(xypad->m_area->position());
    m_vSlider->setValue(xypad->m_vSlider->value());
    m_hSlider->setValue(xypad->m_hSlider->value());

    /* Copy common stuff */
    return VCWidget::copyFrom(widget);
}

/*****************************************************************************
 * Caption
 *****************************************************************************/

void VCXYPad::setCaption(const QString& text)
{
    m_area->setWindowTitle(text);
    VCWidget::setCaption(text);
}

bool VCXYPad::invertedAppearance() const
{
    return !(m_vSlider->invertedAppearance());
}

void VCXYPad::setInvertedAppearance(bool invert)
{
    if (invert == true)
        m_vSlider->setInvertedAppearance(false);
    else
        m_vSlider->setInvertedAppearance(true);
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCXYPad::editProperties()
{
    VCXYPadProperties prop(this, m_doc);
    if (prop.exec() == QDialog::Accepted)
        m_doc->setModified();
}

/*****************************************************************************
 * Fixtures
 *****************************************************************************/

void VCXYPad::appendFixture(const VCXYPadFixture& fxi)
{
    if (fxi.head().isValid() && m_fixtures.indexOf(fxi) == -1)
        m_fixtures.append(fxi);
}

void VCXYPad::removeFixture(GroupHead const & head)
{
    VCXYPadFixture fixture(m_doc);
    fixture.setHead(head);

    m_fixtures.removeAll(fixture);
}

void VCXYPad::clearFixtures()
{
    m_fixtures.clear();
}

QList <VCXYPadFixture> VCXYPad::fixtures() const
{
    return m_fixtures;
}

/*****************************************************************************
 * Current XY position
 *****************************************************************************/

void VCXYPad::writeDMX(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(timer);

    if (m_area->hasPositionChanged() == true)
    {
        // This call also resets the m_changed flag in m_area
        QPoint pt = m_area->position();

        /* Scale XY coordinate values to 0.0 - 1.0 */
        qreal x = SCALE(qreal(pt.x()), qreal(0), qreal(m_area->width()), qreal(0), qreal(1));
        qreal y = SCALE(qreal(pt.y()), qreal(0), qreal(m_area->height()), qreal(0), qreal(1));

        /* Write values outside of mutex lock to keep UI snappy */
        foreach (VCXYPadFixture fixture, m_fixtures)
            fixture.writeDMX(x, y, universes);
    }
}

void VCXYPad::slotPositionChanged(const QPoint& pt)
{
    if (m_sliderInteraction == true)
        return;

    m_padInteraction = true;
    qreal x = SCALE(qreal(pt.x()), qreal(0), qreal(m_area->width()),
                    qreal(m_hSlider->minimum()), qreal(m_hSlider->maximum()));

    qreal y;
    if (invertedAppearance() == false)
    {
        y = SCALE(qreal(pt.y()), qreal(0), qreal(m_area->height()),
                  qreal(m_vSlider->minimum()), qreal(m_vSlider->maximum()));
    }
    else
    {
        y = SCALE(qreal(pt.y()), qreal(m_area->height()), qreal(0),
                  qreal(m_vSlider->minimum()), qreal(m_vSlider->maximum()));
    }

    m_hSlider->setValue(int(x));
    m_vSlider->setValue(int(y));
    if (m_inputValueChanged == false)
        updateFeedback();
    m_padInteraction = false;
    m_inputValueChanged = false;
}

void VCXYPad::slotSliderValueChanged()
{
    if (m_padInteraction == true)
        return;

    int x = 0, y = 0;

    m_sliderInteraction = true;
    if (QObject::sender() == m_hSlider)
    {
        x = int(SCALE(qreal(m_hSlider->value()),
                      qreal(m_hSlider->minimum()), qreal(m_hSlider->maximum()),
                      qreal(0), qreal(m_area->width())));
        y = m_area->position().y();
    }
    else
    {
        if (invertedAppearance() == false)
            y = int(SCALE(qreal(m_vSlider->value()),
                          qreal(m_vSlider->minimum()), qreal(m_vSlider->maximum()),
                          qreal(0), qreal(m_area->height())));
        else
            y = int(SCALE(qreal(m_vSlider->value()),
                          qreal(m_vSlider->maximum()), qreal(m_vSlider->minimum()),
                          qreal(0), qreal(m_area->height())));

        x = m_area->position().x();
    }

    m_area->setPosition(QPoint(x, y));
    m_area->update();
    updateFeedback();
    m_sliderInteraction = false;
}

void VCXYPad::slotRangeValueChanged()
{
    QRect rect(m_hRangeSlider->minimumPosition(), m_vRangeSlider->minimumPosition(),
               m_hRangeSlider->maximumPosition(), m_vRangeSlider->maximumPosition());
    m_area->setRangeWindow(rect);
    m_area->update();
}

void VCXYPad::updateFeedback()
{
    int Xfb = (int)SCALE(float(m_hSlider->value()), float(m_hSlider->minimum()),
                         float(m_hSlider->maximum()), float(0), float(UCHAR_MAX));
    sendFeedback(Xfb, panInputSourceId);
    int Yfb = (int)SCALE(float(m_vSlider->value()), float(m_vSlider->minimum()),
                         float(m_vSlider->maximum()), float(0), float(UCHAR_MAX));
    sendFeedback(Yfb, tiltInputSourceId);
}

/*****************************************************************************
 * External input
 *****************************************************************************/

void VCXYPad::slotInputValueChanged(quint32 universe, quint32 channel,
                                     uchar value)
{
    /* Don't let input data thru in design mode */
    if (mode() == Doc::Design)
        return;

    int x = 0, y = 0;

    QLCInputSource src(universe, channel);
    if (src == inputSource(panInputSourceId))
    {

        qreal areaWidth = m_area->width();
        qreal xOffset = 0;
        QRect rangeWindow = m_area->rangeWindow();
        if (rangeWindow.isValid())
        {
            areaWidth = rangeWindow.width();
            xOffset = rangeWindow.x();
        }
        x = xOffset + int(SCALE(qreal(value), qreal(0), qreal(255),
                      qreal(0), qreal(areaWidth)));
        y = m_area->position().y();
    }
    else if (src == inputSource(tiltInputSourceId))
    {
        qreal yOffset = 0;
        qreal areaHeight = m_area->height();
        QRect rangeWindow = m_area->rangeWindow();
        if (rangeWindow.isValid())
        {
            areaHeight = rangeWindow.height();
            yOffset = rangeWindow.y();
        }
        x = m_area->position().x();
        if (invertedAppearance() == false)
            y = yOffset + int(SCALE(qreal(value), qreal(0), qreal(255),
                          qreal(0), qreal(areaHeight)));
        else
            y = yOffset + int(SCALE(qreal(value), qreal(255), qreal(0),
                          qreal(0), qreal(areaHeight)));
    }
    else
        return;

    m_inputValueChanged = true;

    m_area->setPosition(QPoint(x, y));
    m_area->update();
}

/*****************************************************************************
 * QLC mode
 *****************************************************************************/

void VCXYPad::slotModeChanged(Doc::Mode mode)
{
    QMutableListIterator <VCXYPadFixture> it(m_fixtures);
    while (it.hasNext() == true)
    {
        VCXYPadFixture fxi = it.next();
        if (mode == Doc::Operate)
            fxi.arm();
        else
            fxi.disarm();
        it.setValue(fxi);
    }

    if (mode == Doc::Operate)
    {
        m_doc->masterTimer()->registerDMXSource(this);
        m_vSlider->setEnabled(true);
        m_hSlider->setEnabled(true);
    }
    else
    {
        m_doc->masterTimer()->unregisterDMXSource(this);
        m_vSlider->setEnabled(false);
        m_hSlider->setEnabled(false);
    }

    m_area->setMode(mode);

    /* Reset the changed flag in m_area so that the pad won't immediately set a value
       when mode is changed */
    m_area->position();

    VCWidget::slotModeChanged(mode);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCXYPad::loadXML(const QDomElement* root)
{
    bool visible = false;
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    int xpos = 0;
    int ypos = 0;

    QDomNode node;
    QDomElement tag;
    QString str;

    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCVCXYPad)
    {
        qWarning() << Q_FUNC_INFO << "XY Pad node not found";
        return false;
    }

    /* Widget commons */
    loadXMLCommon(root);

    if (root->hasAttribute(KXMLQLCVCXYPadInvertedAppearance))
    {
        if (root->attribute(KXMLQLCVCXYPadInvertedAppearance) == "false")
            setInvertedAppearance(false);
        else
            setInvertedAppearance(true);
    }

    /* Children */
    node = root->firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();
        if (tag.tagName() == KXMLQLCWindowState)
        {
            loadXMLWindowState(&tag, &x, &y, &w, &h, &visible);
        }
        else if (tag.tagName() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(&tag);
        }
        else if (tag.tagName() == KXMLQLCVCXYPadPan)
        {
            quint32 uni = 0, ch = 0;
            xpos = tag.attribute(KXMLQLCVCXYPadPosition).toInt();
            if (loadXMLInput(tag.firstChild().toElement(), &uni, &ch) == true)
                setInputSource(QLCInputSource(uni, ch), panInputSourceId);
        }
        else if (tag.tagName() == KXMLQLCVCXYPadTilt)
        {
            quint32 uni = 0, ch = 0;
            ypos = tag.attribute(KXMLQLCVCXYPadPosition).toInt();
            if (loadXMLInput(tag.firstChild().toElement(), &uni, &ch) == true)
                setInputSource(QLCInputSource(uni, ch), tiltInputSourceId);
        }
        else if (tag.tagName() == KXMLQLCVCXYPadRangeWindow)
        {
            if (tag.hasAttribute(KXMLQLCVCXYPadRangeHorizMin))
                m_hRangeSlider->setMinimumPosition(tag.attribute(KXMLQLCVCXYPadRangeHorizMin).toInt());
            if (tag.hasAttribute(KXMLQLCVCXYPadRangeHorizMax))
                m_hRangeSlider->setMaximumPosition(tag.attribute(KXMLQLCVCXYPadRangeHorizMax).toInt());
            if (tag.hasAttribute(KXMLQLCVCXYPadRangeVertMin))
                m_vRangeSlider->setMinimumPosition(tag.attribute(KXMLQLCVCXYPadRangeVertMin).toInt());
            if (tag.hasAttribute(KXMLQLCVCXYPadRangeVertMax))
                m_vRangeSlider->setMaximumPosition(tag.attribute(KXMLQLCVCXYPadRangeVertMax).toInt());
            slotRangeValueChanged();
        }
        else if (tag.tagName() == KXMLQLCVCXYPadPosition) // Legacy
        {
            str = tag.attribute(KXMLQLCVCXYPadPositionX);
            xpos = str.toInt();

            str = tag.attribute(KXMLQLCVCXYPadPositionY);
            ypos = str.toInt();
        }
        else if (tag.tagName() == KXMLQLCVCXYPadFixture)
        {
            VCXYPadFixture fxi(m_doc);
            if (fxi.loadXML(tag) == true)
                appendFixture(fxi);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown XY Pad tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    setGeometry(x, y, w, h);
    show(); // Qt doesn't update the widget's geometry without this.
    m_area->setPosition(QPoint(xpos, ypos));

    return true;
}

bool VCXYPad::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
    QDomElement root;
    QDomElement tag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(vc_root != NULL);

    /* VC XY Pad entry */
    root = doc->createElement(KXMLQLCVCXYPad);
    vc_root->appendChild(root);

    saveXMLCommon(doc, &root);

    root.setAttribute(KXMLQLCVCXYPadInvertedAppearance, invertedAppearance());

    /* Fixtures */
    foreach (VCXYPadFixture fixture, m_fixtures)
        fixture.saveXML(doc, &root);

    /* Current XY position */
    QPoint pt(m_area->position());

    /* Custom range window */
    if (m_hRangeSlider->minimumPosition() != 0 ||
        m_hRangeSlider->maximumPosition() != 255 ||
        m_vRangeSlider->minimumPosition() != 0 ||
        m_vRangeSlider->maximumPosition() != 255)
    {
        tag = doc->createElement(KXMLQLCVCXYPadRangeWindow);
        tag.setAttribute(KXMLQLCVCXYPadRangeHorizMin, QString::number(m_hRangeSlider->minimumPosition()));
        tag.setAttribute(KXMLQLCVCXYPadRangeHorizMax, QString::number(m_hRangeSlider->maximumPosition()));
        tag.setAttribute(KXMLQLCVCXYPadRangeVertMin, QString::number(m_vRangeSlider->minimumPosition()));
        tag.setAttribute(KXMLQLCVCXYPadRangeVertMax, QString::number(m_vRangeSlider->maximumPosition()));
        root.appendChild(tag);
    }

    /* Pan */
    tag = doc->createElement(KXMLQLCVCXYPadPan);
    tag.setAttribute(KXMLQLCVCXYPadPosition, QString::number(pt.x()));
    saveXMLInput(doc, &tag, inputSource(panInputSourceId));
    root.appendChild(tag);

    /* Tilt */
    tag = doc->createElement(KXMLQLCVCXYPadTilt);
    tag.setAttribute(KXMLQLCVCXYPadPosition, QString::number(pt.y()));
    saveXMLInput(doc, &tag, inputSource(tiltInputSourceId));
    root.appendChild(tag);

    /* Window state */
    saveXMLWindowState(doc, &root);

    /* Appearance */
    saveXMLAppearance(doc, &root);

    return true;
}
