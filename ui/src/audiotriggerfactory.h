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

class AudioCapture;

class AudioBar
{
public:
    /** Normal constructor */
    AudioBar(int t, uchar v);

    /** Destructor */
    ~AudioBar() { }

    enum BarType
    {
        None = 0,
        DMXBar,
        FunctionBar,
        VCWidgetBar
    };

    void setName(QString nme);
    void attachDmxChannels(QList<SceneValue>list);
    void attachFunction(Function *func);
    void attachWidget(VCWidget *widget);

    void debugInfo();

public:
    QString m_name;
    int m_type;
    uchar m_value;

    QList<SceneValue> m_dmxChannels;
    Function *m_function;
    VCWidget *m_widget;

    uchar min_threshold, max_threshold;
};

class AudioTriggerFactory : public QDialog, public Ui_AudioTriggerFactory, public DMXSource
{
    Q_OBJECT
    
public:
    explicit AudioTriggerFactory(Doc* doc, QWidget *parent = 0);
    ~AudioTriggerFactory();

    AudioBar *getSpectrumBar(int index);
    void setSpectrumBarsNumber(int num);
    void setSpectrumBarType(int index, int type);
    
protected slots:
    void slotEnableCapture(bool enable);
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
};

#endif // AUDIOTRIGGERFACTORY_H
