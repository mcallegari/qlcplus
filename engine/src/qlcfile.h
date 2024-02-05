/*
  Q Light Controller Plus
  qlcfile.h

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

#ifndef QLCFILE_H
#define QLCFILE_H

#include <QFile>
#include <QDir>

class QXmlStreamReader;
class QXmlStreamWriter;
#ifdef QT_XML_LIB
class QDomDocument;
class QDomElement;
#endif
class QString;

/** @addtogroup engine Engine
 * @{
 */

#define KXMLQLCplusNamespace "http://www.qlcplus.org/"

// File extensions
#define KExtFixture          ".qxf"  // 'Q'LC+ 'X'ml 'F'ixture
#define KExtFixtureList      ".qxfl" // 'Q'LC+ 'X'ml 'F'ixture 'L'ist
#define KExtWorkspace        ".qxw"  // 'Q'LC+ 'X'ml 'W'orkspace
#define KExtInputProfile     ".qxi"  // 'Q'LC+ 'X'ml 'I'nput profile
#define KExtModifierTemplate ".qxmt" // 'Q'LC+ 'X'ml 'M'odifier 'T'emplate
#define KExtColorFilters     ".qxcf" // 'Q'LC+ 'X'ml 'C'olor 'F'ilters

#if defined(WIN32) || defined(Q_OS_WIN)
#   define KExtPlugin    ".dll" // Dynamic-Link Library
#elif defined(__APPLE__) || defined(Q_OS_MAC)
#   define KExtPlugin  ".dylib" // DYnamic LIBrary
#else
#   define KExtPlugin    ".so"  // Shared Object
#endif

// Generic XML tags common for all documents
#define KXMLQLCCreator          QString("Creator")
#define KXMLQLCCreatorName      QString("Name")
#define KXMLQLCCreatorVersion   QString("Version")
#define KXMLQLCCreatorAuthor    QString("Author")

// share fixture list tag
#define KXMLQLCFixturesList QString("FixtureList")

// True and false
#define KXMLQLCTrue "True"
#define KXMLQLCFalse "False"

class QLCFile
{
public:
    /**
     * Request a QXmlStreamReader for an XML file
     *
     * @param path Path to the file to read
     * @return QXmlStreamReader (unitialized if not successful)
     */
    static QXmlStreamReader *getXMLReader(const QString& path);

    /**
     * Release an existing instance of an XML reader, by closing
     * the device file, and freeing the resources
     */
    static void releaseXMLReader(QXmlStreamReader *reader);

    /**
     * Write a common XML header on the given document
     *
     * @param xml The instance of a XML writer
     * @param content The content type (Settings, Workspace)
     * @param author The file's author (overridden by current user name if empty)
     * @return true on success, false on failure
     */
    static bool writeXMLHeader(QXmlStreamWriter *xml, const QString& content, const QString& author = QString());

    /**
     * Get a string that gives a textual description for the given file
     * error code.
     *
     * @param error The error code to get description for
     * @return Short description of the given file error
     */
    static QString errorString(QFile::FileError error);

    /**
     * Get the current user name (to be written in a file's "Author" tags).
     * On unix, the user name is whatever's been written to the user's passwd
     * entry.
     *
     * @return Current user name.
     */
    static QString currentUserName();

    /**
     * Method called just once to set the m_hasWindowManager flag
     */
    static void setHasWindowManager(bool enable);

    /**
     * Return if the current platform provides a window manager
     */
    static bool hasWindowManager();

    /**
     * @brief systemDirectory returns a system dependant QDir based
     *        on a QLC+ hardcoded path and a QLC+ hardcoded extension
     * @param path
     * @param extension
     * @return
     */
    static QDir systemDirectory(QString path, QString extension = QString());

    /**
     * @brief systemDirectory returns a system dependant QDir based
     *        on a QLC+ hardcoded path and a list of QLC+ hardcoded extensions
     * @param path
     * @param extension
     * @return
     */
    static QDir userDirectory(QString path, QString fallBackPath, QStringList extensions);

    /** @brief Return a OS dependent prefix used for local file URLs.
      *        Linux and macOS needs "file://", while Windows needs "file:///" */
    static QString fileUrlPrefix();

    /**
     * @brief getQtVersion get the runtime Qt version as number. E.g. 50602
     */
    static quint32 getQtRuntimeVersion();

private:
    static bool m_hasWindowManager;
};

/** @} */

#endif
