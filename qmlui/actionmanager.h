/*
  Q Light Controller Plus
  actionmanager.h

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

#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

#include <QQuickView>
#include <QObject>
#include <QPair>

class FunctionManager;
class ShowManager;

class ActionManager : public QObject
{
    Q_OBJECT
public:
    ActionManager() { }
    ActionManager(QQuickView *view, FunctionManager *fManager, ShowManager *sManager, QObject *parent = 0);

    enum ActionType
    {
        None,
        DeleteFunctions,
        DeleteShowItems
    };
    Q_ENUM(ActionType)

    enum PopupButtonsBits
    {
        OK = (1 << 0),
        Cancel = (1 << 1)
    };
    Q_ENUM(PopupButtonsBits)

    Q_INVOKABLE void requestActionPopup(ActionType type, QString message, int buttonsMask, QVariantList data);
    Q_INVOKABLE void acceptAction();
    Q_INVOKABLE void rejectAction();

signals:

private:
    /** Reference of the QML view */
    QQuickView *m_view;
    FunctionManager *m_functionManager;
    ShowManager *m_showManager;

    QPair<ActionType, QVariantList> m_deferredAction;
};

#endif // ACTIONMANAGER_H
