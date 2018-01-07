/*
  Q Light Controller Plus
  functioneditor.h

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

#ifndef FUNCTIONEDITOR_H
#define FUNCTIONEDITOR_H

#include <QQmlContext>
#include <QQuickView>
#include <QQuickItem>
#include <QObject>

#include "function.h"

class Doc;

class FunctionEditor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString functionName READ functionName WRITE setFunctionName NOTIFY functionNameChanged)
    Q_PROPERTY(int previousID READ previousID WRITE setPreviousID NOTIFY previousIDChanged)
    Q_PROPERTY(bool previewEnabled READ previewEnabled WRITE setPreviewEnabled NOTIFY previewEnabledChanged)
    Q_PROPERTY(int tempoType READ tempoType WRITE setTempoType NOTIFY tempoTypeChanged)

    Q_PROPERTY(int fadeInSpeed READ fadeInSpeed WRITE setFadeInSpeed NOTIFY fadeInSpeedChanged)
    Q_PROPERTY(int holdSpeed READ holdSpeed WRITE setHoldSpeed NOTIFY holdSpeedChanged)
    Q_PROPERTY(int fadeOutSpeed READ fadeOutSpeed WRITE setFadeOutSpeed NOTIFY fadeOutSpeedChanged)
    Q_PROPERTY(int duration READ duration NOTIFY durationChanged)

    Q_PROPERTY(int runOrder READ runOrder WRITE setRunOrder NOTIFY runOrderChanged)
    Q_PROPERTY(int direction READ direction WRITE setDirection NOTIFY directionChanged)

public:
    FunctionEditor(QQuickView *view, Doc *doc, QObject *parent = 0);
    virtual ~FunctionEditor();

    /** Set the ID of the Function being edit */
    virtual void setFunctionID(quint32 ID);

    /** Return the ID of the Function being edited */
    virtual quint32 functionID() const;

    /** Return the type of the Function being edited */
    virtual Function::Type functionType() const;

    /** Get/Set the preview status of the Function being edited */
    virtual bool previewEnabled() const;
    virtual void setPreviewEnabled(bool enable);

    /** Get/Set the name of the Function being edited */
    virtual QString functionName() const;
    virtual void setFunctionName(QString functionName);

    /** Get the ID of the view that was open before this editor.
     *  This is to handle Collection -> Function and back */
    int previousID() const;
    void setPreviousID(int previousID);

    /** Generic method to delete items of an editor.
      * $list might be a list of indices, IDs or something else */
    virtual void deleteItems(QVariantList list);

signals:
    void functionNameChanged(QString functionName);
    void previousIDChanged(int previousID);
    void previewEnabledChanged(bool enabled);

protected:
    /** Reference of the QML view */
    QQuickView *m_view;
    /** Reference of the project workspace */
    Doc *m_doc;
    /** ID of the Function being edited */
    quint32 m_functionID;
    /** ID of the item of the previous view */
    int m_previousID;
    /** Reference of the Function being edited */
    Function *m_function;
    /** Type of the Function being edited */
    Function::Type m_functionType;
    /** Flag that holds if the editor should preview its function */
    bool m_previewEnabled;

    /************************************************************************
     * Speed
     ************************************************************************/
public:
    /** Get/Set the tempo type of the Function being edited */
    virtual int tempoType() const;
    virtual void setTempoType(int tempoType);

    /** Get/Set the Function fade in speed */
    virtual int fadeInSpeed() const;
    virtual void setFadeInSpeed(int fadeInSpeed);

    /** Get/Set the Function hold speed */
    virtual int holdSpeed() const;
    virtual void setHoldSpeed(int holdSpeed);

    /** Get/Set the Function fade out speed */
    virtual int fadeOutSpeed() const;
    virtual void setFadeOutSpeed(int fadeOutSpeed);

    /** Get the Function duration */
    virtual int duration() const;

signals:
    void tempoTypeChanged(int tempoType);
    void fadeInSpeedChanged(int fadeInSpeed);
    void holdSpeedChanged(int holdSpeed);
    void fadeOutSpeedChanged(int fadeOutSpeed);
    void durationChanged(int duration);

    /************************************************************************
     * Run order and direction
     ************************************************************************/
public:
    /** Get/Set the Function run order */
    virtual int runOrder() const;
    virtual void setRunOrder(int runOrder);

    /** Get/Set the Function run direction */
    virtual int direction() const;
    virtual void setDirection(int direction);

signals:
    void runOrderChanged(int runOrder);
    void directionChanged(int direction);
};

#endif // SCENEEDITOR_H
