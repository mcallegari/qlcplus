/*
  Q Light Controller Plus
  audioeditor.h

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

#ifndef AUDIOEDITOR_H
#define AUDIOEDITOR_H

#include "functioneditor.h"

class Audio;
class ListModel;

class AudioEditor : public FunctionEditor
{
    Q_OBJECT

    Q_PROPERTY(QString sourceFileName READ sourceFileName WRITE setSourceFileName NOTIFY sourceFileNameChanged)
    Q_PROPERTY(QStringList mimeTypes READ mimeTypes CONSTANT)
    Q_PROPERTY(QVariant mediaInfo READ mediaInfo NOTIFY mediaInfoChanged)

public:
    AudioEditor(QQuickView *view, Doc *doc, QObject *parent = 0);

    /** Set the ID of the Audio being edited */
    void setFunctionID(quint32 ID);

    /** Get/Set the source file name for this Audio function */
    QString sourceFileName() const;
    void setSourceFileName(QString sourceFileName);

    /** Get the supported file types that can be decoded */
    QStringList mimeTypes() const;

    /** Get the information of the currently loaded media source */
    QVariant mediaInfo() const;

signals:
    void sourceFileNameChanged(QString sourceFileName);
    void mediaInfoChanged();

private:
    /** Reference of the Audio currently being edited */
    Audio *m_audio;
};

#endif // AUDIOEDITOR_H
