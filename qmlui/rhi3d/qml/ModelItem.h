/*
  Q Light Controller Plus
  ModelItem.h

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

#pragma once

#include "qml/MeshItem.h"

class ModelItem : public MeshItem
{
    Q_OBJECT
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)

public:
    explicit ModelItem(QObject *parent = nullptr);

    MeshType type() const override { return MeshType::Model; }

    QString path() const { return m_path; }
    void setPath(const QString &path);

Q_SIGNALS:
    void pathChanged();

private:
    QString m_path;
};
