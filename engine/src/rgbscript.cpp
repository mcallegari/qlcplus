/*
  Q Light Controller
  rgbscript.cpp

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

#include <QCoreApplication>
#include <QScriptEngine>
#include <QScriptValue>
#include <QDomDocument>
#include <QDomElement>
#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <QFile>
#include <QSize>
#include <QDir>

#ifdef WIN32
#   include <windows.h>
#else
#   include <unistd.h>
#endif

#include "rgbscript.h"
#include "qlcconfig.h"

QDir RGBScript::s_customScriptDirectory = QDir(QString(), QString("*.js"),
                                               QDir::Name | QDir::IgnoreCase,
                                               QDir::Files);

QScriptEngine* RGBScript::s_engine = NULL;

/****************************************************************************
 * Initialization
 ****************************************************************************/

RGBScript::RGBScript()
    : RGBAlgorithm()
    , m_apiVersion(0)
{
}

RGBScript::RGBScript(const RGBScript& s)
    : RGBAlgorithm()
    , m_fileName(s.m_fileName)
    , m_contents(s.m_contents)
    , m_apiVersion(0)
{
    evaluate();
}

RGBScript::~RGBScript()
{
}

bool RGBScript::operator==(const RGBScript& s) const
{
    if (this->fileName().isEmpty() == false && this->fileName() == s.fileName())
        return true;
    else
        return false;
}

RGBAlgorithm* RGBScript::clone() const
{
    RGBScript* script = new RGBScript(*this);
    return static_cast<RGBAlgorithm*> (script);
}

/****************************************************************************
 * Load & Evaluation
 ****************************************************************************/

bool RGBScript::load(const QDir& dir, const QString& fileName)
{
    m_contents.clear();
    m_script = QScriptValue();
    m_rgbMap = QScriptValue();
    m_rgbMapStepCount = QScriptValue();
    m_apiVersion = 0;

    m_fileName = fileName;
    QFile file(dir.absoluteFilePath(m_fileName));
    if (file.open(QIODevice::ReadOnly) == false)
    {
        qWarning() << "Unable to load RGB script" << m_fileName << "from" << dir.absolutePath();
        return false;
    }

    QTextStream stream(&file);
    m_contents = stream.readAll();
    file.close();

    QScriptSyntaxCheckResult result = QScriptEngine::checkSyntax(m_contents);
    if (result.state() == QScriptSyntaxCheckResult::Valid)
    {
        return evaluate();
    }
    else
    {
        qWarning() << m_fileName << "Error at line:" << result.errorLineNumber()
                   << ", column:" << result.errorColumnNumber()
                   << ":" << result.errorMessage();
        return false;
    }
}

QString RGBScript::fileName() const
{
    return m_fileName;
}

bool RGBScript::evaluate()
{
    // Create the script engine when it's first needed
    if (s_engine == NULL)
        s_engine = new QScriptEngine(QCoreApplication::instance());
    Q_ASSERT(s_engine != NULL);

    m_rgbMap = QScriptValue();
    m_rgbMapStepCount = QScriptValue();
    m_apiVersion = 0;
    m_script = s_engine->evaluate(m_contents, m_fileName);
    if (s_engine->hasUncaughtException() == true)
    {
        QString msg("%1: %2");
        qWarning() << msg.arg(m_fileName).arg(s_engine->uncaughtException().toString());
        foreach (QString s, s_engine->uncaughtExceptionBacktrace())
            qDebug() << s;
        return false;
    }
    else
    {
        m_rgbMap = m_script.property("rgbMap");
        if (m_rgbMap.isFunction() == false)
        {
            qWarning() << m_fileName << "is missing the rgbMap() function!";
            return false;
        }

        m_rgbMapStepCount = m_script.property("rgbMapStepCount");
        if (m_rgbMapStepCount.isFunction() == false)
        {
            qWarning() << m_fileName << "is missing the rgbMapStepCount() function!";
            return false;
        }

        m_apiVersion = m_script.property("apiVersion").toInteger();
        if (m_apiVersion > 0)
        {
            return true;
        }
        else
        {
            qWarning() << m_fileName << "has an invalid apiVersion:" << m_apiVersion;
            return false;
        }
    }
}

/****************************************************************************
 * Script API
 ****************************************************************************/

int RGBScript::rgbMapStepCount(const QSize& size)
{
    if (m_rgbMapStepCount.isValid() == false)
        return -1;

    QScriptValueList args;
    args << size.width() << size.height();
    QScriptValue value = m_rgbMapStepCount.call(QScriptValue(), args);
    if (value.isNumber() == true)
        return value.toInteger();
    else
        return -1;
}

RGBMap RGBScript::rgbMap(const QSize& size, uint rgb, int step)
{
    RGBMap map;

    if (m_rgbMap.isValid() == false)
        return map;

    QScriptValueList args;
    args << size.width() << size.height() << rgb << step;
    QScriptValue yarray = m_rgbMap.call(QScriptValue(), args);
    if (yarray.isArray() == true)
    {
        int ylen = yarray.property("length").toInteger();
        map = RGBMap(ylen);
        for (int y = 0; y < ylen && y < size.height(); y++)
        {
            QScriptValue xarray = yarray.property(QString::number(y));
            int xlen = xarray.property("length").toInteger();
            map[y].resize(xlen);
            for (int x = 0; x < xlen && x < size.width(); x++)
            {
                QScriptValue yx = xarray.property(QString::number(x));
                map[y][x] = yx.toInteger();
            }
        }
    }
    else
    {
        qWarning() << "Returned value is not an array within an array!";
    }

    return map;
}

QString RGBScript::name() const
{
    QScriptValue name = m_script.property("name");
    if (name.isValid() == true)
        return name.toString();
    else
        return QString();
}

QString RGBScript::author() const
{
    QScriptValue author = m_script.property("author");
    if (author.isValid() == true)
        return author.toString();
    else
        return QString();
}

int RGBScript::apiVersion() const
{
    return m_apiVersion;
}

RGBAlgorithm::Type RGBScript::type() const
{
    return RGBAlgorithm::Script;
}

bool RGBScript::saveXML(QDomDocument* doc, QDomElement* mtx_root) const
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(mtx_root != NULL);

    QDomElement root = doc->createElement(KXMLQLCRGBAlgorithm);
    root.setAttribute(KXMLQLCRGBAlgorithmType, KXMLQLCRGBScript);
    mtx_root->appendChild(root);

    if (apiVersion() > 0 && name().isEmpty() == false)
    {
        QDomText text = doc->createTextNode(name());
        root.appendChild(text);
        return true;
    }
    else
    {
        return false;
    }
}

/****************************************************************************
 * System & User Scripts
 ****************************************************************************/

RGBScript RGBScript::script(const QString& name)
{
    QListIterator <RGBScript> it(scripts());
    while (it.hasNext() == true)
    {
        RGBScript script(it.next());
        if (script.name() == name)
            return script;
    }

    return RGBScript();
}

QStringList RGBScript::scriptNames()
{
    QStringList names;

    QListIterator <RGBScript> it(scripts());
    while (it.hasNext() == true)
        names << it.next().name();

    return names;
}

QList <RGBScript> RGBScript::scripts()
{
    QList <RGBScript> list;
    list << scripts(userScriptDirectory());
    list << scripts(systemScriptDirectory());
    list << scripts(customScriptDirectory());
    return list;
}

QList <RGBScript> RGBScript::scripts(const QDir& dir)
{
    QList <RGBScript> list;
    foreach (QString file, dir.entryList())
    {
        RGBScript script;
        if (script.load(dir, file) == true && list.contains(script) == false)
            list << script;
    }

    return list;
}

QDir RGBScript::systemScriptDirectory()
{
    QDir dir;
#ifdef __APPLE__
    dir.setPath(QString("%1/../%2").arg(QCoreApplication::applicationDirPath())
                                   .arg(RGBSCRIPTDIR));
#else
    dir.setPath(RGBSCRIPTDIR);
#endif

    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*.js"));

    return dir;
}

QDir RGBScript::userScriptDirectory()
{
    QDir dir;

#ifdef Q_WS_X11
    // If the current user is root, return the system profile dir.
    // Otherwise return the user's home dir.
    if (geteuid() == 0)
        dir = QDir(RGBSCRIPTDIR);
    else
        dir.setPath(QString("%1/%2").arg(getenv("HOME")).arg(USERRGBSCRIPTDIR));
#elif __APPLE__
    /* User's input profile directory on OSX */
    dir.setPath(QString("%1/%2").arg(getenv("HOME")).arg(USERRGBSCRIPTDIR));
#else
    /* User's input profile directory on Windows */
    LPTSTR home = (LPTSTR) malloc(256 * sizeof(TCHAR));
    GetEnvironmentVariable(TEXT("UserProfile"), home, 256);
    dir.setPath(QString("%1/%2")
                    .arg(QString::fromUtf16(reinterpret_cast<ushort*> (home)))
                    .arg(USERRGBSCRIPTDIR));
    free(home);
#endif

    /* Ensure that the selected profile directory exists */
    if (dir.exists() == false)
        dir.mkpath(".");

    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*.js"));

    return dir;
}

void RGBScript::setCustomScriptDirectory(const QString& path)
{
    s_customScriptDirectory.setPath(path);
}

QDir RGBScript::customScriptDirectory()
{
    return s_customScriptDirectory;
}
