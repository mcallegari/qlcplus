/*
  Q Light Controller Plus
  rgbplain.h

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

#ifndef RGBPLAIN_H
#define RGBPLAIN_H

#include <QObject>

#include "rgbalgorithm.h"

/** @addtogroup engine_functions Functions
 * @{
 */

#define KXMLQLCRGBPlain QStringLiteral("Plain")

class RGBPlain final : public QObject, public RGBAlgorithm
{
    Q_OBJECT

public:
    RGBPlain(Doc * doc);
    RGBPlain(const RGBPlain& t, QObject *parent = 0);
    ~RGBPlain();

    /** @reimp */
    RGBAlgorithm* clone() const override;

    /************************************************************************
     * RGBAlgorithm
     ************************************************************************/
public:
    /** @reimp */
    int rgbMapStepCount(const QSize& size) override;

    /** @reimp */
    void rgbMapSetColors(const QVector<uint> &colors) override;

    /** @reimp */
    QVector<uint> rgbMapGetColors() override;

    /** @reimp */
    void rgbMap(const QSize& size, uint rgb, int step, RGBMap &map) override;

    /** @reimp */
    QString name() const override;

    /** @reimp */
    QString author() const override;

    /** @reimp */
    int apiVersion() const override;

    /** @reimp */
    void setColors(QVector<QColor> colors) override;

    /** @reimp */
    RGBAlgorithm::Type type() const override;

    /** @reimp */
    int acceptColors() const override;

    /************************************************************************
     * Load & Save
     ************************************************************************/
public:
    /** @reimp */
    bool loadXML(QXmlStreamReader &root) override;

    /** @reimp */
    bool saveXML(QXmlStreamWriter *doc) const override;
};

/** @} */

#endif
