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

#include "audiotriggerwidget.h"
#include "clickandgoslider.h"
#include "dmxsource.h"
#include "vcwidget.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class AudioCapture;
class AudioBar;

/** @addtogroup ui_vc_widgets
 * @{
 */

#define KXMLQLCVCAudioTriggers QString("AudioTriggers")

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
    virtual void notifyFunctionStarting(quint32 fid, qreal intensity);

    /*********************************************************************
     * GUI
     *********************************************************************/
public:
    void enableCapture(bool enable);

    /** Method to toggle the enable button at a UI level.
     *  In this way we let Qt to handle the toggle signal and
     *  start the audio capture in the correct thread */
    void toggleEnableButton(bool toggle);

public slots:
    void slotEnableButtonToggled(bool toggle);

signals:
    void captureEnabled(bool enabled);

protected slots:
    void slotDisplaySpectrum(double *spectrumBands, int size, double maxMagnitude, quint32 power);
    void slotVolumeChanged(int volume);
    void slotUpdateVolumeSlider(int volume);

protected:
    QHBoxLayout *m_hbox;
    QToolButton *m_button;
    QLabel *m_label;
    AudioTriggerWidget *m_spectrum;
    ClickAndGoSlider *m_volumeSlider;
    AudioCapture *m_inputCapture;

    AudioBar *m_volumeBar;
    QList <AudioBar *> m_spectrumBars;

    /*********************************************************************
     * DMXSource
     *********************************************************************/
public:
    /** @reimpl */
    void writeDMX(MasterTimer* timer, QList<Universe*> universes);

private:
    /** Map used to lookup a GenericFader instance for a Universe ID */
    QMap<quint32, QSharedPointer<GenericFader> > m_fadersMap;

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
    void updateFeedback();

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

    /** @reimp */
    void adjustIntensity(qreal val);

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
    bool loadXML(QXmlStreamReader &root);

    /**
     * Save a VCButton's properties to an XML document node
     *
     * @param doc The master XML document to save to
     * @param frame_root The button's VCFrame XML parent node to save to
     */
    bool saveXML(QXmlStreamWriter *doc);
};

/** @} */

#endif // VCAUDIOTRIGGERS_H
