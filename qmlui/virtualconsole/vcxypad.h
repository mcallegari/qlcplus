/*
  Q Light Controller Plus
  vcxypad.h

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

#ifndef VCXYPAD_H
#define VCXYPAD_H

#include "vcwidget.h"

#define KXMLQLCVCXYPad QString("XYPad")
#define KXMLQLCVCXYPadPan           QString("Pan")
#define KXMLQLCVCXYPadTilt          QString("Tilt")
#define KXMLQLCVCXYPadWidth         QString("Width")
#define KXMLQLCVCXYPadHeight        QString("Height")
#define KXMLQLCVCXYPadPosition      QString("Position")
#define KXMLQLCVCXYPadRangeWindow   QString("Window")
#define KXMLQLCVCXYPadRangeHorizMin QString("hMin")
#define KXMLQLCVCXYPadRangeHorizMax QString("hMax")
#define KXMLQLCVCXYPadRangeVertMin  QString("vMin")
#define KXMLQLCVCXYPadRangeVertMax  QString("vMax")

#define KXMLQLCVCXYPadPositionX "X" // Legacy
#define KXMLQLCVCXYPadPositionY "Y" // Legacy

#define KXMLQLCVCXYPadInvertedAppearance "InvertedAppearance"

class VCXYPad : public VCWidget
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCXYPad(Doc* doc = nullptr, QObject *parent = nullptr);
    virtual ~VCXYPad();

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
