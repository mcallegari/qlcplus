/*
  Q Light Controller Plus
  rgbscriptv4.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QJSEngine>
#include <QMutex>
#include <QDebug>
#include <QFile>

#include "rgbscriptv4.h"

#include "rgbscriptscache.h"
#include "qlcconfig.h"
#include "qlcfile.h"

QJSEngine* RGBScript::s_engine = NULL;
QMutex* RGBScript::s_engineMutex = NULL;

/****************************************************************************
 * Initialization
 ****************************************************************************/

RGBScript::RGBScript(Doc * doc)
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
    // Create the script engine when it's first needed
    initEngine();

    QMutexLocker engineLocker(s_engineMutex);

    m_contents.clear();
    m_script = QJSValue();
    m_rgbMap = QJSValue();
    m_rgbMapStepCount = QJSValue();
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

    return evaluate();
}

QString RGBScript::fileName() const
{
    return m_fileName;
}

bool RGBScript::evaluate()
{
    QMutexLocker engineLocker(s_engineMutex);

    m_rgbMap = QJSValue();
    m_rgbMapStepCount = QJSValue();
    m_apiVersion = 0;

    m_script = s_engine->evaluate(m_contents, m_fileName);
    if (m_script.isError())
    {
        QString msg("%1: Uncaught exception at line %2. Error: %3");
        qWarning() << msg.arg(m_fileName)
                         .arg(m_script.property("lineNumber").toInt())
                         .arg(m_script.toString());
        qDebug() << "Stack: " << m_script.property("stack").toString();
        return false;
    }

    m_rgbMap = m_script.property("rgbMap");
    if (m_rgbMap.isCallable() == false)
    {
        qWarning() << m_fileName << "is missing the rgbMap() function!";
        return false;
    }

    m_rgbMapStepCount = m_script.property("rgbMapStepCount");
    if (m_rgbMapStepCount.isCallable() == false)
    {
        qWarning() << m_fileName << "is missing the rgbMapStepCount() function!";
        return false;
    }

    m_apiVersion = m_script.property("apiVersion").toInt();
    if (m_apiVersion > 0)
    {
        if (m_apiVersion == 2)
            return loadProperties();
        return true;
    }
    else
    {
        qWarning() << m_fileName << "has an invalid apiVersion:" << m_apiVersion;
        return false;
    }
}

void RGBScript::initEngine()
{
    if (s_engineMutex == NULL)
    {
        s_engineMutex = new QMutex(QMutex::Recursive);
        s_engine = new QJSEngine();
    }
    Q_ASSERT(s_engineMutex != NULL);
    Q_ASSERT(s_engine != NULL);
}

/****************************************************************************
 * Script API
 ****************************************************************************/

int RGBScript::rgbMapStepCount(const QSize& size)
{
    QMutexLocker engineLocker(s_engineMutex);

    if (m_rgbMapStepCount.isCallable() == false)
        return -1;

    QJSValueList args;
    args << size.width() << size.height();
    QJSValue value = m_rgbMapStepCount.call(args);
    int ret = value.isNumber() ? value.toInt() : -1;
    return ret;
}

RGBMap RGBScript::rgbMap(const QSize& size, uint rgb, int step)
{
    RGBMap map;

    QMutexLocker engineLocker(s_engineMutex);

    if (m_rgbMap.isUndefined() == true)
        return map;

    QJSValueList args;
    args << size.width() << size.height() << rgb << step;
    QJSValue yarray = m_rgbMap.call(args);
    if (yarray.isArray() == true)
    {
        int ylen = yarray.property("length").toInt();
        map = RGBMap(ylen);
        for (int y = 0; y < ylen && y < size.height(); y++)
        {
            QJSValue xarray = yarray.property(QString::number(y));
            int xlen = xarray.property("length").toInt();
            map[y].resize(xlen);
            for (int x = 0; x < xlen && x < size.width(); x++)
            {
                QJSValue yx = xarray.property(QString::number(x));
                map[y][x] = yx.toInt();
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

    QJSValue name = m_script.property("name");
    QString ret = name.isUndefined() ? QString() : name.toString();
    return ret;
}

QString RGBScript::author() const
{
    QMutexLocker engineLocker(s_engineMutex);

    QJSValue author = m_script.property("author");
    QString ret = author.isUndefined() ? QString() : author.toString();
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

int RGBScript::acceptColors() const
{
    QMutexLocker engineLocker(s_engineMutex);

    QJSValue accColors = m_script.property("acceptColors");
    if (!accColors.isUndefined())
        return accColors.toInt();
    // if no property is provided, let's assume the script
    // will accept both start and end colors
    return 2;
}

bool RGBScript::loadXML(QXmlStreamReader &root)
{
    Q_UNUSED(root)

    return false;
}

bool RGBScript::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    if (apiVersion() > 0 && name().isEmpty() == false)
    {
        doc->writeStartElement(KXMLQLCRGBAlgorithm);
        doc->writeAttribute(KXMLQLCRGBAlgorithmType, KXMLQLCRGBScript);
        doc->writeCharacters(name());
        doc->writeEndElement();
        return true;
    }
    else
    {
        return false;
    }
}

/************************************************************************
 * Capabilities
 ************************************************************************/

QList<RGBScriptProperty> RGBScript::properties()
{
    return m_properties;
}

QHash<QString, QString> RGBScript::propertiesAsStrings()
{
    QMutexLocker engineLocker(s_engineMutex);

    QHash<QString, QString> properties;
    foreach(RGBScriptProperty cap, m_properties)
    {
        QJSValue readMethod = m_script.property(cap.m_readMethod);
        if (readMethod.isCallable())
        {
            QJSValueList args;
            QJSValue value = readMethod.call(args);
            if (!value.isUndefined())
                properties.insert(cap.m_name, value.toString());
        }
    }
    return properties;
}

bool RGBScript::setProperty(QString propertyName, QString value)
{
    QMutexLocker engineLocker(s_engineMutex);

    foreach(RGBScriptProperty cap, m_properties)
    {
        if (cap.m_name == propertyName)
        {
            QJSValue writeMethod = m_script.property(cap.m_writeMethod);
            if (writeMethod.isCallable() == false)
            {
                qWarning() << name() << "doesn't have a write function for" << propertyName;
                return false;
            }
            QJSValueList args;
            args << value;
            writeMethod.call(args);
            return true;
        }
    }
    return false;
}

QString RGBScript::property(QString propertyName)
{
    QMutexLocker engineLocker(s_engineMutex);

    foreach(RGBScriptProperty cap, m_properties)
    {
        if (cap.m_name == propertyName)
        {
            QJSValue readMethod = m_script.property(cap.m_readMethod);
            if (readMethod.isCallable() == false)
            {
                qWarning() << name() << "doesn't have a read function for" << propertyName;
                return QString();
            }
            QJSValueList args;
            QJSValue value = readMethod.call(args);
            if (!value.isUndefined())
                return value.toString();
            else
                return QString();
        }
    }
    return QString();
}

bool RGBScript::loadProperties()
{
    QMutexLocker engineLocker(s_engineMutex);

    QJSValue svCaps = m_script.property("properties");
    if (svCaps.isArray() == false)
    {
        qWarning() << m_fileName << "properties is not an array!";
        return false;
    }
    QVariant varCaps = svCaps.toVariant();
    if (varCaps.isValid() == false)
    {
        qWarning() << m_fileName << "has invalid properties!";
        return false;
    }

    m_properties.clear();

    QStringList slCaps = varCaps.toStringList();
    foreach (QString cap, slCaps)
    {
        RGBScriptProperty newCap;

        QStringList propsList = cap.split("|");
        foreach(QString prop, propsList)
        {
            QStringList keyValue = prop.split(":");
            if (keyValue.length() < 2)
            {
                qWarning() << prop << ": malformed property. Please fix it.";
                continue;
            }
            QString key = keyValue.at(0).simplified();
            QString value = keyValue.at(1);
            if (key == "name")
            {
                newCap.m_name = value;
            }
            else if (key == "type")
            {
                if (value == "list") newCap.m_type = RGBScriptProperty::List;
                else if (value == "integer") newCap.m_type = RGBScriptProperty::Integer;
                else if (value == "range") newCap.m_type = RGBScriptProperty::Range;
                else if (value == "string") newCap.m_type = RGBScriptProperty::String;
            }
            else if (key == "display")
            {
                newCap.m_displayName = value.simplified();
            }
            else if (key == "values")
            {
                QStringList values = value.split(",");
                switch(newCap.m_type)
                {
                    case RGBScriptProperty::List:
                        newCap.m_listValues = values;
                    break;
                    case RGBScriptProperty::Range:
                    {
                        if (values.length() < 2)
                        {
                            qWarning() << value << ": malformed property. A range should be defined as 'min,max'. Please fix it.";
                        }
                        else
                        {
                            newCap.m_rangeMinValue = values.at(0).toInt();
                            newCap.m_rangeMaxValue = values.at(1).toInt();
                        }
                    }
                    break;
                    default:
                        qWarning() << value << ": values cannot be applied before the 'type' property or on type:integer and type:string";
                    break;
                }
            }
            else if (key == "write")
            {
                newCap.m_writeMethod = value.simplified();
            }
            else if (key == "read")
            {
                newCap.m_readMethod = value.simplified();
            }
            else
            {
                qWarning() << value << ": unknown property!";
            }
        }

        if (newCap.m_name.isEmpty() == false &&
            newCap.m_type != RGBScriptProperty::None)
                m_properties.append(newCap);
    }

    return true;
}
