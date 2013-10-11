/*
  Q Light Controller Plus
  vcaudiotriggers.h

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

#ifndef VCAUDIOTRIGGERS_H
#define VCAUDIOTRIGGERS_H

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include <QLabel>

#include "audiotriggerwidget.h"
#include "dmxsource.h"
#include "vcwidget.h"

#define KXMLQLCVCAudioTriggers "AudioTriggers"

class AudioCapture;
class AudioBar;

class VCAudioTriggers : public VCWidget, public DMXSource
{
    Q_OBJECT
    Q_DISABLE_COPY(VCAudioTriggers)

public:
    /** Default size for newly-created frames */
    static const QSize defaultSize;

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCAudioTriggers(QWidget* parent, Doc* doc);
    virtual ~VCAudioTriggers();

    /*********************************************************************
     * GUI
     *********************************************************************/
public:
    void enableCapture(bool enable);

signals:
    void enableRequest(quint32 id);

public slots:
    void slotEnableButtonToggled(bool toggle);

protected slots:
    void slotDisplaySpectrum(double *spectrumBands, double maxMagnitude, quint32 power);

protected:
    QHBoxLayout *m_hbox;
    QToolButton *m_button;
    QLabel *m_label;
    AudioTriggerWidget *m_spectrum;
    AudioCapture *m_inputCapture;

    AudioBar *m_volumeBar;
    QList <AudioBar *> m_spectrumBars;

    /*********************************************************************
     * DMXSource
     *********************************************************************/
public:
    /** @reimpl */
    void writeDMX(MasterTimer* timer, UniverseArray* universes);

    /*********************************************************************
     * Key sequence handler
     *********************************************************************/
public:
    void setKeySequence(const QKeySequence& keySequence);
    QKeySequence keySequence() const;

protected slots:
    void slotKeyPressed(const QKeySequence& keySequence);

protected:
    QKeySequence m_keySequence;

    /*************************************************************************
     * External Input
     *************************************************************************/
public:
    void updateFeedback() { }

protected slots:
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value);

    /*********************************************************************
     * Clipboard
     *********************************************************************/
public:
    /** Create a copy of this widget into the given parent */
    VCWidget* createCopy(VCWidget* parent);

protected:
    /** Copy the contents for this widget from another widget */
    bool copyFrom(VCWidget* widget);

    /*************************************************************************
     * VCWidget-inherited
     *************************************************************************/
public:
    /** @reimp */
    void setCaption(const QString& text);

    /** @reimp */
    void setForegroundColor(const QColor& color);

    /** @reimp */
    QColor foregroundColor() const;

    /** @reimp */
    void slotModeChanged(Doc::Mode mode);

    /** @reimp */
    void editProperties();

    /*********************************************************************
     * Web access
     *********************************************************************/
public:
    /** @reimpl */
    QString getCSS();

    /** @reimpl */
    QString getJS();

    /*************************************************************************
     * Configuration
     *************************************************************************/
public:
    /** Get a pointer to a single AudioBar by index.
     *  Note that volume bar has index = 1000
     */
    AudioBar *getSpectrumBar(int index);

    /** Get a list of pointers to all the current audio bars */
    QList<AudioBar *> getAudioBars();

    void setSpectrumBarsNumber(int num);
    void setSpectrumBarType(int index, int type);

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /**
     * Load a VCButton's properties from an XML document node
     *
     * @param doc An XML document to load from
     * @param btn_root A VCButton XML root node containing button properties
     * @return true if successful; otherwise false
     */
    bool loadXML(const QDomElement* root);

    /**
     * Save a VCButton's properties to an XML document node
     *
     * @param doc The master XML document to save to
     * @param frame_root The button's VCFrame XML parent node to save to
     */
    bool saveXML(QDomDocument* doc, QDomElement* vc_root);
};

#endif // VCAUDIOTRIGGERS_H
