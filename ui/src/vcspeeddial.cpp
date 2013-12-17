/*
  Q Light Controller
  vcspeeddial.cpp

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

#include <QDomDocument>
#include <QDomElement>
#include <QLayout>
#include <QDebug>

#include "vcspeeddialproperties.h"
#include "vcpropertieseditor.h"
#include "vcspeeddial.h"
#include "speeddial.h"
#include "qlcmacros.h"
#include "qlcfile.h"

const quint8 VCSpeedDial::absoluteInputSourceId = 0;
const quint8 VCSpeedDial::tapInputSourceId = 1;
const QSize VCSpeedDial::defaultSize(QSize(200, 175));

/****************************************************************************
 * Initialization
 ****************************************************************************/

VCSpeedDial::VCSpeedDial(QWidget* parent, Doc* doc)
    : VCWidget(parent, doc)
    , m_speedTypes(VCSpeedDial::Duration)
    , m_dial(NULL)
    , m_absoluteValueMin(0)
    , m_absoluteValueMax(1000 * 10)
{
    new QVBoxLayout(this);

    m_dial = new SpeedDial(this);
    layout()->addWidget(m_dial);
    connect(m_dial, SIGNAL(valueChanged(int)), this, SLOT(slotDialValueChanged(int)));
    connect(m_dial, SIGNAL(tapped()), this, SLOT(slotDialTapped()));

    setType(VCWidget::SpeedDialWidget);
    setCaption(tr("Duration"));

    QSettings settings;
    QVariant var = settings.value(SETTINGS_SPEEDDIAL_SIZE);
    if (var.isValid() == true)
        resize(var.toSize());
    else
        resize(VCSpeedDial::defaultSize);

    var = settings.value(SETTINGS_SPEEDDIAL_VALUE);
    if (var.isValid() == true)
        m_dial->setValue(var.toUInt());

    slotModeChanged(doc->mode());
}

VCSpeedDial::~VCSpeedDial()
{
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

VCWidget* VCSpeedDial::createCopy(VCWidget* parent)
{
    Q_ASSERT(parent != NULL);

    VCSpeedDial* dial = new VCSpeedDial(parent, m_doc);
    if (dial->copyFrom(this) == false)
    {
        delete dial;
        dial = NULL;
    }

    return dial;
}

bool VCSpeedDial::copyFrom(const VCWidget* widget)
{
    const VCSpeedDial* dial = qobject_cast<const VCSpeedDial*> (widget);
    if (dial == NULL)
        return false;

    m_functions = dial->functions();
    m_speedTypes = dial->speedTypes();

    /* Copy common stuff */
    return VCWidget::copyFrom(widget);
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCSpeedDial::editProperties()
{
    VCSpeedDialProperties sdp(this, m_doc);
    sdp.exec();
}

/*****************************************************************************
 * Caption
 *****************************************************************************/

void VCSpeedDial::setCaption(const QString& text)
{
    VCWidget::setCaption(text);

    Q_ASSERT(m_dial != NULL);
    m_dial->setTitle(text);
}

/*****************************************************************************
 * QLC Mode
 *****************************************************************************/

void VCSpeedDial::slotModeChanged(Doc::Mode mode)
{
    if (mode == Doc::Operate)
    {
        m_dial->setEnabled(true);
        updateFeedback();
    }
    else
    {
        m_dial->setEnabled(false);
    }
}

/****************************************************************************
 * Speed type
 ****************************************************************************/

void VCSpeedDial::setSpeedTypes(VCSpeedDial::SpeedTypes types)
{
    m_speedTypes = types;
}

VCSpeedDial::SpeedTypes VCSpeedDial::speedTypes() const
{
    return m_speedTypes;
}

/****************************************************************************
 * Functions
 ****************************************************************************/

void VCSpeedDial::setFunctions(const QSet <quint32> ids)
{
    m_functions = ids;
}

QSet <quint32> VCSpeedDial::functions() const
{
    return m_functions;
}

void VCSpeedDial::tap()
{
    m_dial->tap();
}

void VCSpeedDial::slotDialValueChanged(int ms)
{
    foreach (quint32 id, m_functions)
    {
        Function* function = m_doc->function(id);
        if (function != NULL)
        {
            if (m_speedTypes & Duration)
                function->setDuration(ms);
            if (m_speedTypes & FadeIn)
                function->setFadeInSpeed(ms);
            if (m_speedTypes & FadeOut)
                function->setFadeOutSpeed(ms);
        }
    }
    updateFeedback();
}

void VCSpeedDial::slotDialTapped()
{
    foreach (quint32 id, m_functions)
    {
        Function* function = m_doc->function(id);
        if (function != NULL)
        {
            if (m_speedTypes & Duration)
                function->tap();
        }
    }
}

/*****************************************************************************
+ * External input
+ *****************************************************************************/

void VCSpeedDial::updateFeedback()
{
    int fbv = (int)SCALE(float(m_dial->value()), float(m_absoluteValueMin),
                     float(m_absoluteValueMax), float(0), float(UCHAR_MAX));

    sendFeedback(fbv, absoluteInputSourceId);
}

void VCSpeedDial::slotInputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    QLCInputSource src(universe, channel);
    if (src == inputSource(tapInputSourceId))
    {
        if (value != 0)
            m_dial->tap();
    }
    else if (src == inputSource(absoluteInputSourceId))
    {
        int ms = static_cast<int> (SCALE(qreal(value), qreal(0), qreal(255),
                                         qreal(absoluteValueMin()),
                                         qreal(absoluteValueMax())));
        m_dial->setValue(ms, true);
    }
}

/*********************************************************************
 * Tap key sequence handler
 *********************************************************************/

void VCSpeedDial::setKeySequence(const QKeySequence& keySequence)
{
    m_tapKeySequence = QKeySequence(keySequence);
}

QKeySequence VCSpeedDial::keySequence() const
{
    return m_tapKeySequence;
}

void VCSpeedDial::slotKeyPressed(const QKeySequence& keySequence)
{
    if (m_tapKeySequence == keySequence)
        m_dial->tap();
}


/****************************************************************************
 * Absolute value range
 ****************************************************************************/

void VCSpeedDial::setAbsoluteValueRange(uint min, uint max)
{
    m_absoluteValueMin = min;
    m_absoluteValueMax = max;
}

uint VCSpeedDial::absoluteValueMin() const
{
    return m_absoluteValueMin;
}

uint VCSpeedDial::absoluteValueMax() const
{
    return m_absoluteValueMax;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCSpeedDial::loadXML(const QDomElement* root)
{
    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCVCSpeedDial)
    {
        qWarning() << Q_FUNC_INFO << "SpeedDial node not found";
        return false;
    }

    /* Widget commons */
    loadXMLCommon(root);

    setSpeedTypes(VCSpeedDial::SpeedTypes(root->attribute(KXMLQLCVCSpeedDialSpeedTypes).toInt()));

    /* Children */
    QDomNode node = root->firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCVCSpeedDialFunction)
        {
            m_functions << tag.text().toUInt();
        }
        else if (tag.tagName() == KXMLQLCVCSpeedDialAbsoluteValue)
        {
            // Value range
            if (tag.hasAttribute(KXMLQLCVCSpeedDialAbsoluteValueMin) &&
                tag.hasAttribute(KXMLQLCVCSpeedDialAbsoluteValueMax))
            {
                uint min = tag.attribute(KXMLQLCVCSpeedDialAbsoluteValueMin).toUInt();
                uint max = tag.attribute(KXMLQLCVCSpeedDialAbsoluteValueMax).toUInt();
                setAbsoluteValueRange(min, max);
            }

            // Input
            QDomNode sub = node.firstChild();
            while (sub.isNull() == false)
            {
                QDomElement subtag = sub.toElement();
                if (subtag.tagName() == KXMLQLCVCWidgetInput)
                {
                    quint32 uni = QLCInputSource::invalidUniverse;
                    quint32 ch = QLCInputSource::invalidChannel;
                    if (loadXMLInput(subtag, &uni, &ch) == true)
                        setInputSource(QLCInputSource(uni, ch), absoluteInputSourceId);
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Unknown absolute value tag:" << tag.tagName();
                }

                sub = sub.nextSibling();
            }
        }
        else if (tag.tagName() == KXMLQLCVCSpeedDialTap)
        {
            // Input
            QDomNode sub = node.firstChild();
            while (sub.isNull() == false)
            {
                QDomElement subtag = sub.toElement();
                if (subtag.tagName() == KXMLQLCVCWidgetInput)
                {
                    quint32 uni = QLCInputSource::invalidUniverse;
                    quint32 ch = QLCInputSource::invalidChannel;
                    if (loadXMLInput(subtag, &uni, &ch) == true)
                        setInputSource(QLCInputSource(uni, ch), tapInputSourceId);
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Unknown tap tag:" << tag.tagName();
                }

                sub = sub.nextSibling();
            }
        }
        else if (tag.tagName() == KXMLQLCVCSpeedDialTapKey)
        {
            setKeySequence(stripKeySequence(QKeySequence(tag.text())));
        }
        else if (tag.tagName() == KXMLQLCWindowState)
        {
            int x = 0, y = 0, w = 0, h = 0;
            bool visible = true;
            loadXMLWindowState(&tag, &x, &y, &w, &h, &visible);
            setGeometry(x, y, w, h);
        }
        else if (tag.tagName() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(&tag);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown speed dial tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool VCSpeedDial::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(vc_root != NULL);

    QDomElement root = doc->createElement(KXMLQLCVCSpeedDial);
    vc_root->appendChild(root);

    saveXMLCommon(doc, &root);

    /* Speed Type */
    root.setAttribute(KXMLQLCVCSpeedDialSpeedTypes, speedTypes());

    /* Window state */
    saveXMLWindowState(doc, &root);

    /* Appearance */
    saveXMLAppearance(doc, &root);

    /* Absolute input */
    QDomElement absInput = doc->createElement(KXMLQLCVCSpeedDialAbsoluteValue);
    absInput.setAttribute(KXMLQLCVCSpeedDialAbsoluteValueMin, absoluteValueMin());
    absInput.setAttribute(KXMLQLCVCSpeedDialAbsoluteValueMax, absoluteValueMax());
    saveXMLInput(doc, &absInput, inputSource(absoluteInputSourceId));
    root.appendChild(absInput);

    /* Tap input */
    QDomElement tap = doc->createElement(KXMLQLCVCSpeedDialTap);
    saveXMLInput(doc, &tap, inputSource(tapInputSourceId));
    root.appendChild(tap);

    /* Key sequence */
    if (m_tapKeySequence.isEmpty() == false)
    {
        QDomElement tag = doc->createElement(KXMLQLCVCSpeedDialTapKey);
        root.appendChild(tag);
        QDomText text = doc->createTextNode(m_tapKeySequence.toString());
        tag.appendChild(text);
    }

    /* Functions */
    foreach (quint32 id, m_functions)
    {
        QDomElement function = doc->createElement(KXMLQLCVCSpeedDialFunction);
        QDomText functionText = doc->createTextNode(QString::number(id));
        function.appendChild(functionText);
        root.appendChild(function);
    }

    return true;
}

void VCSpeedDial::postLoad()
{
    /* Remove such function IDs that don't exist */
    QMutableSetIterator <quint32> it(m_functions);
    while (it.hasNext() == true)
    {
        it.next();
        Function* function = m_doc->function(it.value());
        if (function == NULL)
            it.remove();
    }
}
