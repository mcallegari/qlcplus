/*
  Q Light Controller
  qlcfile.cpp

  Copyright (C) Heikki Junnila

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

#include <QFile>
#include <QtXml>

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

#define KXMLQLCplusNamespace "http://qlcplus.sourceforge.net/"

QDomDocument QLCFile::readXML(const QString& path)
{
    if (path.isEmpty() == true)
    {
        qWarning() << Q_FUNC_INFO
                   << "Empty path given. Not attempting to load file.";
        return QDomDocument();
    }

    QDomDocument doc;
    QFile file(path);
    if (file.open(QIODevice::ReadOnly) == true)
    {
        QString msg;
        int line = 0;
        int col = 0;
        if (doc.setContent(&file, false, &msg, &line, &col) == false)
        {
            qWarning() << Q_FUNC_INFO << "Error loading file" << path
                       << ":" << msg << ", line:" << line << ", col:" << col;
        }
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "Unable to open file:" << path;
    }

    file.close();

    return doc;
}

QDomDocument QLCFile::getXMLHeader(const QString& content, const QString& author)
{
    if (content.isEmpty() == true)
        return QDomDocument();

    QDomImplementation dom;
    QDomDocument doc(dom.createDocumentType(content, QString(), QString()));

    QDomProcessingInstruction instr = doc.createProcessingInstruction( 
        "xml", "version='1.0' encoding='UTF-8'");

    doc.appendChild(instr);

    QDomElement root;
    QDomElement tag;
    QDomElement subtag;
    QDomText text;

    root = doc.createElement(content);
    root.setAttribute("xmlns", KXMLQLCplusNamespace + content);

    doc.appendChild(root);

    /* Creator tag */
    tag = doc.createElement(KXMLQLCCreator);
    root.appendChild(tag);

    /* Creator name */
    subtag = doc.createElement(KXMLQLCCreatorName);
    tag.appendChild(subtag);
    text = doc.createTextNode(APPNAME);
    subtag.appendChild(text);

    /* Creator version */
    subtag = doc.createElement(KXMLQLCCreatorVersion);
    tag.appendChild(subtag);
    text = doc.createTextNode(QString(APPVERSION));
    subtag.appendChild(text);

    /* Author */
    subtag = doc.createElement(KXMLQLCCreatorAuthor);
    tag.appendChild(subtag);
    if (author.isEmpty() == true)
        text = doc.createTextNode(currentUserName());
    else
        text = doc.createTextNode(author);
    subtag.appendChild(text);

    return doc;
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
        return QString::fromUtf16((ushort*) name);
    else
        return QString("Unknown windows user");
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
}

bool QLCFile::isRaspberry()
{
#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
    QFile cpuInfoFile("/proc/cpuinfo");
    if (cpuInfoFile.exists() == true)
    {
        cpuInfoFile.open(QFile::ReadOnly);
        QString content = QLatin1String(cpuInfoFile.readAll());
        cpuInfoFile.close();
        if (content.contains("BCM2708") || content.contains("BCM2709"))
            return true;
    }
    return false;
#else
    return false;
#endif
}

QDir QLCFile::systemDirectory(QString path, QString extension)
{
    QDir dir;
#if defined(__APPLE__) || defined(Q_OS_MAC)
         dir.setPath(QString("%1/../%2").arg(QCoreApplication::applicationDirPath())
                    .arg(path));
#elif defined(WIN32) || defined(Q_OS_WIN)
        dir.setPath(QString("%1%2%3").arg(QCoreApplication::applicationDirPath())
                    .arg(QDir::separator())
                    .arg(path));
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
    if (geteuid() == 0 && QLCFile::isRaspberry() == false)
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
                    .arg(QString::fromUtf16(reinterpret_cast<ushort*> (home)))
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
