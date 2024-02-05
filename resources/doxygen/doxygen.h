/*
  Q Light Controller Plus
  doxygen.h

  Copyright (C) Jano Svitok

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

#ifndef DOXYGEN_H
#define DOXYGEN_H

/** \mainpage
    This documentation is aimed at developers wanting to find their way around QLC+ sources.
    <br />

    Project home: http://www.qlcplus.org/<br />
    Github page: https://github.com/mcallegari/qlcplus/
 */

/** \defgroup engine Engine
 *  @{
 */

/** \defgroup engine_functions Functions */
/** \defgroup engine_audio Audio */

/** @} */


/** \defgroup fixtureeditor Fixture Editor */

/** \defgroup plugins Plugins */

/** \defgroup ui UI
 *  @{
 */

/** \defgroup ui_fixtures Fixture Manager */
/** \defgroup ui_functions Function Manager */
/** \defgroup ui_shows Show Manager */
/** \defgroup ui_vc Virtual Console
 *  @{
 */

/** \defgroup ui_vc_widgets Widgets */
/** \defgroup ui_vc_props Properties */

/** @} */

/** \defgroup ui_simpledesk Simple Desk */
/** \defgroup ui_io Input/Output Manager */
/** \defgroup ui_mon DMX Monitor */

/** @} */

/** \defgroup webaccess Web access */

// Define simple version of Qt containers so that doxygen recognizes them:

template<class T> class QList { public: T element; };
template<class T> class QVector { public: T element; };
template<class T> class QSet { public: T element; };
template<class T1,T2> class QMap { public: T1 key; T2 value; };

#endif
