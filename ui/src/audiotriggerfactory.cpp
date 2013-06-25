/*
  Q Light Controller Plus
  audiotriggerfactory.cpp

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

#include "vcbutton.h"
#include "vcslider.h"
#include "scenevalue.h"
#include "virtualconsole.h"
#include "audiotriggerfactory.h"
#include "audiotriggersconfiguration.h"

#if defined(__APPLE__)
  #include "audiocapture_portaudio.h"
#elif defined(WIN32)
  #include "audiocapture_wavein.h"
#else
  #include "audiocapture_alsa.h"
#endif

#include <QDomElement>

#define KXMLQLCATFBarsNumber   "BarsNumber"

#define KXMLQLCVolumeBar "VolumeBar"
#define KXMLQLCSpectrumBar "SpectrumBar"

#define KXMLQLCAudioBarIndex "Index"
#define KXMLQLCAudioBarName "Name"
#define KXMLQLCAudioBarType "Type"
#define KXMLQLCAudioBarDMXChannels "DMXChannels"
#define KXMLQLCAudioBarFunction "FunctionID"
#define KXMLQLCAudioBarWidget "WidgetID"

AudioTriggerFactory* AudioTriggerFactory::s_instance = NULL;

AudioTriggerFactory::AudioTriggerFactory(Doc *doc, QWidget *parent)
    : QDialog(parent)
    , m_doc(doc)
{
    Q_ASSERT(s_instance == NULL);
    s_instance = this;

    setupUi(this);

    m_spectrum = new AudioTriggerWidget(this);
#if defined(__APPLE__)
    m_inputCapture = new AudioCapturePortAudio();
#elif defined(WIN32)
    m_inputCapture = new AudioCaptureWaveIn();
#else
    m_inputCapture = new AudioCaptureAlsa();
#endif

    // create the  AudioBar spectrum data. To be loaded
    // from the project
    m_volumeBar = new AudioBar(AudioBar::None, 0);
    for (int i = 0; i < m_inputCapture->bandsNumber(); i++)
    {
        AudioBar *asb = new AudioBar(AudioBar::None, 0);
        m_spectrumBars.append(asb);
    }

    m_spectrum->setBarsNumber(m_inputCapture->bandsNumber());
    m_spectrum->setMaxFrequency(AudioCapture::maxFrequency());

    m_gridLayout->addWidget(m_spectrum);

    connect(m_enableBox, SIGNAL(toggled(bool)),
            this, SLOT(slotEnableCapture(bool)));
    connect(m_configButton, SIGNAL(clicked()),
            this, SLOT(slotConfiguration()));
}

AudioTriggerFactory::~AudioTriggerFactory()
{
    m_inputCapture->stop();
    m_doc->masterTimer()->unregisterDMXSource(this);
    delete m_inputCapture;
}

AudioTriggerFactory *AudioTriggerFactory::instance()
{
    return s_instance;
}

AudioBar *AudioTriggerFactory::getSpectrumBar(int index)
{
    if (index == 1000)
        return m_volumeBar;
    if (index >= 0 && index < m_spectrumBars.size())
        return m_spectrumBars.at(index);

    return NULL;
}

void AudioTriggerFactory::setSpectrumBarsNumber(int num)
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

void AudioTriggerFactory::setSpectrumBarType(int index, int type)
{
    if (index == 1000)
    {
        m_volumeBar->m_type = type;
        return;
    }
    if (index >= 0 && index < m_spectrumBars.size())
        m_spectrumBars[index]->m_type = type;
}

void AudioTriggerFactory::slotEnableCapture(bool enable)
{
    if (enable == true)
    {
        connect(m_inputCapture, SIGNAL(dataProcessed(double *, double, quint32)),
                this, SLOT(slotDisplaySpectrum(double *, double, quint32)));
        m_inputCapture->initialize(44100, 1, 2048);
        m_inputCapture->start();
    }
    else
    {
        m_inputCapture->stop();
    }
    m_configButton->setEnabled(enable);
}

void AudioTriggerFactory::slotDisplaySpectrum(double *spectrumBands, double maxMagnitude, quint32 power)
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

void AudioTriggerFactory::slotConfiguration()
{
    m_inputCapture->stop();
    m_doc->masterTimer()->unregisterDMXSource(this);

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
    if (m_volumeBar->m_type == AudioBar::DMXBar)
        m_doc->masterTimer()->registerDMXSource(this);
    else
    {
        foreach(AudioBar *bar, m_spectrumBars)
        {
            if (bar->m_type == AudioBar::DMXBar)
                m_doc->masterTimer()->registerDMXSource(this);
        }
    }
    m_inputCapture->start();
}

/*********************************************************************
 * DMXSource
 *********************************************************************/

void AudioTriggerFactory::writeDMX(MasterTimer *timer, UniverseArray *universes)
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

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool AudioTriggerFactory::loadXML(const QDomElement &root)
{
    qDebug() << Q_FUNC_INFO;

    if (root.tagName() != KXMLQLCAudioTriggerFactory)
    {
        qWarning() << Q_FUNC_INFO << "Audio Trigger Factory node not found";
        return false;
    }

    if (root.hasAttribute(KXMLQLCATFBarsNumber))
    {
        int barsNumber = root.attribute(KXMLQLCATFBarsNumber).toInt();
        setSpectrumBarsNumber(barsNumber);
    }

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCVolumeBar)
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
            qWarning() << Q_FUNC_INFO << "Unrecognized Audio Triggers Factory node:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool AudioTriggerFactory::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    qDebug() << Q_FUNC_INFO;

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

    qDebug() << "hasAssignment";

    /* Audio Trigger Factory entry */
    QDomElement atf_root = doc->createElement(KXMLQLCAudioTriggerFactory);
    atf_root.setAttribute(KXMLQLCATFBarsNumber, m_spectrumBars.count());
    wksp_root->appendChild(atf_root);

    if (m_volumeBar->m_type != AudioBar::None)
    {
        m_volumeBar->saveXML(doc, &atf_root, KXMLQLCVolumeBar, 1000);
    }
    int idx = 0;
    foreach (AudioBar *bar, m_spectrumBars)
    {
        if (bar->m_type != AudioBar::None)
            bar->saveXML(doc, &atf_root, KXMLQLCSpectrumBar, idx);
        idx++;
    }

    return true;
}

/************************************************************************
 * AudioBar class methods
 ************************************************************************/

AudioBar::AudioBar(int t, uchar v)
{
    m_type = t;
    m_value = v;
    m_dmxChannels.clear();
    m_absDmxChannels.clear();
    m_function = NULL;
    m_widget = NULL;
    m_minThreshold = 51; // 20%
    m_maxThreshold = 204; // 80%
}

AudioBar *AudioBar::createCopy()
{
    AudioBar *copy = new AudioBar();
    copy->m_type = m_type;
    copy->m_value = m_value;
    copy->m_name = m_name;
    copy->m_dmxChannels = m_dmxChannels;
    copy->m_absDmxChannels = m_absDmxChannels;
    copy->m_function = m_function;
    copy->m_widget = m_widget;
    copy->m_minThreshold = m_minThreshold;
    copy->m_maxThreshold = m_maxThreshold;

    return copy;
}

void AudioBar::setName(QString nme)
{
    m_name = nme;
}

void AudioBar::setMinThreshold(uchar value)
{
    m_minThreshold = value;
}

void AudioBar::setMaxThreshold(uchar value)
{
    m_maxThreshold = value;
}

void AudioBar::attachDmxChannels(Doc *doc, QList<SceneValue> list)
{
    m_dmxChannels.clear();
    m_dmxChannels = list;
    m_absDmxChannels.clear();
    foreach(SceneValue scv, m_dmxChannels)
    {
        Fixture *fx = doc->fixture(scv.fxi);
        if (fx != NULL)
        {
            quint32 absAddr = fx->universeAddress() + scv.channel;
            m_absDmxChannels.append(absAddr);
        }
    }
}

void AudioBar::attachFunction(Function *func)
{
    if (func != NULL)
    {
        qDebug() << Q_FUNC_INFO << "Attaching function:" << func->name();
        m_function = func;
    }
}

void AudioBar::attachWidget(VCWidget *widget)
{
    if (widget != NULL)
    {
        qDebug() << Q_FUNC_INFO << "Attaching widget:" << widget->caption();
        m_widget = widget;
    }
}

void AudioBar::checkFunctionThresholds(Doc *doc)
{
    if (m_function == NULL)
        return;
    if (m_value >= m_maxThreshold && m_function->isRunning() == false)
        m_function->start(doc->masterTimer());
    else if (m_value < m_minThreshold && m_function->isRunning() == true)
        m_function->stop();
}

void AudioBar::checkWidgetFunctionality()
{
    if (m_widget == NULL)
        return;

    if (m_widget->type() == VCWidget::ButtonWidget)
    {
        VCButton *btn = (VCButton *)m_widget;
        if (m_value >= m_maxThreshold && btn->isOn() == false)
            btn->setOn(true);
        else if (m_value < m_minThreshold && btn->isOn() == true)
            btn->setOn(false);
    }
    else if (m_widget->type() == VCWidget::SliderWidget)
    {
        VCSlider *slider = (VCSlider *)m_widget;
        slider->setSliderValue(m_value);
    }
}

void AudioBar::debugInfo()
{
    qDebug() << "[AudioBar] " << m_name;
    qDebug() << "   type:" << m_type << ", value:" << m_value;

}

bool AudioBar::loadXML(const QDomElement &root)
{
    if (root.hasAttribute(KXMLQLCAudioBarName))
        m_name = root.attribute(KXMLQLCAudioBarName);

    if (root.hasAttribute(KXMLQLCAudioBarType))
    {
        m_type = root.attribute(KXMLQLCAudioBarType).toInt();

        if (m_type == AudioBar::DMXBar)
        {
            QDomNode node = root.firstChild();
            if (node.isNull() == false)
            {
                QDomElement tag = node.toElement();
                if (tag.tagName() == KXMLQLCAudioBarDMXChannels)
                {
                    QString dmxValues = tag.text();
                    if (dmxValues.isEmpty() == false)
                    {
                        m_dmxChannels.clear();
                        QStringList varray = dmxValues.split(",");
                        for (int i = 0; i < varray.count(); i+=2)
                        {
                            m_dmxChannels.append(SceneValue(QString(varray.at(i)).toUInt(),
                                                         QString(varray.at(i + 1)).toUInt(), 0));
                        }
                    }
                }
            }
        }
    }
    return true;
}

bool AudioBar::saveXML(QDomDocument *doc, QDomElement *atf_root, QString tagName, int index)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(atf_root != NULL);

    qDebug() << Q_FUNC_INFO;

    QDomElement ab_tag = doc->createElement(tagName);
    ab_tag.setAttribute(KXMLQLCAudioBarName, m_name);
    ab_tag.setAttribute(KXMLQLCAudioBarType, m_type);
    ab_tag.setAttribute(KXMLQLCAudioBarIndex, index);
    if (m_type == AudioBar::DMXBar && m_dmxChannels.count() > 0)
    {
        QDomElement dmx_tag = doc->createElement(KXMLQLCAudioBarDMXChannels);
        QString chans;
        foreach (SceneValue scv, m_dmxChannels)
        {
            if (chans.isEmpty() == false)
                chans.append(",");
            chans.append(QString("%1,%2").arg(scv.fxi).arg(scv.channel));
        }
        if (chans.isEmpty() == false)
        {
            QDomText text = doc->createTextNode(chans);
            dmx_tag.appendChild(text);
        }

        ab_tag.appendChild(dmx_tag);
    }
    else if (m_type == AudioBar::FunctionBar && m_function != NULL)
    {
        ab_tag.setAttribute(KXMLQLCAudioBarFunction, m_function->id());
    }
    else if (m_type == AudioBar::VCWidgetBar && m_widget != NULL)
    {
        ab_tag.setAttribute(KXMLQLCAudioBarWidget, m_widget->id());
    }
    atf_root->appendChild(ab_tag);

    return true;
}
