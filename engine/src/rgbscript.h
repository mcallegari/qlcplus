/*
  Q Light Controller
  rgbscript.h

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

#ifndef RGBSCRIPT_H
#define RGBSCRIPT_H

#include <QScriptValue>
#include <QMutex>
#include "rgbalgorithm.h"
#include "rgbscriptproperty.h"

class QScriptEngine;
class QSize;
class QDir;

/** @addtogroup engine_functions Functions
 * @{
 */

#define KXMLQLCRGBScript "Script"

class RGBScript : public RGBAlgorithm
{
    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    RGBScript(Doc *doc);
    RGBScript(const RGBScript& s);
    ~RGBScript();

    RGBScript& operator=(const RGBScript& s);

    /** Comparison operator. Uses simply fileName() == s.fileName(). */
    bool operator==(const RGBScript& s) const;

    /** @reimp */
    RGBAlgorithm* clone() const;

    /************************************************************************
     * Load & Evaluation
     ************************************************************************/
public:
    /** Load script contents from $file located in $dir */
    bool load(const QDir& dir, const QString& fileName);

    /** Get the filename for this script */
    QString fileName() const;

    /** Evaluate the script's contents and see if it checks out */
    bool evaluate();

private:
    static QScriptEngine *s_engine; //! The engine that runs all scripts
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    static QMutex* s_engineMutex;   //! Protection
#else
    static QRecursiveMutex* s_engineMutex;
#endif
    QString m_fileName;             //! The file name that contains this script
    QString m_contents;             //! The file's contents

private:
    /** Init engine, engine mutex, and scripts map */
    static void initEngine();

    /** Handle an error after evaluate() or call() of a script */
    static void displayError(QScriptValue e, const QString& fileName);

    /************************************************************************
     * RGBAlgorithm API
     ************************************************************************/
public:
    /** @reimp */
    int rgbMapStepCount(const QSize& size);

    /** @reimp */
    void rgbMapSetColors(QVector<uint> &colors);

    /** @reimp */
    QVector<uint> rgbMapGetColors();

    /** @reimp */
    void rgbMap(const QSize& size, uint rgb, int step, RGBMap &map);

    /** @reimp */
    QString name() const;

    /** @reimp */
    QString author() const;

    /** @reimp */
    int apiVersion() const;

    /** @reimp */
    RGBAlgorithm::Type type() const;

    /** @reimp */
    int acceptColors() const;

    /** @reimp */
    bool loadXML(QXmlStreamReader &root);

    /** @reimp */
    bool saveXML(QXmlStreamWriter *doc) const;

private:
    int m_apiVersion;               //! The API version that the script uses
    QScriptValue m_script;          //! The script itself
    QScriptValue m_rgbMap;          //! rgbMap() function
    QScriptValue m_rgbMapStepCount; //! rgbMapStepCount() function
    QScriptValue m_rgbMapSetColors; //! rgbMapSetColors() function
    QScriptValue m_rgbMapGetColors; //! rgbMapSetColors() function

    /************************************************************************
     * Properties
     ************************************************************************/
public:
    /** Return a list of the loaded script properties */
    QList<RGBScriptProperty> properties();

    /** Return properties as strings */
    QHash<QString, QString> propertiesAsStrings();

    /** Set a property to the given value */
    bool setProperty(QString propertyName, QString value);

    /** Read the value of the property with the given name */
    QString property(QString propertyName) const;

private:
    /** Load the script properties if any is available */
    bool loadProperties();

private:
    QList<RGBScriptProperty> m_properties; //! the script properties list
};

/** @} */

#endif
