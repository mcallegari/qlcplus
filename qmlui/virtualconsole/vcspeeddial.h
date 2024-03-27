/*
  Q Light Controller Plus
  vcspeeddial.h

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

#ifndef VCSPEEDDIAL_H
#define VCSPEEDDIAL_H

#include "vcwidget.h"

#define KXMLQLCVCSpeedDial                  QString("SpeedDial")
#define KXMLQLCVCSpeedDialSpeedTypes        QString("SpeedTypes")
#define KXMLQLCVCSpeedDialAbsoluteValue     QString("AbsoluteValue")
#define KXMLQLCVCSpeedDialAbsoluteValueMin  QString("Minimum")
#define KXMLQLCVCSpeedDialAbsoluteValueMax  QString("Maximum")
#define KXMLQLCVCSpeedDialTap               QString("Tap")
#define KXMLQLCVCSpeedDialMult              QString("Mult")
#define KXMLQLCVCSpeedDialDiv               QString("Div")
#define KXMLQLCVCSpeedDialMultDivReset      QString("MultDivReset")
#define KXMLQLCVCSpeedDialApply             QString("Apply")
#define KXMLQLCVCSpeedDialTapKey            QString("Key")
#define KXMLQLCVCSpeedDialMultKey           QString("MultKey")
#define KXMLQLCVCSpeedDialDivKey            QString("DivKey")
#define KXMLQLCVCSpeedDialMultDivResetKey   QString("MultDivResetKey")
#define KXMLQLCVCSpeedDialApplyKey          QString("ApplyKey")
#define KXMLQLCVCSpeedDialResetFactorOnDialChange QString("ResetFactorOnDialChange")
#define KXMLQLCVCSpeedDialVisibilityMask    QString("Visibility")
#define KXMLQLCVCSpeedDialTime              QString("Time")

// Legacy: infinite checkbox
#define KXMLQLCVCSpeedDialInfinite      QString("Infinite")
#define KXMLQLCVCSpeedDialInfiniteKey   QString("InfiniteKey")

class VCSpeedDial : public VCWidget
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCSpeedDial(Doc* doc = nullptr, QObject *parent = nullptr);
    virtual ~VCSpeedDial();

    /** @reimp */
    QString defaultCaption();

    /** @reimp */
    void setupLookAndFeel(qreal pixelDensity, int page);

    /** @reimp */
    void render(QQuickView *view, QQuickItem *parent);

    /** @reimp */
    QString propertiesResource() const;

    /** @reimp */
    VCWidget *createCopy(VCWidget *parent);

protected:
    /** @reimp */
    bool copyFrom(const VCWidget* widget);

private:
    FunctionParent functionParent() const;

    /*********************************************************************
     * Type
     *********************************************************************/
public:

signals:

private:

    /*********************************************************************
     * Data
     *********************************************************************/
public:

protected slots:

signals:

private:

    /*********************************************************************
     * Functions connections
     *********************************************************************/
public:

signals:

private:

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc);
};

#endif
