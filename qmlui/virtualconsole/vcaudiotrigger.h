/*
  Q Light Controller Plus
  vcaudiotrigger.h

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

#ifndef VCAUDIOTRIGGER_H
#define VCAUDIOTRIGGER_H

#include "vcwidget.h"

#define KXMLQLCVCAudioTriggers QStringLiteral("AudioTriggers")

class VCAudioTrigger : public VCWidget
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCAudioTrigger(Doc* doc = nullptr, QObject *parent = nullptr);
    virtual ~VCAudioTrigger();

    /** @reimp */
    QString defaultCaption() override;

    /** @reimp */
    void setupLookAndFeel(qreal pixelDensity, int page) override;

    /** @reimp */
    void render(QQuickView *view, QQuickItem *parent) override;

    /** @reimp */
    QString propertiesResource() const override;

    /** @reimp */
    VCWidget *createCopy(VCWidget *parent) override;

protected:
    /** @reimp */
    bool copyFrom(const VCWidget* widget) override;

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
    bool loadXML(QXmlStreamReader &root) override;
    bool saveXML(QXmlStreamWriter *doc) override;
};

#endif
