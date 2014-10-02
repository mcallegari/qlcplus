/*
  Q Light Controller
  rgbscript.cpp

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
#include <QMutex>

#if defined(WIN32) || defined(Q_OS_WIN)
#   include <windows.h>
#else
#   include <unistd.h>
#endif

#include "rgbscript.h"
#include "qlcconfig.h"
#include "qlcfile.h"

QDir RGBScript::s_customScriptDirectory = QDir(QString(), QString("*.js"),
                                               QDir::Name | QDir::IgnoreCase,
                                               QDir::Files);

QScriptEngine* RGBScript::s_engine = NULL;
QMutex* RGBScript::s_engineMutex = NULL;

/****************************************************************************
 * Initialization
 ****************************************************************************/

RGBScript::RGBScript(const Doc * doc)
    : RGBAlgorithm(doc)
    , m_apiVersion(0)
{
}

RGBScript::RGBScript(const RGBScript& s)
    : RGBAlgorithm(s.doc())
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

    if (s_engineMutex == NULL)
        s_engineMutex = new QMutex(QMutex::Recursive);
    Q_ASSERT(s_engineMutex != NULL);

    QMutexLocker engineLocker(s_engineMutex);
    QScriptSyntaxCheckResult result = QScriptEngine::checkSyntax(m_contents);
    if (result.state() == QScriptSyntaxCheckResult::Valid)
        return evaluate();
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
    if (s_engineMutex == NULL)
        s_engineMutex = new QMutex(QMutex::Recursive);
    Q_ASSERT(s_engineMutex != NULL);

    m_rgbMap = QScriptValue();
    m_rgbMapStepCount = QScriptValue();
    m_apiVersion = 0;

    QMutexLocker engineLocker(s_engineMutex);
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
    QMutexLocker engineLocker(s_engineMutex);
    if (m_rgbMapStepCount.isValid() == false)
        return -1;

    QScriptValueList args;
    args << size.width() << size.height();
    QScriptValue value = m_rgbMapStepCount.call(QScriptValue(), args);
    int ret = value.isNumber() ? value.toInteger() : -1;
    return ret;
}

RGBMap RGBScript::rgbMap(const QSize& size, uint rgb, int step)
{
    RGBMap map;

    QMutexLocker engineLocker(s_engineMutex);
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
    QMutexLocker engineLocker(s_engineMutex);
    QScriptValue name = m_script.property("name");
    QString ret = name.isValid() ? name.toString() : QString();
    return ret;
}

QString RGBScript::author() const
{
    QMutexLocker engineLocker(s_engineMutex);
    QScriptValue author = m_script.property("author");
    QString ret = author.isValid() ? author.toString() : QString();
    return ret;
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

RGBScript RGBScript::script(const Doc * doc, const QString& name)
{
    QListIterator <RGBScript> it(scripts(doc));
    while (it.hasNext() == true)
    {
        RGBScript script(it.next());
        if (script.name() == name)
            return script;
    }

    return RGBScript(doc);
}

QStringList RGBScript::scriptNames(const Doc * doc)
{
    QStringList names;

    QListIterator <RGBScript> it(scripts(doc));
    while (it.hasNext() == true)
        names << it.next().name();

    return names;
}

QList <RGBScript> RGBScript::scripts(const Doc * doc)
{
    QList <RGBScript> list;
    list << scripts(doc, userScriptDirectory());
    list << scripts(doc, systemScriptDirectory());
    list << scripts(doc, customScriptDirectory());
    return list;
}

QList <RGBScript> RGBScript::scripts(const Doc * doc, const QDir& dir)
{
    QList <RGBScript> list;
    foreach (QString file, dir.entryList())
    {
        RGBScript script(doc);
        if (script.load(dir, file) == true && list.contains(script) == false)
            list << script;
    }

    return list;
}

QDir RGBScript::systemScriptDirectory()
{
    return QLCFile::systemDirectory(QString(RGBSCRIPTDIR), QString(".js"));
}

QDir RGBScript::userScriptDirectory()
{
    return QLCFile::userDirectory(QString(USERRGBSCRIPTDIR), QString(RGBSCRIPTDIR),
                                  QStringList() << QString("*%1").arg(".js"));
}

void RGBScript::setCustomScriptDirectory(const QString& path)
{
    s_customScriptDirectory.setPath(path);
}

QDir RGBScript::customScriptDirectory()
{
    return s_customScriptDirectory;
}
