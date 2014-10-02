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

#define KXMLQLCRGBPlain "Plain"

class RGBPlain : public QObject, public RGBAlgorithm
{
    Q_OBJECT

public:
    RGBPlain(const Doc * doc);
    RGBPlain(const RGBPlain& t, QObject *parent = 0);
    ~RGBPlain();

    /** @reimp */
    RGBAlgorithm* clone() const;

    /************************************************************************
     * RGBAlgorithm
     ************************************************************************/
public:
    /** @reimp */
    int rgbMapStepCount(const QSize& size);

    /** @reimp */
    RGBMap rgbMap(const QSize& size, uint rgb, int step);

    /** @reimp */
    QString name() const;

    /** @reimp */
    QString author() const;

    /** @reimp */
    int apiVersion() const;

    /** @reimp */
    void setColors(QColor start, QColor end);

    /** @reimp */
    RGBAlgorithm::Type type() const;

    /************************************************************************
     * Load & Save
     ************************************************************************/
public:
    /** @reimp */
    bool loadXML(const QDomElement& root);

    /** @reimp */
    bool saveXML(QDomDocument* doc, QDomElement* mtx_root) const;
};

/** @} */

#endif
