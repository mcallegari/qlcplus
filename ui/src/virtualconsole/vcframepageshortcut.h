/*
  Q Light Controller Plus
  vcframepageshortcut.h

  Copyright (c) Lukas Jähn
                Massimo Callegari

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

#ifndef VCFRAMEPAGESHORTCUT_H
#define VCFRAMEPAGESHORTCUT_H

#include <QSharedPointer>
#include <QKeySequence>

#include "qlcinputsource.h"

class QXmlStreamReader;
class QXmlStreamWriter;

/** @addtogroup ui_vc_widgets
 * @{
 */

#define KXMLQLCVCFramePageShortcut       QString("Shortcut")
#define KXMLQLCVCFramePageShortcutPage   QString("Page")
#define KXMLQLCVCFramePageShortcutName   QString("Name")

class VCFramePageShortcut
{
public:
    explicit VCFramePageShortcut(int pageIndex, quint8 inputID);

    /** Destructor */
    ~VCFramePageShortcut();

    QString name() const;
    void setName(QString name = QString());

    /************************************************************************
     * Load & Save
     ***********************************************************************/
public:
    /** Load properties and contents from an XML tree */
    bool loadXML(QXmlStreamReader &root);

    /** Save properties and contents to an XML document */
    bool saveXML(QXmlStreamWriter *doc);

protected:
    /** The page name */
    QString m_name;

public:
    /** The shortcut unique ID */
    quint8 m_id;
    /** The associated VCFrame page index */
    int m_page;
    /** Reference to the input source to jump to this page */
    QSharedPointer<QLCInputSource> m_inputSource;
    /** The key sequence to jump to this page */
    QKeySequence m_keySequence;
};

/** @} */

#endif // VCFRAMEPAGESHORTCUT_H
