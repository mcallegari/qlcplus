/*
  Q Light Controller Plus
  audiotriggerfactory.h

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

#ifndef AUDIOTRIGGERFACTORY_H
#define AUDIOTRIGGERFACTORY_H

#include <QDialog>
#include <QThread>

#include "ui_audiotriggerfactory.h"
#include "audiotriggerwidget.h"
#include "scenevalue.h"
#include "dmxsource.h"
#include "function.h"
#include "vcwidget.h"
#include "doc.h"

#define KXMLQLCAudioTriggerFactory "AudioTriggerFactory"

class AudioCapture;

class AudioBar
{
public:
    /** Normal constructor */
    AudioBar(int t = 0, uchar v = 0);

    /** Destructor */
    ~AudioBar() { }

    enum BarType
    {
        None = 0,
        DMXBar,
        FunctionBar,
        VCWidgetBar
    };

    AudioBar *createCopy();
    void setName(QString nme);
    void setMinThreshold(uchar value);
    void setMaxThreshold(uchar value);

    void attachDmxChannels(Doc *doc, QList<SceneValue>list);
    void attachFunction(Function *func);
    void attachWidget(VCWidget *widget);

    void checkFunctionThresholds(Doc *doc);
    void checkWidgetFunctionality();

    void debugInfo();

    /** Load properties and contents from an XML tree */
    bool loadXML(const QDomElement& root);

    /** Save properties and contents to an XML document */
    bool saveXML(QDomDocument* doc, QDomElement* atf_root, QString tagName, int index);

public:
    QString m_name;
    int m_type;
    uchar m_value;

    /** List of individual DMX channels when m_type == DMXBar */
    QList<SceneValue> m_dmxChannels;
    /** List of absolute DMX channel addresses when m_type == DMXBar.
      * This is precalculated to speed up writeDMX */
    QList<int> m_absDmxChannels;
    /** Reference to an attached Function when m_type == FunctionBar */
    Function *m_function;
    /** Reference to an attached VCWidget when m_type == VCWidgetBar */
    VCWidget *m_widget;

    uchar m_minThreshold, m_maxThreshold;
};

class AudioTriggerFactory : public QDialog, public Ui_AudioTriggerFactory, public DMXSource
{
    Q_OBJECT
    
public:
    explicit AudioTriggerFactory(Doc* doc, QWidget *parent = 0);
    ~AudioTriggerFactory();

    /** Get the singleton instance */
    static AudioTriggerFactory* instance();

    /** Get a pointer to a single AudioBar by index.
     *  Note that volume bar has index = 1000
     */
    AudioBar *getSpectrumBar(int index);

    /** Get a list of pointers to all the current audio bars */
    QList<AudioBar *> getAudioBars();

    void setSpectrumBarsNumber(int num);
    void setSpectrumBarType(int index, int type);
    
private:
    static AudioTriggerFactory* s_instance;

protected slots:
    void slotEnableCapture(bool enable);
    void slotDisplaySpectrum(double *spectrumBands, double maxMagnitude, quint32 power);
    void slotConfiguration();

private:
    Doc *m_doc;
    AudioCapture *m_inputCapture;
    AudioTriggerWidget *m_spectrum;

    AudioBar *m_volumeBar;
    QList <AudioBar *> m_spectrumBars;

    /*********************************************************************
     * DMXSource
     *********************************************************************/
public:
    /** @reimpl */
    void writeDMX(MasterTimer* timer, UniverseArray* universes);

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /** Load properties and contents from an XML tree */
    bool loadXML(const QDomElement& root);

    /** Save properties and contents to an XML document */
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root);
};

#endif // AUDIOTRIGGERFACTORY_H
