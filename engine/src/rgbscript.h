/*
  Q Light Controller
  rgbscript.h

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef RGBSCRIPT_H
#define RGBSCRIPT_H

#include <QScriptValue>
#include "rgbalgorithm.h"

class QScriptEngine;
class QSize;
class QDir;

#define KXMLQLCRGBScript "Script"

class RGBScript : public RGBAlgorithm
{
    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    RGBScript();
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
    bool saveXML(QDomDocument* doc, QDomElement* mtx_root) const;

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
    static RGBScript script(const QString& name);

    /** Get available (user, system and custom) script names */
    static QStringList scriptNames();

    /** Get available (user, system and custom) scripts */
    static QList <RGBScript> scripts();

    /** Get available scripts from the given directory path */
    static QList <RGBScript> scripts(const QDir& path);

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

#endif
