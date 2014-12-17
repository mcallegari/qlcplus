/*
  Q Light Controller Plus
  vcaudiotriggers.cpp

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

#include <QtXml>
#include <QMessageBox>

#include "vcaudiotriggersproperties.h"
#include "vcpropertieseditor.h"
#include "vcaudiotriggers.h"
#include "virtualconsole.h"
#include "audiocapture.h"
#include "universe.h"
#include "audiobar.h"
#include "apputil.h"
#include "doc.h"

#define KXMLQLCVCATKey "Key"
#define KXMLQLCVCATBarsNumber "BarsNumber"

#define KXMLQLCVolumeBar    "VolumeBar"
#define KXMLQLCSpectrumBar  "SpectrumBar"

const QSize VCAudioTriggers::defaultSize(QSize(300, 200));

VCAudioTriggers::VCAudioTriggers(QWidget* parent, Doc* doc)
    : VCWidget(parent, doc)
    , m_hbox(NULL)
    , m_button(NULL)
    , m_label(NULL)
    , m_spectrum(NULL)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    , m_volumeSlider(NULL)
#endif
    , m_inputCapture(NULL)
{
    /* Set the class name "VCAudioTriggers" as the object name as well */
    setObjectName(VCAudioTriggers::staticMetaObject.className());

    setType(VCWidget::AudioTriggersWidget);
    setFrameStyle(KVCFrameStyleSunken);

    new QVBoxLayout(this);

    /* Main HBox */
    m_hbox = new QHBoxLayout();
    m_hbox->setGeometry(QRect(0, 0, 300, 40));

    layout()->setSpacing(2);
    layout()->setContentsMargins(4, 4, 4, 4);
    layout()->addItem(m_hbox);

    m_button = new QToolButton(this);
    m_button->setStyle(AppUtil::saneStyle());
    m_button->setIconSize(QSize(32, 32));
    m_button->setMinimumSize(QSize(32, 32));
    m_button->setMaximumSize(QSize(32, 32));
    m_button->setIcon(QIcon(":/check.png"));
    m_button->setCheckable(true);
    QString btnSS = "QToolButton { background-color: #E0DFDF; border: 1px solid gray; border-radius: 3px; padding: 3px; } ";
    btnSS += "QToolButton:checked { background-color: #D7DE75; border: 1px solid gray; border-radius: 3px; padding: 3px; } ";
    m_button->setStyleSheet(btnSS);
    m_button->setEnabled(false);

    m_hbox->addWidget(m_button);
    connect(m_button, SIGNAL(toggled(bool)), this, SLOT(slotEnableButtonToggled(bool)));

    m_label = new QLabel(this);
    m_label->setText(this->caption());
    QString txtColor = "white";
    if (m_hasCustomForegroundColor)
        txtColor = this->foregroundColor().name();
    m_label->setStyleSheet("QLabel { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #345D27, stop: 1 #0E1A0A); "
                           "color: " + txtColor + "; border-radius: 3px; padding: 3px; margin-left: 2px; }");

    if (m_hasCustomFont)
        m_label->setFont(font());
    else
    {
        QFont m_font = QApplication::font();
        m_font.setBold(true);
        m_font.setPixelSize(12);
        m_label->setFont(m_font);
    }
    m_hbox->addWidget(m_label);

    m_inputCapture = m_doc->audioInputCapture();

    // create the  AudioBar items to hold the spectrum data.
    // To be loaded from the project
    m_volumeBar = new AudioBar(AudioBar::None, 0);
    for (int i = 0; i < m_inputCapture->defaultBarsNumber(); i++)
    {
        AudioBar *asb = new AudioBar(AudioBar::None, 0);
        m_spectrumBars.append(asb);
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QHBoxLayout *hbox2 = new QHBoxLayout();
    m_volumeSlider = new ClickAndGoSlider();
    m_volumeSlider->setOrientation(Qt::Vertical);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setStyleSheet(CNG_DEFAULT_STYLE);
    m_volumeSlider->setValue(100);
    m_volumeSlider->setFixedWidth(32);
    m_volumeSlider->setEnabled(false);
    connect(m_volumeSlider, SIGNAL(valueChanged(int)),
            this, SLOT(slotVolumeChanged(int)));
#endif
    m_spectrum = new AudioTriggerWidget(this);
    m_spectrum->setBarsNumber(m_inputCapture->defaultBarsNumber());
    m_spectrum->setMaxFrequency(AudioCapture::maxFrequency());
    m_spectrum->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    layout()->addWidget(m_spectrum);
#else
    layout()->addItem(hbox2);
    hbox2->addWidget(m_spectrum);
    hbox2->addWidget(m_volumeSlider);
#endif
    /* Initial size */
    QSettings settings;
    QVariant var = settings.value(SETTINGS_AUDIOTRIGGERS_SIZE);
    if (var.isValid() == true)
        resize(var.toSize());
    else
        resize(defaultSize);

    slotModeChanged(m_doc->mode());
}

VCAudioTriggers::~VCAudioTriggers()
{
    if (m_inputCapture)
    {
        if (m_inputCapture->isRunning())
            m_inputCapture->stop();

        disconnect(m_inputCapture, SIGNAL(dataProcessed(double*,int,double,quint32)),
                this, SLOT(slotDisplaySpectrum(double*,int,double,quint32)));
        m_inputCapture->unregisterBandsNumber(m_spectrum->barsNumber());
    }
}

void VCAudioTriggers::enableWidgetUI(bool enable)
{
    if (m_button)
        m_button->setEnabled(enable);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    m_volumeSlider->setEnabled(enable);
#endif
}

void VCAudioTriggers::notifyFunctionStarting(quint32 fid)
{
    // Stop on any other function started
    Q_UNUSED(fid);
    if (m_button->isChecked() == true)
        enableCapture(false);
}

void VCAudioTriggers::enableCapture(bool enable)
{
    if (enable == true)
    {
        // in case the audio input device has been changed in the meantime...
        m_inputCapture = m_doc->audioInputCapture();

        if (m_inputCapture->isInitialized() == false)
        {
            if (m_inputCapture->initialize(44100, 1, 2048) == false)
            {
                QMessageBox::warning(this, tr("Audio open error"),
                                     tr("An error occurred while initializing the selected audio device. Please review your audio input settings."));
                m_button->setChecked(false);
                return;
            }
        }
        m_inputCapture->registerBandsNumber(m_spectrum->barsNumber());
        // Invalid ID: Stop every other widget
        emit functionStarting(Function::invalidId());
        connect(m_inputCapture, SIGNAL(dataProcessed(double*,int,double,quint32)),
                this, SLOT(slotDisplaySpectrum(double*,int,double,quint32)));
        if (m_inputCapture->isRunning() == false)
            m_inputCapture->start();
        m_button->blockSignals(true);
        m_button->setChecked(true);
        m_button->blockSignals(false);
    }
    else
    {
        // in case the audio input device has been changed in the meantime...
        m_inputCapture = m_doc->audioInputCapture();

        if (m_inputCapture->isRunning())
        {
            m_inputCapture->unregisterBandsNumber(m_spectrum->barsNumber());
            m_inputCapture->stop();
        }

        m_button->blockSignals(true);
        m_button->setChecked(false);
        m_button->blockSignals(false);
        disconnect(m_inputCapture, SIGNAL(dataProcessed(double*,int,double,quint32)),
                this, SLOT(slotDisplaySpectrum(double*,int,double,quint32)));
    }
}

void VCAudioTriggers::slotEnableButtonToggled(bool toggle)
{
    if (mode() == Doc::Design)
        return;

    enableCapture(toggle);
}

void VCAudioTriggers::slotDisplaySpectrum(double *spectrumBands, int size,
                                          double maxMagnitude, quint32 power)
{
    qDebug() << "Display spectrum ----- bars:" << size;
    if (size != m_spectrum->barsNumber())
        return;

    m_spectrum->displaySpectrum(spectrumBands, maxMagnitude, power);
    m_volumeBar->m_value = m_spectrum->getUcharVolume();

    if (mode() == Doc::Design)
        return;

    if (m_volumeBar->m_type == AudioBar::FunctionBar)
        m_volumeBar->checkFunctionThresholds(m_doc);
    else if (m_volumeBar->m_type == AudioBar::VCWidgetBar)
        m_volumeBar->checkWidgetFunctionality();

    for (int i = 0; i < m_spectrumBars.count(); i++)
    {
        m_spectrumBars[i]->m_value = m_spectrum->getUcharBand(i);
        if (m_spectrumBars[i]->m_type == AudioBar::FunctionBar)
            m_spectrumBars[i]->checkFunctionThresholds(m_doc);
        else if (m_spectrumBars[i]->m_type == AudioBar::VCWidgetBar)
            m_spectrumBars[i]->checkWidgetFunctionality();
    }
}

#if QT_VERSION >= 0x050000
void VCAudioTriggers::slotVolumeChanged(int volume)
{
    if (m_inputCapture != NULL)
        m_inputCapture->setVolume((qreal)volume / 100);
}
#endif

/*********************************************************************
 * DMXSource
 *********************************************************************/

void VCAudioTriggers::writeDMX(MasterTimer *timer, QList<Universe *> universes)
{
    Q_UNUSED(timer);

    if (mode() == Doc::Design)
        return;

    if (m_volumeBar->m_type == AudioBar::DMXBar)
    {
        for(int i = 0; i < m_volumeBar->m_absDmxChannels.count(); i++)
        {
            quint32 address = m_volumeBar->m_absDmxChannels.at(i) & 0x01FF;
            int uni = m_volumeBar->m_absDmxChannels.at(i) >> 9;
            if (uni < universes.count())
                universes[uni]->write(address, m_volumeBar->m_value);
        }
    }
    foreach(AudioBar *sb, m_spectrumBars)
    {
        if (sb->m_type == AudioBar::DMXBar)
        {
            for(int i = 0; i < sb->m_absDmxChannels.count(); i++)
            {
                quint32 address = sb->m_absDmxChannels.at(i) & 0x01FF;
                int uni = sb->m_absDmxChannels.at(i) >> 9;
                if (uni < universes.count())
                    universes[uni]->write(address, sb->m_value);
            }
        }
    }
}

/*********************************************************************
 * Key sequence handler
 *********************************************************************/

void VCAudioTriggers::setKeySequence(const QKeySequence& keySequence)
{
    m_keySequence = QKeySequence(keySequence);
}

QKeySequence VCAudioTriggers::keySequence() const
{
    return m_keySequence;
}

void VCAudioTriggers::slotKeyPressed(const QKeySequence& keySequence)
{
    if (isEnabled() == false)
        return;

    if (m_keySequence == keySequence)
    {
        if (m_inputCapture->isRunning())
            slotEnableButtonToggled(false);
        else
            slotEnableButtonToggled(true);
    }
}

void VCAudioTriggers::slotInputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    if (isEnabled() == false)
        return;

    if (checkInputSource(universe, (page() << 16) | channel, value, sender()))
    {
        if (m_inputCapture->isRunning() == false && value > 0)
            slotEnableButtonToggled(true);
        else
            slotEnableButtonToggled(false);
    }
}

/*********************************************************************
 * Clipboard
 *********************************************************************/

VCWidget *VCAudioTriggers::createCopy(VCWidget *parent)
{
    Q_ASSERT(parent != NULL);

    VCAudioTriggers* triggers = new VCAudioTriggers(parent, m_doc);
    if (triggers->copyFrom(this) == false)
    {
        delete triggers;
        triggers = NULL;
    }

    return triggers;
}

bool VCAudioTriggers::copyFrom(const VCWidget *widget)
{
    const VCAudioTriggers* triggers = qobject_cast <const VCAudioTriggers*> (widget);
    if (triggers == NULL)
        return false;

    /* TODO: Copy triggers-specific stuff */

    /* Copy common stuff */
    return VCWidget::copyFrom(widget);
}


/*************************************************************************
 * VCWidget-inherited
 *************************************************************************/

void VCAudioTriggers::setCaption(const QString &text)
{
    if (m_label != NULL)
        m_label->setText(text);

    VCWidget::setCaption(text);
}

void VCAudioTriggers::setForegroundColor(const QColor &color)
{
    if (m_label != NULL)
    {
        m_label->setStyleSheet("QLabel { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #345D27, stop: 1 #0E1A0A); "
                               "color: " + color.name() + "; border-radius: 3px; padding: 3px; margin-left: 2px; }");
        m_hasCustomForegroundColor = true;
        m_doc->setModified();
    }
}

QColor VCAudioTriggers::foregroundColor() const
{
    if (m_label != NULL)
        return m_label->palette().color(m_label->foregroundRole());
    else
        return VCWidget::foregroundColor();
}

void VCAudioTriggers::slotModeChanged(Doc::Mode mode)
{
    if (mode == Doc::Operate)
    {
        enableWidgetUI(true);

        foreach(AudioBar *bar, getAudioBars())
        {
            if (bar->m_type == AudioBar::DMXBar)
            {
                m_doc->masterTimer()->registerDMXSource(this, "AudioTriggers");
                break;
            }
        }
    }
    else
    {
        enableWidgetUI(false);
        enableCapture(false);
        m_doc->masterTimer()->unregisterDMXSource(this);
    }
    VCWidget::slotModeChanged(mode);
}

/*************************************************************************
 * Configuration
 *************************************************************************/

AudioBar *VCAudioTriggers::getSpectrumBar(int index)
{
    if (index == volumeBarIndex())
        return m_volumeBar;
    if (index >= 0 && index < m_spectrumBars.size())
        return m_spectrumBars.at(index);

    return NULL;
}

QList<AudioBar *> VCAudioTriggers::getAudioBars()
{
    QList <AudioBar *> list;
    list.append(m_volumeBar);
    list.append(m_spectrumBars);

    return list;
}

void VCAudioTriggers::setSpectrumBarsNumber(int num)
{
    if (num > m_spectrumBars.count())
    {
        int barsToAdd = num - m_spectrumBars.count();
        for (int i = 0 ; i < barsToAdd; i++)
        {
            AudioBar *asb = new AudioBar(AudioBar::None, 0);
            m_spectrumBars.append(asb);
        }
    }
    else if (num < m_spectrumBars.count())
    {
        int barsToRemove = m_spectrumBars.count() - num;
        for (int i = 0 ; i < barsToRemove; i++)
            m_spectrumBars.removeLast();
    }

    if (m_spectrum != NULL)
        m_spectrum->setBarsNumber(num);
}

void VCAudioTriggers::setSpectrumBarType(int index, int type)
{
    if (index == volumeBarIndex())
    {
        m_volumeBar->setType(type);
        return;
    }
    if (index >= 0 && index < m_spectrumBars.size())
    {
        m_spectrumBars[index]->setType(type);
    }
}


void VCAudioTriggers::editProperties()
{
    // make a backup copy of the current bars
    AudioBar *tmpVolume = m_volumeBar->createCopy();
    QList <AudioBar *> tmpSpectrumBars;
    foreach(AudioBar *bar, m_spectrumBars)
        tmpSpectrumBars.append(bar->createCopy());
    int barsNumber = m_spectrumBars.count();

    AudioTriggersConfiguration atc(this, m_doc, barsNumber,
                                   m_inputCapture->maxFrequency());
    if (atc.exec() == QDialog::Rejected)
    {
        // restore the previous bars backup
        delete m_volumeBar;
        m_volumeBar = tmpVolume;
        m_spectrumBars.clear();
        foreach(AudioBar *bar, tmpSpectrumBars)
            m_spectrumBars.append(bar);
    }
    m_spectrum->setBarsNumber(m_spectrumBars.count());
    if (barsNumber != m_spectrumBars.count())
    {
        m_inputCapture->unregisterBandsNumber(barsNumber);
        m_inputCapture->registerBandsNumber(m_spectrumBars.count());
    }
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool VCAudioTriggers::loadXML(const QDomElement *root)
{
    QDomNode node;
    QDomElement tag;

    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCVCAudioTriggers)
    {
        qWarning() << Q_FUNC_INFO << "Audio Triggers node not found";
        return false;
    }
    if (root->hasAttribute(KXMLQLCVCATBarsNumber))
    {
        int barsNum = root->attribute(KXMLQLCVCATBarsNumber).toInt();
        setSpectrumBarsNumber(barsNum);
    }

    /* Widget commons */
    loadXMLCommon(root);

    /* Children */
    node = root->firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();
        if (tag.tagName() == KXMLQLCWindowState)
        {
            int x = 0, y = 0, w = 0, h = 0;
            bool visible = false;
            loadXMLWindowState(&tag, &x, &y, &w, &h, &visible);
            setGeometry(x, y, w, h);
        }
        else if (tag.tagName() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(&tag);
        }
        else if (tag.tagName() == KXMLQLCVCWidgetInput)
        {
            loadXMLInput(&tag);
        }
        else if (tag.tagName() == KXMLQLCVCATKey)
        {
            setKeySequence(stripKeySequence(QKeySequence(tag.text())));
        }
        else if (tag.tagName() == KXMLQLCVolumeBar)
        {
            m_volumeBar->loadXML(tag, m_doc);
            if (m_volumeBar->m_type == AudioBar::FunctionBar)
            {
                if (tag.hasAttribute(KXMLQLCAudioBarFunction))
                {
                    quint32 fid = tag.attribute(KXMLQLCAudioBarFunction).toUInt();
                    Function *func = m_doc->function(fid);
                    if (func != NULL)
                        m_volumeBar->m_function = func;
                }
            }
            else if (m_volumeBar->m_type == AudioBar::VCWidgetBar)
            {
                if (tag.hasAttribute(KXMLQLCAudioBarWidget))
                {
                    quint32 wid = tag.attribute(KXMLQLCAudioBarWidget).toUInt();
                    m_volumeBar->m_widgetID = wid;
                }
            }
        }
        else if (tag.tagName() == KXMLQLCSpectrumBar)
        {
            if (tag.hasAttribute(KXMLQLCAudioBarIndex))
            {
                int idx = tag.attribute(KXMLQLCAudioBarIndex).toInt();
                if (idx >= 0 && idx < m_spectrumBars.count())
                {
                    m_spectrumBars[idx]->loadXML(tag, m_doc);
                    if (m_spectrumBars[idx]->m_type == AudioBar::FunctionBar)
                    {
                        if (tag.hasAttribute(KXMLQLCAudioBarFunction))
                        {
                            quint32 fid = tag.attribute(KXMLQLCAudioBarFunction).toUInt();
                            Function *func = m_doc->function(fid);
                            if (func != NULL)
                                m_spectrumBars[idx]->m_function = func;
                        }
                    }
                    else if (m_spectrumBars[idx]->m_type == AudioBar::VCWidgetBar)
                    {
                        if (tag.hasAttribute(KXMLQLCAudioBarWidget))
                        {
                            quint32 wid = tag.attribute(KXMLQLCAudioBarWidget).toUInt();
                            m_spectrumBars[idx]->m_widgetID = wid;
                        }
                    }
                }
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown audio triggers tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool VCAudioTriggers::saveXML(QDomDocument *doc, QDomElement *vc_root)
{
    QDomElement root;
    QDomElement tag;
    QDomText text;
    //QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(vc_root != NULL);

    /* VC button entry */
    root = doc->createElement(KXMLQLCVCAudioTriggers);
    root.setAttribute(KXMLQLCVCATBarsNumber, m_spectrumBars.count());
    vc_root->appendChild(root);

    saveXMLCommon(doc, &root);

    /* Window state */
    saveXMLWindowState(doc, &root);

    /* Appearance */
    saveXMLAppearance(doc, &root);

    /* Key sequence */
    if (m_keySequence.isEmpty() == false)
    {
        tag = doc->createElement(KXMLQLCVCATKey);
        root.appendChild(tag);
        text = doc->createTextNode(m_keySequence.toString());
        tag.appendChild(text);
    }

    /* External input */
    saveXMLInput(doc, &root);

    /* Lookup for any assigned bar */
    bool hasAssignment = false;
    if (m_volumeBar->m_type != AudioBar::None)
        hasAssignment = true;
    else
    {
        foreach(AudioBar *bar, m_spectrumBars)
        {
            if (bar->m_type != AudioBar::None)
            {
                hasAssignment = true;
                break;
            }
        }
    }

    if (hasAssignment == false)
        return false;

    if (m_volumeBar->m_type != AudioBar::None)
    {
        m_volumeBar->saveXML(doc, &root, KXMLQLCVolumeBar, volumeBarIndex());
    }
    int idx = 0;
    foreach (AudioBar *bar, m_spectrumBars)
    {
        if (bar->m_type != AudioBar::None)
            bar->saveXML(doc, &root, KXMLQLCSpectrumBar, idx);
        idx++;
    }

    return true;
}




