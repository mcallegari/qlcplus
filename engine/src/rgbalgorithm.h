/*
  Q Light Controller Plus
  rgbalgorithm.h

  Copyright (c) Heikki Junnila
                Massimo Callegari

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

class QXmlStreamReader;
class QXmlStreamWriter;

class Doc;

/** @addtogroup engine_functions Functions
 * @{
 */

typedef QVector<QVector<uint> > RGBMap;

#define KXMLQLCRGBAlgorithm     QString("Algorithm")
#define KXMLQLCRGBAlgorithmType QString("Type")

#define RGBAlgorithmColorDisplayCount 5

class RGBAlgorithm
{
public:
    RGBAlgorithm(Doc* doc);
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

    Doc * doc() const { return m_doc; }
    Doc * doc() { return m_doc; }

private:

    Doc * m_doc;

    /************************************************************************
     * RGB API
     ************************************************************************/
public:
    /** Maximum step count for rgbMap() function. */
    virtual int rgbMapStepCount(const QSize& size) = 0;

    /** Set the colors for the RGBmap */
    virtual void rgbMapSetColors(QVector<uint> &colors) = 0;

    /** Get the colors from the RGB script */
    virtual QVector<uint> rgbMapGetColors() = 0;

    /** Load a RGBMap for the given step. */
    virtual void rgbMap(const QSize& size, uint rgb, int step, RGBMap &map) = 0;

    /** Release resources that may have been acquired in rgbMap() */
    virtual void postRun() {}

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
    /** Set the colors the algorithm can use */
    virtual void setColors(QVector<QColor>);

    /** Get the color which is set for the algorithm */
    virtual QColor getColor(uint i) const;

private:
    QVector<QColor> m_colors;

    /************************************************************************
     * Available algorithms
     ************************************************************************/
public:
    static QStringList algorithms(Doc * doc);
    static RGBAlgorithm* algorithm(Doc * doc, const QString& name);

    /************************************************************************
     * Load & Save
     ************************************************************************/
public:
    /** Load an RGBAlgorithm from a workspace file and return it as a new pointer. */
    static RGBAlgorithm* loader(Doc *doc, QXmlStreamReader &root);

    /** Load the contents of information saved in XML into a RGBAlgorithm  object */
    virtual bool loadXML(QXmlStreamReader &root) = 0;

    /** Save the contents of an RGBAlgorithm (run-time info) to a workspace file. */
    virtual bool saveXML(QXmlStreamWriter *doc) const = 0;
};

/** @} */

#endif
