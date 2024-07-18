/*
  Q Light Controller Plus
  qlcfile.cpp

  Copyright (C) Heikki Junnila
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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QCoreApplication>
#include <QFile>

#ifdef QT_XML_LIB
#   include <QtXml>
#else
#   include <QDebug>
#endif

#if defined(WIN32) || defined(Q_OS_WIN)
#   include <windows.h>
#   include <lmcons.h>
#else
#   include <sys/types.h>
#   include <unistd.h>
#   include <pwd.h>
#endif

#include "qlcconfig.h"
#include "qlcfile.h"

bool QLCFile::m_hasWindowManager = true;

QXmlStreamReader *QLCFile::getXMLReader(const QString &path)
{
    QXmlStreamReader *reader = NULL;

    if (path.isEmpty() == true)
    {
        qWarning() << Q_FUNC_INFO
                   << "Empty path given. Not attempting to load file.";
        return reader;
    }

    QFile *file = new QFile(path);
    if (file->open(QIODevice::ReadOnly | QFile::Text) == true)
    {
        reader = new QXmlStreamReader(file);
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "Unable to open file:" << path;
    }

    return reader;
}

void QLCFile::releaseXMLReader(QXmlStreamReader *reader)
{
    if (reader == NULL)
        return;

    if (reader->device() != NULL)
    {
        if (reader->device()->isOpen())
            reader->device()->close();
        delete reader->device();
    }
    delete reader;
}

bool QLCFile::writeXMLHeader(QXmlStreamWriter *xml, const QString &content, const QString &author)
{
    if (xml == NULL || xml->device() == NULL)
        return false;

    xml->writeStartDocument();
    xml->writeDTD(QString("<!DOCTYPE %1>").arg(content));

    xml->writeStartElement(content);
    xml->writeAttribute("xmlns", KXMLQLCplusNamespace + content);

    xml->writeStartElement(KXMLQLCCreator);
    xml->writeTextElement(KXMLQLCCreatorName, APPNAME);
    xml->writeTextElement(KXMLQLCCreatorVersion, APPVERSION);
    if (author.isEmpty())
        xml->writeTextElement(KXMLQLCCreatorAuthor, currentUserName());
    else
        xml->writeTextElement(KXMLQLCCreatorAuthor, author);
    xml->writeEndElement(); // close KXMLQLCCreator

    return true;
}

QString QLCFile::errorString(QFile::FileError error)
{
    switch (error)
    {
    case QFile::NoError:
        return QObject::tr("No error occurred.");
    case QFile::ReadError:
        return QObject::tr("An error occurred when reading from the file.");
    case QFile::WriteError:
        return QObject::tr("An error occurred when writing to the file.");
    case QFile::FatalError:
        return QObject::tr("A fatal error occurred.");
    case QFile::ResourceError:
        return QObject::tr("Resource error occurred.");
    case QFile::OpenError:
        return QObject::tr("The file could not be opened.");
    case QFile::AbortError:
        return QObject::tr("The operation was aborted.");
    case QFile::TimeOutError:
        return QObject::tr("A timeout occurred.");
    case QFile::UnspecifiedError:
        return QObject::tr("An unspecified error occurred.");
    case QFile::RemoveError:
        return QObject::tr("The file could not be removed.");
    case QFile::RenameError:
        return QObject::tr("The file could not be renamed.");
    case QFile::PositionError:
        return QObject::tr("The position in the file could not be changed.");
    case QFile::ResizeError:
        return QObject::tr("The file could not be resized.");
    case QFile::PermissionsError:
        return QObject::tr("The file could not be accessed.");
    case QFile::CopyError:
        return QObject::tr("The file could not be copied.");
    default:
        return QObject::tr("An unknown error occurred.");
    }
}

QString QLCFile::currentUserName()
{
#if defined(WIN32) || defined(Q_OS_WIN)
    DWORD length = UNLEN + 1;
    TCHAR name[length];
    if (GetUserName(name, &length))
        return QString::fromUtf16(reinterpret_cast<char16_t*>(name));
    else
        return QString("Unknown windows user");
#else
 #if defined(Q_OS_ANDROID)
    return QString(getenv("USER"));
 #else
    QString name;
    struct passwd* passwd = getpwuid(getuid());
    if (passwd == NULL)
        name.append(getenv("USER"));
    else
        name.append(passwd->pw_gecos);
    name.remove(",,,");
    return name;
 #endif
#endif
}

void QLCFile::setHasWindowManager(bool enable)
{
    m_hasWindowManager = enable;
}

bool QLCFile::hasWindowManager()
{
    return m_hasWindowManager;
}

QDir QLCFile::systemDirectory(QString path, QString extension)
{
    QDir dir;
#if defined(Q_OS_IOS)
    dir.setPath(QString("%1/%2").arg(QCoreApplication::applicationDirPath())
                                   .arg(path));
#elif defined(__APPLE__) || defined(Q_OS_MAC)
    dir.setPath(QString("%1/../%2").arg(QCoreApplication::applicationDirPath())
                                   .arg(path));
#elif defined(WIN32) || defined(Q_OS_WIN)
    dir.setPath(QString("%1%2%3").arg(QCoreApplication::applicationDirPath())
                                 .arg(QDir::separator())
                                 .arg(path));
#elif defined(Q_OS_ANDROID)
    dir.setPath(QString("assets:/%1").arg(path.remove(0, path.lastIndexOf("/") + 1)));
#else
    dir.setPath(path);
#endif

    dir.setFilter(QDir::Files);
    if (!extension.isEmpty())
        dir.setNameFilters(QStringList() << QString("*%1").arg(extension));

    return dir;
}

QDir QLCFile::userDirectory(QString path, QString fallBackPath, QStringList extensions)
{
    Q_UNUSED(fallBackPath)
    QDir dir;

#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
    // If the current user is root, return the system fixture dir.
    // Otherwise return a path under user's home dir.
    if (geteuid() == 0 && QLCFile::hasWindowManager())
        dir = QDir(fallBackPath);
    else
        dir.setPath(QString("%1/%2").arg(getenv("HOME")).arg(path));
#elif defined(__APPLE__) || defined (Q_OS_MAC)
    /* User's input profile directory on OSX */
    dir.setPath(QString("%1/%2").arg(getenv("HOME")).arg(path));
#else
    /* User's input profile directory on Windows */
    LPTSTR home = (LPTSTR) malloc(256 * sizeof(TCHAR));
    GetEnvironmentVariable(TEXT("UserProfile"), home, 256);
    dir.setPath(QString("%1/%2")
                    .arg(QString::fromUtf16(reinterpret_cast<char16_t*>(home)))
                    .arg(path));
    free(home);
#endif

    // Ensure the directory exists
    if (dir.exists() == false)
        dir.mkpath(".");

    dir.setFilter(QDir::Files);
    dir.setNameFilters(extensions);

    return dir;
}

QString QLCFile::fileUrlPrefix()
{
#if defined(WIN32) || defined(Q_OS_WIN)
    return QString("file:///");
#else
    return QString("file://");
#endif
}

quint32 QLCFile::getQtRuntimeVersion()
{
    QString ver(qVersion());
    if (ver.isEmpty())
        return 0;

    QStringList digits = ver.split(".");

    return digits.at(0).toInt() * 10000 + digits.at(1).toInt() * 100 + digits.at(2).toInt();
}
