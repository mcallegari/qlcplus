/*
  Q Light Controller Plus
  vcaudiotriggers.h

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

#ifndef VCAUDIOTRIGGERS_H
#define VCAUDIOTRIGGERS_H

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include <QLabel>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
 #include "clickandgoslider.h"
#endif

#include "audiotriggerwidget.h"
#include "dmxsource.h"
#include "vcwidget.h"

class AudioCapture;
class AudioBar;

/** @addtogroup ui_vc_widgets
 * @{
 */

#define KXMLQLCVCAudioTriggers "AudioTriggers"

class VCAudioTriggers : public VCWidget, public DMXSource
{
    Q_OBJECT
    Q_DISABLE_COPY(VCAudioTriggers)

public:
    /** Default size for newly-created widget */
    static const QSize defaultSize;

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCAudioTriggers(QWidget* parent, Doc* doc);
    virtual ~VCAudioTriggers();

    void enableWidgetUI(bool enable);

    /** @reimp */
    virtual void notifyFunctionStarting(quint32 fid);

    /*********************************************************************
     * GUI
     *********************************************************************/
public:
    void enableCapture(bool enable);

public slots:
    void slotEnableButtonToggled(bool toggle);

protected slots:
    void slotDisplaySpectrum(double *spectrumBands, int size, double maxMagnitude, quint32 power);
#if QT_VERSION >= 0x050000
    void slotVolumeChanged(int volume);
#endif

protected:
    QHBoxLayout *m_hbox;
    QToolButton *m_button;
    QLabel *m_label;
    AudioTriggerWidget *m_spectrum;
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    ClickAndGoSlider *m_volumeSlider;
#endif
    AudioCapture *m_inputCapture;

    AudioBar *m_volumeBar;
    QList <AudioBar *> m_spectrumBars;

    /*********************************************************************
     * DMXSource
     *********************************************************************/
public:
    /** @reimpl */
    void writeDMX(MasterTimer* timer, QList<Universe*> universes);

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
    bool copyFrom(const VCWidget* widget);

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

    /*************************************************************************
     * Configuration
     *************************************************************************/
public:

    static int volumeBarIndex() { return 1000; }

    /** Get a pointer to a single AudioBar by index.
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

/** @} */

#endif // VCAUDIOTRIGGERS_H
