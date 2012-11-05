/*
  Q Light Controller
  qlcfile.h

  Copyright (C) Heikki Junnila

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

#ifndef QLCFILE_H
#define QLCFILE_H

#include <QFile>

class QDomDocument;
class QDomElement;
class QString;

// File extensions
#define KExtFixture      ".qxf" // 'Q'lc 'X'ml 'F'ixture
#define KExtWorkspace    ".qxw" // 'Q'lc 'X'ml 'W'orkspace
#define KExtInputProfile ".qxi" // 'Q'lc 'X'ml 'I'nput profile
#ifdef WIN32
#   define KExtPlugin    ".dll" // Dynamic-Link Library
#elif __APPLE__
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
};

#endif
