/*
  Q Light Controller
  qlcfixturedefcache.cpp

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
#include <QList>
#include <QDebug>
#include <QSet>

#if defined(WIN32) || defined(Q_OS_WIN)
#   include <windows.h>
#else
#   include <unistd.h>
#endif

#include "qlcfixturedefcache.h"
#include "avolitesd4parser.h"
#include "qlcfixturedef.h"
#include "qlcconfig.h"
#include "qlcfile.h"

#define FIXTURES_MAP_NAME "FixturesMap.xml"
#define KXMLQLCFixtureMap "FixturesMap"

QLCFixtureDefCache::QLCFixtureDefCache()
{
}

QLCFixtureDefCache::~QLCFixtureDefCache()
{
    clear();
}

QLCFixtureDef* QLCFixtureDefCache::fixtureDef(
    const QString& manufacturer, const QString& model) const
{
    QListIterator <QLCFixtureDef*> it(m_defs);
    while (it.hasNext() == true)
    {
        QLCFixtureDef* def = it.next();
        if (def->manufacturer() == manufacturer && def->model() == model)
            return def;
    }

    return NULL;
}

QStringList QLCFixtureDefCache::manufacturers() const
{
    QSet <QString> makers;

    // Gather a list of manufacturers
    QListIterator <QLCFixtureDef*> it(m_defs);
    while (it.hasNext() == true)
        makers << it.next()->manufacturer();

    // Bounce the QSet into a QStringList
    QStringList list;
    foreach (QString manuf, makers)
        list << manuf;

    return list;
}

QStringList QLCFixtureDefCache::models(const QString& manufacturer) const
{
    QSet <QString> models;
    QListIterator <QLCFixtureDef*> it(m_defs);
    while (it.hasNext() == true)
    {
        QLCFixtureDef* def = it.next();
        if (def->manufacturer() == manufacturer)
            models << def->model();
    }

    // Bounce the QSet into a QStringList
    QStringList list;
    foreach (QString manuf, models)
        list << manuf;

    return list;
}

bool QLCFixtureDefCache::addFixtureDef(QLCFixtureDef* fixtureDef)
{
    if (fixtureDef == NULL)
        return false;

    if (models(fixtureDef->manufacturer()).contains(fixtureDef->model()) == false)
    {
        m_defs << fixtureDef;
        return true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "Cache already contains"
                   << fixtureDef->name();
        return false;
    }
}

bool QLCFixtureDefCache::storeFixtureDef(QString filename, QString data)
{
    QDir userFolder = userDefinitionDirectory();

    QFile file(userFolder.absoluteFilePath(filename));
    if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
        return false;

    file.write(data.toLatin1());
    file.close();

    // reload user definitions
    load(userDefinitionDirectory());

    return true;
}

bool QLCFixtureDefCache::load(const QDir& dir)
{
    qDebug() << Q_FUNC_INFO << dir.path();

    if (dir.exists() == false || dir.isReadable() == false)
        return false;

    /* Attempt to read all specified files from the given directory */
    QStringListIterator it(dir.entryList());
    while (it.hasNext() == true)
    {
        QString path(dir.absoluteFilePath(it.next()));

        if (path.toLower().endsWith(KExtFixture) == true)
            loadQXF(path);
        else if (path.toLower().endsWith(KExtAvolitesFixture) == true)
            loadD4(path);
        else
            qWarning() << Q_FUNC_INFO << "Unrecognized fixture extension:" << path;
    }

    return true;
}

bool QLCFixtureDefCache::loadMap(const QDir &dir)
{
    qDebug() << Q_FUNC_INFO << dir.path();

    if (dir.exists() == false || dir.isReadable() == false)
        return false;

    QString mapPath(dir.absoluteFilePath(FIXTURES_MAP_NAME));

    if (mapPath.isEmpty() == true)
        return false;

    QDomDocument doc = QLCFile::readXML(mapPath);
    if (doc.isNull() == true)
    {
        qWarning() << Q_FUNC_INFO << "Unable to read from" << mapPath;
        return QFile::ReadError;
    }

    if (doc.doctype().name() == KXMLQLCFixtureMap)
    {
        QDomElement root = doc.documentElement();
        if (root.tagName() == KXMLQLCFixtureMap)
        {
            QDomNode node = root.firstChild();
            while (node.isNull() == false)
            {
                QString defFile= "";
                QString manufacturer = "";
                QString model = "";

                QDomElement tag = node.toElement();
                if (tag.tagName() == "fixture")
                {
                    if (tag.hasAttribute("path"))
                        defFile = QString(dir.absoluteFilePath(tag.attribute("path")));
                    if(tag.hasAttribute("mf"))
                        manufacturer = tag.attribute("mf");
                    if(tag.hasAttribute("md"))
                        model = tag.attribute("md");

                    if (defFile.isEmpty() == false &&
                        manufacturer.isEmpty() == false &&
                        model.isEmpty() == false)
                    {
                        QLCFixtureDef* fxi = new QLCFixtureDef();
                        Q_ASSERT(fxi != NULL);

                        fxi->setDefinitionSourceFile(defFile);
                        fxi->setManufacturer(manufacturer);
                        fxi->setModel(model);

                        /* Delete the def if it's a duplicate. */
                        if (addFixtureDef(fxi) == false)
                            delete fxi;
                        fxi = NULL;
                    }
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Unknown Fixture Map tag: " << tag.tagName();
                }

                node = node.nextSibling();
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << mapPath
                       << "is not a fixture map file";
            return false;
        }
    }
    else
    {
        qWarning() << Q_FUNC_INFO << mapPath
                   << "is not a fixture map file";
        return false;
    }

    return true;
}

void QLCFixtureDefCache::clear()
{
    while (m_defs.isEmpty() == false)
        delete m_defs.takeFirst();
}

QDir QLCFixtureDefCache::systemDefinitionDirectory()
{
    QDir dir;
#if defined(__APPLE__) || defined(Q_OS_MAC)
    dir.setPath(QString("%1/../%2").arg(QCoreApplication::applicationDirPath())
                                   .arg(FIXTUREDIR));
#else
    dir.setPath(FIXTUREDIR);
#endif

    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));

    return dir;
}

QDir QLCFixtureDefCache::userDefinitionDirectory()
{
    QDir dir;

#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
    // If the current user is root, return the system fixture dir.
    // Otherwise return a path under user's home dir.
    if (geteuid() == 0 && QLCFile::isRaspberry() == false)
        dir = QDir(FIXTUREDIR);
    else
        dir.setPath(QString("%1/%2").arg(getenv("HOME")).arg(USERFIXTUREDIR));
#elif defined(__APPLE__) || defined (Q_OS_MAC)
    /* User's input profile directory on OSX */
    dir.setPath(QString("%1/%2").arg(getenv("HOME")).arg(USERFIXTUREDIR));
#else
    /* User's input profile directory on Windows */
    LPTSTR home = (LPTSTR) malloc(256 * sizeof(TCHAR));
    GetEnvironmentVariable(TEXT("UserProfile"), home, 256);
    dir.setPath(QString("%1/%2")
                    .arg(QString::fromUtf16(reinterpret_cast<ushort*> (home)))
                    .arg(USERFIXTUREDIR));
    free(home);
#endif

    // Ensure the directory exists
    if (dir.exists() == false)
        dir.mkpath(".");

    dir.setFilter(QDir::Files);
    QStringList filters;
    filters << QString("*%1").arg(KExtFixture);
    filters << QString("*%1").arg(KExtAvolitesFixture);
    dir.setNameFilters(filters);

    return dir;
}

void QLCFixtureDefCache::loadQXF(const QString& path)
{
    QLCFixtureDef* fxi = new QLCFixtureDef();
    Q_ASSERT(fxi != NULL);

    QFile::FileError error = fxi->loadXML(path);
    if (error == QFile::NoError)
    {
        /* Delete the def if it's a duplicate. */
        if (addFixtureDef(fxi) == false)
            delete fxi;
        fxi = NULL;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "Fixture definition loading from"
                   << path << "failed:" << QLCFile::errorString(error);
        delete fxi;
        fxi = NULL;
    }
}

void QLCFixtureDefCache::loadD4(const QString& path)
{
    AvolitesD4Parser parser;
    if (parser.loadXML(path) == false)
    {
        qWarning() << Q_FUNC_INFO << "Unable to load D4 fixture from" << path
                   << ":" << parser.lastError();
        return;
    }

    QLCFixtureDef* fxi = new QLCFixtureDef();
    Q_ASSERT(fxi != NULL);
    if (parser.fillFixtureDef(fxi) == false)
    {
        qWarning() << Q_FUNC_INFO << "Unable to parse D4 fixture from" << path
                   << ":" << parser.lastError();
        delete fxi;
        return;
    }

    /* Delete the def if it's a duplicate. */
    if (addFixtureDef(fxi) == false)
    {
        qDebug() << Q_FUNC_INFO << "Deleting duplicate" << path;
        delete fxi;
    }
    fxi = NULL;
}
