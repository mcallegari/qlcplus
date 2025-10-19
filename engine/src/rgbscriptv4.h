/*
  Q Light Controller Plus
  rgbscriptv4.h

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

#ifndef RGBSCRIPTV4_H
#define RGBSCRIPTV4_H

#include <QHash>
#include <QJSValue>

#include "rgbalgorithm.h"
#include "rgbscriptproperty.h"

class QJSEngine;
class QDir;

class JSThread;

/** @addtogroup engine_functions Functions
 * @{
 */

#define KXMLQLCRGBScript QStringLiteral("Script")

class RGBScript final : public RGBAlgorithm
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
    RGBAlgorithm* clone() const override;

    /************************************************************************
     * Load & Evaluation
     ************************************************************************/
public:
    /** Load script contents from $file */
    bool load(const QString& fileName);

    /** Get the filename for this script */
    QString fileName() const;

    /** Evaluate the script's contents and see if it checks out */
    bool evaluate();

private:
    static void initEngine();
    static void cleanupEngine();

    /** Handle an error after evaluate() or call() of a script */
    static void displayError(QJSValue e, const QString& fileName);

private:
    QString m_fileName;             //! The file name that contains this script
    QString m_contents;             //! The file's contents
    static JSThread *s_jsThread;

    /************************************************************************
     * RGBAlgorithm API
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
    RGBAlgorithm::Type type() const override;

    /** @reimp */
    int acceptColors() const override;

    /** @reimp */
    bool loadXML(QXmlStreamReader &root) override;

    /** @reimp */
    bool saveXML(QXmlStreamWriter *doc) const override;

private:
    int m_apiVersion;           //! The API version that the script uses
    QJSValue m_script;          //! The script itself
    QJSValue m_rgbMap;          //! rgbMap() function
    QJSValue m_rgbMapStepCount; //! rgbMapStepCount() function
    QJSValue m_rgbMapSetColors; //! rgbMapSetColors() function
    QJSValue m_rgbMapGetColors; //! rgbMapSetColors() function

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
