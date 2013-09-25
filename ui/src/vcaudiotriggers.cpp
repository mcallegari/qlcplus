/*
  Q Light Controller Plus
  vcaudiotriggers.cpp

  Copyright (c) Massimo Callegari

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QtXml>

#if defined(__APPLE__)
  #include "audiocapture_portaudio.h"
#elif defined(WIN32)
  #include "audiocapture_wavein.h"
#else
  #include "audiocapture_alsa.h"
#endif

#include "audiotriggersconfiguration.h"
#include "vcpropertieseditor.h"
#include "vcaudiotriggers.h"
#include "virtualconsole.h"
#include "audiobar.h"
#include "apputil.h"
#include "doc.h"

#define KXMLQLCATBarsNumber "BarsNumber"

#define KXMLQLCVolumeBar    "VolumeBar"
#define KXMLQLCSpectrumBar  "SpectrumBar"

const QSize VCAudioTriggers::defaultSize(QSize(300, 200));

VCAudioTriggers::VCAudioTriggers(QWidget* parent, Doc* doc)
    : VCWidget(parent, doc)
    , m_hbox(NULL)
    , m_button(NULL)
    , m_label(NULL)
    , m_spectrum(NULL)
    , m_inputCapture(NULL)
{
    /* Set the class name "VCAudioTriggers" as the object name as well */
    setObjectName(VCAudioTriggers::staticMetaObject.className());

    setType(VCWidget::AudioTriggersWidget);
    setFrameStyle(KVCFrameStyleSunken);

    QVBoxLayout *vbox = new QVBoxLayout(this);
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

    // create the  AudioBar spectrum data. To be loaded
    // from the project
    m_volumeBar = new AudioBar(AudioBar::None, 0);
    for (int i = 0; i < m_inputCapture->bandsNumber(); i++)
    {
        AudioBar *asb = new AudioBar(AudioBar::None, 0);
        m_spectrumBars.append(asb);
    }

    m_spectrum = new AudioTriggerWidget(this);
    m_spectrum->setBarsNumber(m_inputCapture->bandsNumber());
    m_spectrum->setMaxFrequency(AudioCapture::maxFrequency());
    m_spectrum->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    vbox->addWidget(m_spectrum);

    /* Initial size */
    QSettings settings;
    QVariant var = settings.value(SETTINGS_AUDIOTRIGGERS_SIZE);
    if (var.isValid() == true)
        resize(var.toSize());
    else
        resize(defaultSize);
}

VCAudioTriggers::~VCAudioTriggers()
{
}

void VCAudioTriggers::enableCapture(bool enable)
{
    if (enable == true)
    {
        if (m_inputCapture->isRunning())
            return;

        m_inputCapture->setBandsNumber(m_spectrum->barsNumber());
        if (m_inputCapture->isInitialized() == false)
            m_inputCapture->initialize(44100, 1, 2048);
        m_inputCapture->start();
        m_button->setChecked(true);
        connect(m_inputCapture, SIGNAL(dataProcessed(double *, double, quint32)),
                this, SLOT(slotDisplaySpectrum(double *, double, quint32)));
    }
    else
    {
        m_inputCapture->stop();
        m_button->setChecked(false);
        disconnect(m_inputCapture, SIGNAL(dataProcessed(double *, double, quint32)),
                this, SLOT(slotDisplaySpectrum(double *, double, quint32)));
    }
}

void VCAudioTriggers::slotEnableButtonToggled(bool toggle)
{
    if (toggle == true)
    {
        emit enableRequest(this->id());
    }
    else
    {
        m_inputCapture->stop();
    }
}

void VCAudioTriggers::slotDisplaySpectrum(double *spectrumBands, double maxMagnitude, quint32 power)
{
    m_spectrum->displaySpectrum(spectrumBands, maxMagnitude, power);
    m_volumeBar->m_value = m_spectrum->getUcharVolume();

    if (m_doc->mode() == Doc::Design)
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

void VCAudioTriggers::writeDMX(MasterTimer *timer, UniverseArray *universes)
{
    Q_UNUSED(timer);

    if (m_doc->mode() == Doc::Design)
        return;

    if (m_volumeBar->m_type == AudioBar::DMXBar)
    {
        for(int i = 0; i < m_volumeBar->m_absDmxChannels.count(); i++)
            universes->write(m_volumeBar->m_absDmxChannels.at(i), m_volumeBar->m_value, QLCChannel::Intensity);
    }
    foreach(AudioBar *sb, m_spectrumBars)
    {
        if (sb->m_type == AudioBar::DMXBar)
        {
            for(int i = 0; i < sb->m_absDmxChannels.count(); i++)
                universes->write(sb->m_absDmxChannels.at(i), sb->m_value, QLCChannel::Intensity);
        }
    }
}

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

bool VCAudioTriggers::copyFrom(VCWidget *widget)
{
    VCAudioTriggers* triggers = qobject_cast <VCAudioTriggers*> (widget);
    if (triggers == NULL)
        return false;

    /* Copy triggers-specific stuff */

    /* Copy common stuff */
    return VCWidget::copyFrom(widget);
}

void VCAudioTriggers::setCaption(const QString &text)
{
    if (m_label != NULL)
        m_label->setText(text);

    VCWidget::setCaption(text);
}

void VCAudioTriggers::slotModeChanged(Doc::Mode mode)
{
    if (mode == Doc::Operate)
    {
        if (m_button)
            m_button->setEnabled(true);

        foreach(AudioBar *bar, getAudioBars())
        {
            if (bar->m_type == AudioBar::DMXBar)
            {
                m_doc->masterTimer()->registerDMXSource(this);
                break;
            }
        }
    }
    else
    {
        if (m_button)
            m_button->setEnabled(false);
        m_doc->masterTimer()->unregisterDMXSource(this);
    }
    VCWidget::slotModeChanged(mode);
}

/*************************************************************************
 * Configuration
 *************************************************************************/

AudioBar *VCAudioTriggers::getSpectrumBar(int index)
{
    if (index == 1000)
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
        for (int i = 0 ; i < num - m_spectrumBars.count(); i++)
        {
            AudioBar *asb = new AudioBar(AudioBar::None, 0);
            m_spectrumBars.append(asb);
        }
    }
    else if (num < m_spectrumBars.count())
    {
        for (int i = 0 ; i < m_spectrumBars.count() - num; i++)
            m_spectrumBars.takeLast();
    }
    if (m_inputCapture != NULL)
        m_inputCapture->setBandsNumber(num);
}

void VCAudioTriggers::setSpectrumBarType(int index, int type)
{
    if (index == 1000)
    {
        m_volumeBar->m_type = type;
        return;
    }
    if (index >= 0 && index < m_spectrumBars.size())
        m_spectrumBars[index]->m_type = type;
}


void VCAudioTriggers::editProperties()
{
    // make a backup copy of the current bars
    AudioBar *tmpVolume = m_volumeBar->createCopy();
    QList <AudioBar *> tmpSpectrumBars;
    foreach(AudioBar *bar, m_spectrumBars)
        tmpSpectrumBars.append(bar->createCopy());

    AudioTriggersConfiguration atc(this, m_doc, m_inputCapture);
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
    m_inputCapture->setBandsNumber(m_spectrumBars.count());
}

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
        else if (tag.tagName() == KXMLQLCVolumeBar)
        {
            m_volumeBar->loadXML(tag);
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
                    VCWidget *widget = VirtualConsole::instance()->widget(wid);
                    if (widget != NULL)
                        m_volumeBar->m_widget = widget;
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
                    m_spectrumBars[idx]->loadXML(tag);
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
                            VCWidget *widget = VirtualConsole::instance()->widget(wid);
                            if (widget != NULL)
                                m_spectrumBars[idx]->m_widget = widget;
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
    //QDomElement tag;
    //QDomText text;
    //QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(vc_root != NULL);

    /* VC button entry */
    root = doc->createElement(KXMLQLCVCAudioTriggers);
    root.setAttribute(KXMLQLCATBarsNumber, m_spectrumBars.count());
    vc_root->appendChild(root);

    saveXMLCommon(doc, &root);

    /* Window state */
    saveXMLWindowState(doc, &root);

    /* Appearance */
    saveXMLAppearance(doc, &root);

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
        m_volumeBar->saveXML(doc, &root, KXMLQLCVolumeBar, 1000);
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




