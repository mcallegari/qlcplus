/*
  Q Light Controller Plus
  sequence.h

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

#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "chaser.h"
#include "scene.h"
#include "chaserrunner.h"

/** @addtogroup engine_functions Functions
 * @{
 */

class Sequence : public Chaser
{
    Q_OBJECT
    Q_DISABLE_COPY(Sequence)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    Sequence(Doc* doc);
    virtual ~Sequence();

    /** @reimp */
    QIcon getIcon() const;

    /*********************************************************************
     * Copying
     *********************************************************************/
public:
    /** @reimp */
    Function* createCopy(Doc* doc, bool addToDoc = true);

    /** Copy the contents for this function from another function */
    bool copyFrom(const Function* function);

    /*********************************************************************
     * Bound Scene
     *********************************************************************/
public:
    /**
     * Set the Scene ID bound to this Sequence
     * @param sceneID The ID of the Scene to bound
     *
     */
    void setBoundSceneID(quint32 sceneID);

    /**
     * Returns the current bound scene ID
     *
     * @return The associated Scene for this Chaser in sequence mode
     */
    quint32 boundSceneID() const;

    QList<quint32> components();

protected:
    /** The Scene ID associated to this Sequence */
    quint32 m_boundSceneID;

    /** Temporary flag that indicates if the Sequence steps need to be
     *  adjusted against the bound Scene values. Normally the bound Scene
     *  has a lower ID, so it is found during the first XML load, but in
     *  case it is not, steps will be fixed on postLoad */
    bool m_needFixup;

    /*********************************************************************
     * Save & Load
     *********************************************************************/
public:
    /** @reimpl */
    bool saveXML(QXmlStreamWriter *doc);

    /** @reimpl */
    bool loadXML(QXmlStreamReader &root);

    /** @reimp */
    void postLoad();
};

/** @} */

#endif
