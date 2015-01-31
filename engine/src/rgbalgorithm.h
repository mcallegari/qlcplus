/*
  Q Light Controller
  rgbalgorithm.h

  Copyright (c) Heikki Junnila

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

#ifndef RGBALGORITHM_H
#define RGBALGORITHM_H

#include <QString>
#include <QVector>
#include <QColor>
#include <QSize>

class QDomDocument;
class QDomElement;

class Doc;

/** @addtogroup engine_functions Functions
 * @{
 */

typedef QVector<QVector<uint> > RGBMap;

#define KXMLQLCRGBAlgorithm "Algorithm"
#define KXMLQLCRGBAlgorithmType "Type"

class RGBAlgorithm
{
public:
    RGBAlgorithm(const Doc* doc);
    virtual ~RGBAlgorithm() { /* NOP */ }

    enum Type
    {
        Text,
        Script,
        Image,
        Audio,
        Plain
    };

    /** Create a clone of the algorithm. Caller takes ownership of the pointer. */
    virtual RGBAlgorithm* clone() const = 0;

    const Doc * doc() const { return m_doc; }
 
private:

    const Doc * m_doc;

    /************************************************************************
     * RGB API
     ************************************************************************/
public:
    /** Maximum step count for rgbMap() function. */
    virtual int rgbMapStepCount(const QSize& size) = 0;

    /** Get the RGBMap for the given step. */
    virtual RGBMap rgbMap(const QSize& size, uint rgb, int step) = 0;

    /** Get the name of the algorithm. */
    virtual QString name() const = 0;

    /** Get the algorithm's author's name. */
    virtual QString author() const = 0;

    /** Get the algorithm's API version. 0 == invalid or unevaluated script. */
    virtual int apiVersion() const = 0;

    /** Get the algorithm's type */
    virtual Type type() const = 0;

    /** Return if the algorithm accepts/needs colors:
     *  0 = colors not accepted (e.g. the algorithm will generate them on its own)
     *  1 = only start color is accepted
     *  2 = start and end colors are both accepted
     */
    virtual int acceptColors() const = 0;

    /************************************************************************
     * RGB Colors
     ************************************************************************/
public:
    /** Set the start/end color the algorithm can use */
    virtual void setColors(QColor start, QColor end);

    QColor startColor() { return m_startColor; }

    QColor endColor() { return m_endColor; }

private:
    QColor m_startColor, m_endColor;

    /************************************************************************
     * Available algorithms
     ************************************************************************/
public:
    static QStringList algorithms(const Doc * doc);
    static RGBAlgorithm* algorithm(const Doc * doc, const QString& name);

    /************************************************************************
     * Load & Save
     ************************************************************************/
public:
    /** Load an RGBAlgorithm from a workspace file and return it as a new pointer. */
    static RGBAlgorithm* loader(const Doc *doc, const QDomElement& root);

    /** Save the contents of an RGBAlgorithm (run-time info) to a workspace file. */
    virtual bool saveXML(QDomDocument* doc, QDomElement* root) const = 0;
};

/** @} */

#endif
