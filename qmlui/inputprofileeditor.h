/*
  Q Light Controller Plus
  inputprofileeditor.h

  Copyright (C) Massimo Callegari

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

#ifndef INPUTPROFILEEDITOR_H
#define INPUTPROFILEEDITOR_H

#include <QObject>
#include <QMap>

class Doc;
class QLCInputProfile;

class InputProfileEditor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool modified READ modified WRITE setModified NOTIFY modifiedChanged FINAL)
    Q_PROPERTY(QString manufacturer READ manufacturer WRITE setManufacturer NOTIFY manufacturerChanged FINAL)
    Q_PROPERTY(QString model READ model WRITE setModel NOTIFY modelChanged FINAL)
    Q_PROPERTY(int type READ type WRITE setType NOTIFY typeChanged FINAL)
    Q_PROPERTY(QVariant channels READ channels NOTIFY channelsChanged FINAL)

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    InputProfileEditor(QLCInputProfile *profile, Doc *doc, QObject *parent = nullptr);
    ~InputProfileEditor();

    /* Get/Set the profile modified state */
    bool modified() const;
    void setModified(bool newModified = true);

    /* Get/Set the manufacturer of the profile currently being edited */
    QString manufacturer() const;
    void setManufacturer(const QString &newManufacturer);

    /* Get/Set the model of the profile currently being edited */
    QString model() const;
    void setModel(const QString &newModel);

    /* Get/Set the type of the profile currently being edited */
    int type();
    void setType(const int &newType);

    /* Enable/Disable input detection */
    Q_INVOKABLE void toggleDetection();

    /* Return a QML-ready list of channels of the profile
     * currently being edited */
    QVariant channels();

protected slots:
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value, const QString& key);

signals:
    void modifiedChanged();
    void manufacturerChanged();
    void modelChanged();
    void typeChanged();
    void channelsChanged();

private:
    Doc *m_doc;
    QLCInputProfile *m_profile;
    bool m_modified;
    bool m_detection;
    // map of <channel, values> used to detect if
    // an input signal comes from a button or a fader
    QMap<quint32, QVector<uchar>> m_channelsMap;
};

#endif
