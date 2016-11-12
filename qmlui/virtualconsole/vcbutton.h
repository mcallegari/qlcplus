/*
  Q Light Controller Plus
  vcbutton.h

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

#ifndef VCBUTTON_H
#define VCBUTTON_H

#include "vcwidget.h"

#define KXMLQLCVCButton "Button"

#define KXMLQLCVCButtonFunction "Function"
#define KXMLQLCVCButtonFunctionID "ID"

#define KXMLQLCVCButtonAction "Action"
#define KXMLQLCVCButtonActionFlash "Flash"
#define KXMLQLCVCButtonActionToggle "Toggle"
#define KXMLQLCVCButtonActionBlackout "Blackout"
#define KXMLQLCVCButtonActionStopAll "StopAll"

#define KXMLQLCVCButtonStopAllFadeTime "FadeOut"

#define KXMLQLCVCButtonIntensity "Intensity"
#define KXMLQLCVCButtonIntensityAdjust "Adjust"

class FunctionParent;

class VCButton : public VCWidget
{
    Q_OBJECT

    Q_PROPERTY(ButtonAction actionType READ actionType WRITE setActionType NOTIFY actionTypeChanged)
    Q_PROPERTY(bool isOn READ isOn WRITE setOn NOTIFY isOnChanged)
    Q_PROPERTY(quint32 functionID READ functionID WRITE setFunctionID NOTIFY functionIDChanged)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCButton(Doc* doc = NULL, QObject *parent = 0);
    virtual ~VCButton();

    /** @reimp */
    void setID(quint32 id);

    /** @reimp */
    QString defaultCaption();

    /** @reimp */
    void render(QQuickView *view, QQuickItem *parent);

    /** @reimp */
    QString propertiesResource() const;

    /*********************************************************************
     * Function attachment
     *********************************************************************/
public:
    /**
     * Attach a function to a VCButton. This function is started when the
     * button is pressed down.
     *
     * @param function An ID of a function to attach
     */
    Q_INVOKABLE void setFunctionID(quint32 fid);

    /**
     * Get the ID of the function attached to a VCButton
     *
     * @return The ID of the attached function or Function::invalidId()
     *         if there isn't one
     */
    quint32 functionID() const;

    /**
     *  The actual method used to request a change of state of this
     *  Button. Depending on the action type this will start/stop
     *  the attached Function, if any */
    Q_INVOKABLE void requestStateChange(bool pressed);

    /** @reimp */
    void notifyFunctionStarting(VCWidget *widget, quint32 fid, qreal fIntensity);

protected slots:
    /** Handler for function running signal */
    void slotFunctionRunning(quint32 fid);

    /** Handler for function stop signal */
    void slotFunctionStopped(quint32 fid);

    /** Basically the same as slotFunctionStopped() but for flash signal */
    void slotFunctionFlashing(quint32 fid, bool state);

private:
    FunctionParent functionParent() const;

protected:
    /** The ID of the Function that this button is controlling */
    quint32 m_functionID;

    /*********************************************************************
     * Button state
     *********************************************************************/
public:
    /** Get the current on/off state of the button */
    bool isOn() const;

    /** Set the button on/off state */
    void setOn(bool isOn);

signals:
    void isOnChanged(bool isOn);
    void functionIDChanged(quint32 id);

protected:
    bool m_isOn;

    /*********************************************************************
     * Button action
     *********************************************************************/
public:
    /**
     * Toggle: Start/stop the assigned function.
     * Flash: Keep the function running as long as the button is kept down.
     * Blackout: Toggle blackout on/off.
     * StopAll: Stop all functions (panic button).
     */
    enum ButtonAction { Toggle, Flash, Blackout, StopAll };
    Q_ENUM(ButtonAction)

    ButtonAction actionType() const;

    void setActionType(ButtonAction actionType);

    static QString actionToString(ButtonAction action);
    static ButtonAction stringToAction(const QString& str);

    void setStopAllFadeOutTime(int ms);
    int stopAllFadeTime();

signals:
    void actionTypeChanged(ButtonAction actionType);

protected:
    ButtonAction m_actionType;
    /** if button action is StopAll, this indicates the time
     *  in milliseconds of fadeout before stopping */
    int m_blackoutFadeOutTime;

    /*********************************************************************
     * Startup intensity adjustment
     *********************************************************************/
public:
    /**
     * Make the button adjust the attached function's intensity when the
     * button is used to start the function.
     *
     * @param enable true to make the button adjust intensity, false to disable
     *               intensity adjustment
     */
    void enableStartupIntensity(bool enable);

    /** Check, whether the button adjusts intensity */
    bool isStartupIntensityEnabled() const;

    /**
     * Set the amount of the startupintensity adjustment.
     *
     * @param fraction Intensity adjustment amount (0.0 - 1.0)
     */
    void setStartupIntensity(qreal fraction);

    /** Get the amount of intensity adjustment. */
    qreal startupIntensity() const;

protected:
    bool m_startupIntensityEnabled;
    qreal m_startupIntensity;

    /*********************************************************************
     * External input
     *********************************************************************/
public slots:
    /** @reimp */
    void slotInputValueChanged(quint8 id, uchar value);

    /*********************************************************************
     * Load & Save
     *********************************************************************/

public:
    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc);
};

#endif
