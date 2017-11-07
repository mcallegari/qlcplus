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
    Q_PROPERTY(ButtonState state READ state WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(quint32 functionID READ functionID WRITE setFunctionID NOTIFY functionIDChanged)
    Q_PROPERTY(bool startupIntensityEnabled READ startupIntensityEnabled WRITE setStartupIntensityEnabled NOTIFY startupIntensityEnabledChanged)
    Q_PROPERTY(qreal startupIntensity READ startupIntensity WRITE setStartupIntensity NOTIFY startupIntensityChanged)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCButton(Doc* doc = NULL, QObject *parent = 0);
    virtual ~VCButton();

    /** @reimp */
    QString defaultCaption();

    /** @reimp */
    void setupLookAndFeel(qreal pixelDensity, int page);

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

    /** @reimp */
    void adjustFunctionIntensity(Function *f, qreal value);

    /**
     *  The actual method used to request a change of state of this
     *  Button. Depending on the action type this will start/stop
     *  the attached Function, if any */
    Q_INVOKABLE void requestStateChange(bool pressed);

    /** @reimp */
    void notifyFunctionStarting(VCWidget *widget, quint32 fid, qreal fIntensity);

signals:
    void functionIDChanged(quint32 id);

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
    enum ButtonState
    {
        Inactive,
        Monitoring,
        Active
    };
    Q_ENUM(ButtonState)

    /** Get/Set the button pressure state */
    ButtonState state() const;
    void setState(ButtonState state);

signals:
    /** Signal emitted when the button has actually changed the graphic state */
    void stateChanged(int state);

protected:
    ButtonState m_state;

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

    /*****************************************************************************
     * Function startup intensity adjustment
     *****************************************************************************/
public:
    /** Get/Set if a startup intensity amount should be applied
     *  when starting the attached Function */
    bool startupIntensityEnabled() const;
    void setStartupIntensityEnabled(bool enable);

    /** Get/Set the amount of intensity adjustment applied
     *  when starting the attached Function */
    qreal startupIntensity() const;
    void setStartupIntensity(qreal fraction);

signals:
    void startupIntensityEnabledChanged();
    void startupIntensityChanged();

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
    /** @reimp */
    bool loadXML(QXmlStreamReader &root);

    /** @reimp */
    bool saveXML(QXmlStreamWriter *doc);
};

#endif
