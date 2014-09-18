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
#include "rgbalgorithm.h"

class QScriptEngine;
class QSize;
class QDir;
class QMutex;

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
    RGBScript(const Doc * doc);
    RGBScript(const RGBScript& s);
    ~RGBScript();

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
    static QScriptEngine* s_engine; //! The engine that runs all scripts
    static QMutex* s_engineMutex;   //! Protection
    QString m_fileName;             //! The file name that contains this script
    QString m_contents;             //! The file's contents

    /************************************************************************
     * RGBAlgorithm API
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
    RGBAlgorithm::Type type() const;

    /** @reimp */
    bool loadXML(const QDomElement& root);

    /** @reimp */
    bool saveXML(QDomDocument* Doc, QDomElement* mtx_root) const;

private:
    int m_apiVersion;               //! The API version that the script uses
    QScriptValue m_script;          //! The script itself
    QScriptValue m_rgbMap;          //! rgbMap() function
    QScriptValue m_rgbMapStepCount; //! rgbMapStepCount() function

    /************************************************************************
     * System & User Scripts
     ************************************************************************/
public:
    /** Get a script by its public name */
    static RGBScript script(const Doc * doc, const QString& name);

    /** Get available (user, system and custom) script names */
    static QStringList scriptNames(const Doc * doc);

    /** Get available (user, system and custom) scripts */
    static QList <RGBScript> scripts(const Doc * doc);

    /** Get available scripts from the given directory path */
    static QList <RGBScript> scripts(const Doc * doc, const QDir& path);

    /** The system RGBScript directory */
    static QDir systemScriptDirectory();

    /** The user RGBScript directory */
    static QDir userScriptDirectory();

    /** Set the custom RGBScript directory */
    static void setCustomScriptDirectory(const QString& path);

    /** Get the custom RGBScript directory */
    static QDir customScriptDirectory();

private:
    static QDir s_customScriptDirectory;
};

/** @} */

#endif
