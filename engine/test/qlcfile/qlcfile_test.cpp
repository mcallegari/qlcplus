/*
  Q Light Controller - Unit tests
  qlcfile_test.cpp

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

#include <QtTest>

#if defined(WIN32) || defined(Q_OS_WIN)
#else
#   include <sys/types.h>
#   include <sys/stat.h>
#endif

#include "qlcfile_test.h"
#include "qlcfile.h"
#include "qlcconfig.h"

void QLCFile_Test::readXML()
{
#ifdef QT_XML_LIB
    QDomDocument doc;

    doc = QLCFile::readXML(QString());
    QVERIFY(doc.isNull() == true);

    doc = QLCFile::readXML("foobar");
    QVERIFY(doc.isNull() == true);

    doc = QLCFile::readXML("broken.xml");
    QVERIFY(doc.isNull() == false);
    QCOMPARE(doc.firstChild().toElement().tagName(), QString("Workspace"));
    QCOMPARE(doc.firstChild().firstChild().toElement().tagName(), QString("Creator"));

    QString source("readonly.xml.in");
    QString path("readonly.xml");
    QFile(source).copy(path);

#if !defined(WIN32) && !defined(Q_OS_WIN)
	QFile::Permissions perms = QFile::permissions(path);
	QFile::setPermissions(path, 0);
    doc = QLCFile::readXML(path);
    QVERIFY(doc.isNull() == true);
	QFile::setPermissions(path, perms);
#endif

    doc = QLCFile::readXML(path);
    QVERIFY(doc.isNull() == false);
    QCOMPARE(doc.firstChild().toElement().tagName(), QString("Workspace"));
    QCOMPARE(doc.firstChild().firstChild().toElement().tagName(), QString("Creator"));
#endif
}

void QLCFile_Test::getXMLHeader()
{
#ifdef QT_XML_LIB
    bool insideCreatorTag = false, author = false, appname = false,
         appversion = false;
    QDomDocument doc;

    doc = QLCFile::getXMLHeader(QString());
    QVERIFY(doc.isNull() == true);

    doc = QLCFile::getXMLHeader("Settings");
    QVERIFY(doc.isNull() == false);
    QCOMPARE(doc.doctype().name(), QString("Settings"));

    QDomNode node(doc.firstChild());
    node = node.nextSibling();
    QCOMPARE(node.toElement().tagName(), QString("Settings"));
    node = node.firstChild();
    QCOMPARE(node.toElement().tagName(), QString("Creator"));
    node = node.firstChild();
    while (node.isNull() == false)
    {
        // Verify that program enters this while loop
        insideCreatorTag = true;

        QDomElement tag(node.toElement());
        if (tag.tagName() == KXMLQLCCreatorAuthor)
            author = true; // User might not have a full name so don't check the contents
        else if (tag.tagName() == KXMLQLCCreatorName)
            appname = true;
        else if (tag.tagName() == KXMLQLCCreatorVersion)
            appversion = true;
        else
            QFAIL(QString("Unrecognized tag: %1").arg(tag.tagName()).toUtf8().constData());

        node = node.nextSibling();
    }

    QCOMPARE(insideCreatorTag, true);
    QCOMPARE(author, true);
    QCOMPARE(appname, true);
    QCOMPARE(appversion, true);
#endif
}

void QLCFile_Test::errorString()
{
    QCOMPARE(QLCFile::errorString(QFile::NoError),
             tr("No error occurred."));
    QCOMPARE(QLCFile::errorString(QFile::ReadError),
             tr("An error occurred when reading from the file."));
    QCOMPARE(QLCFile::errorString(QFile::WriteError),
             tr("An error occurred when writing to the file."));
    QCOMPARE(QLCFile::errorString(QFile::FatalError),
             tr("A fatal error occurred."));
    QCOMPARE(QLCFile::errorString(QFile::ResourceError),
             tr("Resource error occurred."));
    QCOMPARE(QLCFile::errorString(QFile::OpenError),
             tr("The file could not be opened."));
    QCOMPARE(QLCFile::errorString(QFile::AbortError),
             tr("The operation was aborted."));
    QCOMPARE(QLCFile::errorString(QFile::TimeOutError),
             tr("A timeout occurred."));
    QCOMPARE(QLCFile::errorString(QFile::UnspecifiedError),
             tr("An unspecified error occurred."));
    QCOMPARE(QLCFile::errorString(QFile::RemoveError),
             tr("The file could not be removed."));
    QCOMPARE(QLCFile::errorString(QFile::RenameError),
             tr("The file could not be renamed."));
    QCOMPARE(QLCFile::errorString(QFile::PositionError),
             tr("The position in the file could not be changed."));
    QCOMPARE(QLCFile::errorString(QFile::ResizeError),
             tr("The file could not be resized."));
    QCOMPARE(QLCFile::errorString(QFile::PermissionsError),
             tr("The file could not be accessed."));
    QCOMPARE(QLCFile::errorString(QFile::CopyError),
             tr("The file could not be copied."));
    QCOMPARE(QLCFile::errorString(QFile::FileError(31337)),
             tr("An unknown error occurred."));
}

QTEST_APPLESS_MAIN(QLCFile_Test)
