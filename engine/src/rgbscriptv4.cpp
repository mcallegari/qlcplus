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
#include <QThread>
#include <QDebug>
#include <QFile>

// cppcheck-suppress missingIncludeSystem
#include <QCoreApplication>
// cppcheck-suppress missingIncludeSystem
#include <QSemaphore>

#include "rgbscriptv4.h"

#include "rgbscriptscache.h"
#include "qlcconfig.h"
#include "qlcfile.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

JSThread* RGBScript::s_jsThread = NULL;

class JSThread: public QThread
{
public:
    QJSEngine *engine;
    QSemaphore ready;
    void run()
    {
        engine = new QJSEngine();
        ready.release(1);
        exec();
        delete engine;
    }
};


RGBScript::RGBScript(Doc *doc)
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
    foreach (RGBScriptProperty cap, s.m_properties)
    {
        setProperty(cap.m_name, s.property(cap.m_name));
    }
}

RGBScript::~RGBScript()
{
}

RGBScript &RGBScript::operator=(const RGBScript &s)
{
    if (this != &s)
    {
        m_fileName = s.m_fileName;
        m_contents = s.m_contents;
        m_apiVersion = s.m_apiVersion;
        evaluate();
        foreach (RGBScriptProperty cap, s.m_properties)
        {
            setProperty(cap.m_name, s.property(cap.m_name));
        }
    }

    return *this;
}

bool RGBScript::operator==(const RGBScript& s) const
{
    return this->fileName().isEmpty() == false && this->fileName() == s.fileName();
}

RGBAlgorithm* RGBScript::clone() const
{
    RGBScript *script = new RGBScript(*this);
    return static_cast<RGBAlgorithm*> (script);
}

/****************************************************************************
 * Load & Evaluation
 ****************************************************************************/

bool RGBScript::load(const QString& fileName)
{
    // Create the script engine when it's first needed
    initEngine();

    {
        m_contents.clear();
        m_script = QJSValue();
        m_rgbMap = QJSValue();
        m_rgbMapStepCount = QJSValue();
        m_rgbMapSetColors = QJSValue();
        m_apiVersion = 0;
    }

    m_fileName = fileName;
    QFile file(m_fileName);
    if (file.open(QIODevice::ReadOnly) == false)
    {
        qWarning() << "Unable to load RGB script" << m_fileName;
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

void RGBScript::initEngine()
{
    if (s_jsThread == NULL)
    {
        s_jsThread = new JSThread();
        s_jsThread->start();
        // cppcheck-suppress unknownMacro
        qAddPostRoutine(RGBScript::cleanupEngine);
        s_jsThread->ready.acquire(1);
    }
    Q_ASSERT(s_jsThread->engine != NULL);
}

void RGBScript::cleanupEngine()
{
    s_jsThread->exit();
    s_jsThread->wait();
    delete s_jsThread;
    s_jsThread = NULL;
}


bool RGBScript::evaluate()
{
    if (s_jsThread != NULL && QThread::currentThread() != s_jsThread)
    {
        bool retVal;
        QMetaObject::invokeMethod(s_jsThread->engine, [this]{ return evaluate();}, Qt::BlockingQueuedConnection, &retVal);
        return retVal;
    }

    m_rgbMap = QJSValue();
    m_rgbMapStepCount = QJSValue();
    m_rgbMapSetColors = QJSValue();
    m_apiVersion = 0;

    if (m_fileName.isEmpty() || m_contents.isEmpty())
    {
        qWarning() << m_fileName << ": Script filename or content is empty, cannot parse";
        return false;
    }

    initEngine();

    m_script = s_jsThread->engine->evaluate(m_contents, m_fileName);
    if (m_script.isError())
    {
        displayError(m_script, m_fileName);
        return false;
    }

    m_rgbMap = m_script.property(QStringLiteral("rgbMap"));
    if (m_rgbMap.isCallable() == false)
    {
        qWarning() << m_fileName << "is missing the rgbMap() function!";
        return false;
    }

    m_rgbMapStepCount = m_script.property(QStringLiteral("rgbMapStepCount"));
    if (m_rgbMapStepCount.isCallable() == false)
    {
        qWarning() << m_fileName << "is missing the rgbMapStepCount() function!";
        return false;
    }

    m_apiVersion = m_script.property("apiVersion").toInt();
    if (m_apiVersion > 0)
    {
        if (m_apiVersion >= 3)
        {
            m_rgbMapSetColors = m_script.property(QStringLiteral("rgbMapSetColors"));
            if (m_rgbMapSetColors.isCallable() == false)
            {
                qWarning() << m_fileName << "is missing the rgbMapSetColors() function!";
                return false;
            }
        }
        if (m_apiVersion >= 2)
            return loadProperties();
        return true;
    }
    else
    {
        qWarning() << m_fileName << "has an invalid apiVersion:" << m_apiVersion;
        return false;
    }
}

void RGBScript::displayError(QJSValue e, const QString& fileName)
{
    if (e.isError())
    {
        QString msg("%1: Exception at line %2. Error: %3");
        qWarning() << msg.arg(fileName)
                         .arg(e.property("lineNumber").toInt())
                         .arg(e.toString());
        qDebug() << "Stack: " << e.property("stack").toString();
    }
}

/****************************************************************************
 * Script API
 ****************************************************************************/

int RGBScript::rgbMapStepCount(const QSize& size)
{
    if (s_jsThread != NULL && QThread::currentThread() != s_jsThread)
    {
        int retVal;
        QMetaObject::invokeMethod(s_jsThread->engine, [this, size]{ return rgbMapStepCount(size);}, Qt::BlockingQueuedConnection, &retVal);
        return retVal;
    }

    if (m_rgbMapStepCount.isCallable() == false)
        return -1;

    QJSValueList args;
    args << size.width() << size.height();
    QJSValue value = m_rgbMapStepCount.call(args);
    if (value.isError())
    {
        displayError(value, m_fileName);
        return -1;
    } 
    else 
    {
        int ret = value.isNumber() ? value.toInt() : -1;
        return ret;
    }
}

void RGBScript::rgbMapSetColors(const QVector<uint> &colors)
{
    if (s_jsThread != NULL && QThread::currentThread() != s_jsThread)
    {
        QMetaObject::invokeMethod(s_jsThread->engine, [this, colors]{ return rgbMapSetColors(colors);}, Qt::QueuedConnection);
        return;
    }

    if (m_apiVersion <= 2)
        return;

    if (m_rgbMap.isUndefined() == true)
        return;

    if (m_rgbMapSetColors.isCallable() == false)
        return;

    int accColors = acceptColors();
    int rawColorCount = colors.count();

    QJSValue jsRawColors = s_jsThread->engine->newArray(accColors);
    for (int i = 0; i < rawColorCount && i < accColors; i++)
        jsRawColors.setProperty(i, QJSValue(colors.at(i)));

    QJSValueList args;
    args << jsRawColors;

    QJSValue value = m_rgbMapSetColors.call(args);
    if (value.isError())
        displayError(value, m_fileName);
}

QVector<uint> RGBScript::rgbMapGetColors()
{
    if (s_jsThread != NULL && QThread::currentThread() != s_jsThread)
    {
        QVector<uint> retVal;
        QMetaObject::invokeMethod(s_jsThread->engine, [this]{ return rgbMapGetColors();}, Qt::BlockingQueuedConnection, &retVal);
        return retVal;
    }

    QVector<uint> colArray;

    if (m_rgbMap.isUndefined() == true)
        return colArray;

    QJSValue colors = m_rgbMapGetColors.call();
    if (!colors.isError() && colors.isArray())
    {
        QVariantList arr = colors.toVariant().toList();
        foreach (QVariant color, arr)
            colArray.append(color.toUInt());
    }

    return colArray;
}

void RGBScript::rgbMap(const QSize& size, uint rgb, int step, RGBMap &map)
{
    if (s_jsThread != NULL && QThread::currentThread() != s_jsThread)
    {
        QMetaObject::invokeMethod(s_jsThread->engine, [this, size, rgb, step, &map]{ rgbMap(size, rgb, step, map);}, Qt::BlockingQueuedConnection);
        return;
    }

    if (m_rgbMap.isUndefined() == true)
        return;

    // Call the rgbMap function
    QJSValueList args;
    args << size.width() << size.height() << rgb << step;

    QJSValue yarray(m_rgbMap.call(args));
    if (yarray.isError())
        displayError(yarray, m_fileName);

    // Check the matrix to be a valid matrix
    if (yarray.isArray())
    {
        QVariantList yvArray = yarray.toVariant().toList();
        int ylen = yvArray.length();
        map.resize(ylen);

        for (int y = 0; y < ylen && y < size.height(); y++)
        {
            QVariantList xvArray = yvArray.at(y).toList();
            int xlen = xvArray.length();
            map[y].resize(xlen);

            for (int x = 0; x < xlen && x < size.width(); x++)
                map[y][x] = xvArray.at(x).toUInt();
        }
    }
    else
    {
        qWarning() << "Returned value is not an array within an array!";
    }
}

QString RGBScript::name() const
{
    if (s_jsThread != NULL && QThread::currentThread() != s_jsThread)
    {
        QString retVal;
        QMetaObject::invokeMethod(s_jsThread->engine, [this]{ return name();}, Qt::BlockingQueuedConnection, &retVal);
        return retVal;
    }

    QJSValue name = m_script.property(QStringLiteral("name"));
    QString ret = name.isUndefined() ? QString() : name.toString();
    return ret;
}

QString RGBScript::author() const
{
    if (s_jsThread != NULL && QThread::currentThread() != s_jsThread)
    {
        QString retVal;
        QMetaObject::invokeMethod(s_jsThread->engine, [this]{ return author();}, Qt::BlockingQueuedConnection, &retVal);
        return retVal;
    }

    QJSValue author = m_script.property(QStringLiteral("author"));
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
    if (s_jsThread != NULL && QThread::currentThread() != s_jsThread)
    {
        int retVal;
        QMetaObject::invokeMethod(s_jsThread->engine, [this]{ return acceptColors();}, Qt::BlockingQueuedConnection, &retVal);
        return retVal;
    }

    QJSValue accColors = m_script.property(QStringLiteral("acceptColors"));
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
    if (s_jsThread != NULL && QThread::currentThread() != s_jsThread)
    {
        QHash<QString, QString> retVal;
        QMetaObject::invokeMethod(s_jsThread->engine, [this]{ return propertiesAsStrings();}, Qt::BlockingQueuedConnection, &retVal);
        return retVal;
    }

    QHash<QString, QString> properties;
    foreach (RGBScriptProperty cap, m_properties)
    {
        QJSValue readMethod = m_script.property(cap.m_readMethod);
        if (readMethod.isCallable())
        {
            QJSValueList args;
            QJSValue value = readMethod.call(args);
            if (value.isError())
                displayError(value, m_fileName);
            else if (!value.isUndefined())
                properties.insert(cap.m_name, value.toString());
        }
    }
    return properties;
}

bool RGBScript::setProperty(QString propertyName, QString value)
{
    if (s_jsThread != NULL && QThread::currentThread() != s_jsThread)
    {
        bool retVal;
        QMetaObject::invokeMethod(s_jsThread->engine, [this, propertyName, value]{ return setProperty(propertyName, value);}, Qt::BlockingQueuedConnection, &retVal);
        return retVal;
    }

    foreach (RGBScriptProperty cap, m_properties)
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
            QJSValue written = writeMethod.call(args);
            if (written.isError())
            {
                displayError(written, m_fileName);
                return false;
            } 
            else 
            {
                return true;
            }
        }
    }
    return false;
}

QString RGBScript::property(QString propertyName) const
{
    if (s_jsThread != NULL && QThread::currentThread() != s_jsThread)
    {
        QString retVal;
        QMetaObject::invokeMethod(s_jsThread->engine, [this, propertyName]{ return property(propertyName);}, Qt::BlockingQueuedConnection, &retVal);
        return retVal;
    }

    foreach (RGBScriptProperty cap, m_properties)
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
            if (value.isError())
            {
                displayError(value, m_fileName);
                return QString();
            } 
            else if (!value.isUndefined())
            {
                return value.toString();
            }
            else
            {
                return QString();
            }
        }
    }
    return QString();
}

bool RGBScript::loadProperties()
{
    QJSValue svCaps = m_script.property(QStringLiteral("properties"));
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

        QStringList propsList = cap.split('|');
        foreach (QString prop, propsList)
        {
            QStringList keyValue = prop.split(':');
            if (keyValue.length() < 2)
            {
                qWarning() << prop << ": malformed property. Please fix it.";
                continue;
            }
            QString key = keyValue.at(0).simplified();
            QString value = keyValue.at(1);
            if (key == QStringLiteral("name"))
            {
                newCap.m_name = value;
            }
            else if (key == QStringLiteral("type"))
            {
                if (value == "list") newCap.m_type = RGBScriptProperty::List;
                else if (value == "float") newCap.m_type = RGBScriptProperty::Float;
                else if (value == "range") newCap.m_type = RGBScriptProperty::Range;
                else if (value == "string") newCap.m_type = RGBScriptProperty::String;
            }
            else if (key == QStringLiteral("display"))
            {
                newCap.m_displayName = value.simplified();
            }
            else if (key == QStringLiteral("values"))
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
            else if (key == QStringLiteral("write"))
            {
                newCap.m_writeMethod = value.simplified();
            }
            else if (key == QStringLiteral("read"))
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
