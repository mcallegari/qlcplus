/*
  Q Light Controller Plus
  qlcclipboard.h

  Copyright (c) Massimo Callegari

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef QLCCLIPBOARD_H
#define QLCCLIPBOARD_H

#include <QList>

#include "chaserstep.h"
#include "scenevalue.h"

class Chaser;
class Scene;
class Doc;

class QLCClipboard: public QObject
{
    Q_OBJECT

public:
    QLCClipboard(Doc *doc);

public:
    enum ActionType
    {
        CopyChaserStep,
        NoType
    };

    void resetContents();

private:
    Doc* m_doc;

    /********************************************************************
     * Copy Action
     ********************************************************************/

public:
    void copyContent(quint32 sourceID, QList <ChaserStep> steps);
    void copyContent(quint32 sourceID, QList <SceneValue> values);
    void copyContent(quint32 sourceID, Function *function);

    bool hasChaserSteps();
    bool hasSceneValues();
    bool hasFunction();

    QList <ChaserStep> getChaserSteps();
    QList <SceneValue> getSceneValues();
    Function *getFunction();

private:
    QList <ChaserStep> m_copySteps;
    QList <SceneValue> m_copySceneValues;
    Function *m_copyFunction;
};

#endif // QLCCLIPBOARD_H
