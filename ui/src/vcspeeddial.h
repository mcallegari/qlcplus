/*
  Q Light Controller
  vcspeeddial.h

  Copyright (c) Heikki Junnila

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

#ifndef VCSPEEDDIAL_H
#define VCSPEEDDIAL_H

#include <QSet>

#include "vcwidget.h"

#define KXMLQLCVCSpeedDial "SpeedDial"
#define KXMLQLCVCSpeedDialFunction "Function"
#define KXMLQLCVCSpeedDialSpeedTypes "SpeedTypes"
#define KXMLQLCVCSpeedDialAbsoluteValue "AbsoluteValue"
#define KXMLQLCVCSpeedDialAbsoluteValueMin "Minimum"
#define KXMLQLCVCSpeedDialAbsoluteValueMax "Maximum"
#define KXMLQLCVCSpeedDialTap "Tap"
#define KXMLQLCVCSpeedDialTapKey "Key"

class QDomDocument;
class QDomElement;
class SpeedDial;

class VCSpeedDial : public VCWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(VCSpeedDial)

public:
    static const quint8 absoluteInputSourceId;
    static const quint8 tapInputSourceId;
    static const QSize defaultSize;

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    VCSpeedDial(QWidget* parent, Doc* doc);
    ~VCSpeedDial();

    /*************************************************************************
     * Clipboard
     *************************************************************************/
public:
    /** @reimp */
    VCWidget* createCopy(VCWidget* parent);

protected:
    /** @reimp */
    bool copyFrom(VCWidget* widget);

    /*************************************************************************
     * Caption
     *************************************************************************/
public:
    /** @reimp */
    void setCaption(const QString& text);

    /*************************************************************************
     * QLC Mode
     *************************************************************************/
public slots:
    /** @reimp */
    void slotModeChanged(Doc::Mode mode);

    /*************************************************************************
     * Properties
     *************************************************************************/
public:
    /** @reimp */
    void editProperties();

    /************************************************************************
     * Speed Type
     ************************************************************************/
public:
    enum SpeedType
    {
        FadeIn   = 1 << 0,
        FadeOut  = 1 << 1,
        Duration = 1 << 2
    };
    Q_DECLARE_FLAGS(SpeedTypes, SpeedType)

    /**
     * Set the speed type that is to be controlled thru the dial. See
     * enum SpeedType for possible values.
     *
     * @param type The Speed type to control
     */
    void setSpeedTypes(VCSpeedDial::SpeedTypes types);

    /**
     * Get the speed type that is controlled thru the dial.
     *
     * @return The speed type controlled by the dial
     */
    VCSpeedDial::SpeedTypes speedTypes() const;

private:
    SpeedTypes m_speedTypes;

    /************************************************************************
     * Functions
     ************************************************************************/
public:
    /**
     * Set the set of functions that are controlled by the dial.
     */
    void setFunctions(const QSet <quint32> ids);

    /**
     * Get the set of functions that are controlled by the dial.
     */
    QSet <quint32> functions() const;

private slots:
    /** Catch dial value changes and patch them to controlled functions */
    void slotDialValueChanged(int ms);

    /** Catch dial tap button clicks and patch them to controlled functions */
    void slotDialTapped();

private:
    QSet <quint32> m_functions;
    SpeedDial* m_dial;

    /*********************************************************************
     * External input
     *********************************************************************/
public:
    /** @reimp */
    void updateFeedback();

protected slots:
    /** @reimp */
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value);

    /*********************************************************************
     * Tap key sequence handler
     *********************************************************************/
public:
    void setKeySequence(const QKeySequence& keySequence);
    QKeySequence keySequence() const;

protected slots:
    void slotKeyPressed(const QKeySequence& keySequence);

protected:
    QKeySequence m_tapKeySequence;

    /************************************************************************
     * Absolute value range
     ************************************************************************/
public:
    void setAbsoluteValueRange(uint min, uint max);
    uint absoluteValueMin() const;
    uint absoluteValueMax() const;

private:
    uint m_absoluteValueMin;
    uint m_absoluteValueMax;

    /*************************************************************************
     * Load & Save
     *************************************************************************/
public:
    /** @reimp */
    bool loadXML(const QDomElement* root);

    /** @reimp */
    bool saveXML(QDomDocument* doc, QDomElement* vc_root);

    /** @reimp */
    void postLoad();
};
Q_DECLARE_OPERATORS_FOR_FLAGS(VCSpeedDial::SpeedTypes)

#endif
