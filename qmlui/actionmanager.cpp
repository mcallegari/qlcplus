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

#include <QQuickItem>

#include "actionmanager.h"

#include "functionmanager.h"
#include "showmanager.h"

ActionManager::ActionManager(QQuickView *view, FunctionManager *fManager, ShowManager *sManager, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_functionManager(fManager)
    , m_showManager(sManager)
{

}

void ActionManager::requestActionPopup(ActionManager::ActionType type, QString message, int buttonsMask, QVariantList data)
{
    QQuickItem *popupItem = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("popupBox"));
    if (popupItem == NULL)
    {
        qDebug() << "popupBox not found !";
        return;
    }

    qDebug() << "[ActionManager] buttonsMask:" << buttonsMask << data;

    popupItem->setProperty("message", message);
    popupItem->setProperty("buttonsMask", buttonsMask);

    // save the requested action to be processed when the user
    // confirms or rejects from the popup
    m_deferredAction = QPair<ActionType, QVariantList> (type, data);

    popupItem->setProperty("visible", true);
}

void ActionManager::acceptAction()
{
    ActionType action = m_deferredAction.first;
    if (action == None)
        return;

    QVariantList data = m_deferredAction.second;

    switch(action)
    {
        case DeleteFunctions:
        {
            m_functionManager->deleteFunctions(data);
        }
        break;
        case DeleteShowItems:
        {
            m_showManager->deleteShowItems(data);
        }
        break;
        default: break;
    }

    // invalidate the action just processed
    m_deferredAction = QPair<ActionType, QVariantList> (None, QVariantList());
}

void ActionManager::rejectAction()
{
    // invalidate the current action
    m_deferredAction = QPair<ActionType, QVariantList> (None, QVariantList());
}

