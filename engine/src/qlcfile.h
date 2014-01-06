/*
  Q Light Controller
  qlcfile.h

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

#ifndef QLCFILE_H
#define QLCFILE_H

#include <QFile>

class QDomDocument;
class QDomElement;
class QString;

// File extensions
#define KExtFixture      ".qxf" // 'Q'LC+ 'X'ml 'F'ixture
#define KExtFixtureList  ".qxfl" // 'Q'LC+ 'X'ml 'F'ixture 'L'ist
#define KExtWorkspace    ".qxw" // 'Q'LC+ 'X'ml 'W'orkspace
#define KExtInputProfile ".qxi" // 'Q'LC+ 'X'ml 'I'nput profile
#if defined(WIN32) || defined(Q_OS_WIN)
#   define KExtPlugin    ".dll" // Dynamic-Link Library
#elif defined(__APPLE__) || defined(Q_OS_MAC)
#   define KExtPlugin  ".dylib" // DYnamic LIBrary
#else
#   define KExtPlugin    ".so"  // Shared Object
#endif

// Generic XML tags common for all documents
#define KXMLQLCCreator "Creator"
#define KXMLQLCCreatorName "Name"
#define KXMLQLCCreatorVersion "Version"
#define KXMLQLCCreatorAuthor "Author"

// True and false
#define KXMLQLCTrue "True"
#define KXMLQLCFalse "False"

class QLCFile
{
public:
    /**
     * Read an XML file to a QDomDocument structure
     *
     * @param path Path to the file to read
     * @return QDomDocument (null doc if not successful)
     *
     * @return An error code (QFile::NoError if successful)
     */
    static QDomDocument readXML(const QString& path);

    /**
     * Get a common XML file header as a QDomDocument
     *
     * @param content The content type (Settings, Workspace)
     * @param author The file's author (overridden by current user name if empty)
     * @return A new QDomDocument containing the header
     */
    static QDomDocument getXMLHeader(const QString& content, const QString& author = QString());

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
     * Return if the current platform is a Raspberry Pi
     */
    static bool isRaspberry();
};

#endif
