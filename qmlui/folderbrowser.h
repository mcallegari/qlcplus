/*
  Q Light Controller Plus
  folderbrowser.h

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

#ifndef FOLDERBROWSER_H
#define FOLDERBROWSER_H

#include <QVariant>

class FolderBrowser : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FolderBrowser)

    Q_PROPERTY(QString currentPath READ currentPath WRITE setCurrentPath NOTIFY currentPathChanged)
    Q_PROPERTY(QVariant pathModel READ pathModel NOTIFY pathModelChanged)
    Q_PROPERTY(QVariant folderModel READ folderModel NOTIFY folderModelChanged)
    Q_PROPERTY(QVariant drivesModel READ drivesModel NOTIFY drivesModelChanged)

public:
    FolderBrowser(QObject *parent = nullptr);
    ~FolderBrowser();

    Q_INVOKABLE void initialize();
    Q_INVOKABLE QString separator() const;

    QString currentPath() const;
    void setCurrentPath(QString path);

    QVariant pathModel() const;
    QVariant folderModel() const;
    QVariant drivesModel() const;

signals:
    void currentPathChanged();
    void pathModelChanged();
    void folderModelChanged();
    void drivesModelChanged();

private:
    QString m_currentPath;
    QVariant m_folderModel;
};

#endif
