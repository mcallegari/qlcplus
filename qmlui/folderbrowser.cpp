/*
  Q Light Controller Plus
  folderbrowser.cpp

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
#include <QDirIterator>
#include <QDebug>

#include "folderbrowser.h"
#include "qlcfile.h"

FolderBrowser::FolderBrowser(QObject *)
{
}

FolderBrowser::~FolderBrowser()
{
}

void FolderBrowser::initialize()
{
    QDir homeDir = QLCFile::userDirectory(QString(), QString(), QStringList());
    setCurrentPath(homeDir.absolutePath());
}

QString FolderBrowser::separator() const
{
    return "/"; //QDir::separator();
}

QString FolderBrowser::currentPath() const
{
    return m_currentPath;
}

void FolderBrowser::setCurrentPath(QString path)
{
    if (path == m_currentPath)
        return;

    qDebug() << "Set path:" << path;

    QDir dir(path);
    m_currentPath = dir.absolutePath();

    emit currentPathChanged();
    emit pathModelChanged();
    emit folderModelChanged();
}


QString FolderBrowser::selectedNameFilter() const
{
    return m_selectedNameFilter;
}

void FolderBrowser::setSelectedNameFilter(const QString &newSelectedNameFilter)
{
    if (m_selectedNameFilter == newSelectedNameFilter)
        return;

    m_selectedNameFilter = newSelectedNameFilter;
    emit selectedNameFilterChanged();
    emit folderModelChanged();
}

QVariant FolderBrowser::pathModel() const
{
    QVariantList list;

    if (m_currentPath.isEmpty())
        return list;

    QStringList tList = m_currentPath.split(separator());

    QString absPath;

    for (QString &tk : tList)
    {
        QVariantMap params;

#if defined(WIN32) || defined(Q_OS_WIN)
        if (!absPath.isEmpty())
            absPath.append(separator());
#else
        if (absPath.isEmpty() && tk.isEmpty())
        {
            // on *nix systems paths start with a slash
            absPath.append(QDir::separator());
            params.insert("name", "<root>");
            params.insert("absPath", absPath);
            list.prepend(params);
            continue;
        }
        else
        {
            absPath.append(separator());
        }
#endif
        absPath.append(tk);

        QDir dir(absPath);
        //qDebug() << "Name:" << tk << ", absPath:" << absPath;
        params.insert("name", tk);
        params.insert("absPath", dir.absolutePath());
        list.append(params);
    }

    return QVariant::fromValue(list);
}

QVariant FolderBrowser::folderModel() const
{
    QVariantList list;

    if (m_currentPath.isEmpty())
        return list;

    QDir cp(m_currentPath);
    cp.setSorting(QDir::SortFlag::DirsFirst | QDir::SortFlag::Name);
    cp.setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);

    // extract name filters
    const qsizetype l = m_selectedNameFilter.indexOf('(');
    const qsizetype r = m_selectedNameFilter.lastIndexOf(')');
    if (l >= 0 && r > l)
    {
        const QString patterns = m_selectedNameFilter.mid(l + 1, r - l - 1);   // "*.qxw *.qxf"
        QStringList filters = patterns.split(' ', Qt::SkipEmptyParts);
        cp.setNameFilters(filters);
    }

    for (QFileInfo &info : cp.entryInfoList())
    {
        QVariantMap params;

        //qDebug() << "Name:" << info.fileName();
        params.insert("name", info.fileName());
        params.insert("isFolder", info.isDir() ? true : false);
        list.append(params);
    }

    return QVariant::fromValue(list);
}

QVariant FolderBrowser::drivesModel() const
{
    QVariantList list;

    QVariantMap home;
    home.insert("name", "Home");
    home.insert("path", QDir::homePath());
    list.append(home);

    for (QFileInfo &info : QDir::drives())
    {
        QVariantMap params;
        params.insert("name", info.absolutePath());
        params.insert("path", info.absolutePath());
        list.append(params);
    }

    return QVariant::fromValue(list);
}

