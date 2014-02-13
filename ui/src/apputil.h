/*
  Q Light Controller
  apputil.h

  Copyright (c) Heikki Junnila

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

#ifndef APPUTIL_H
#define APPUTIL_H

#include <QStyledItemDelegate>

class QWidget;

/** @addtogroup ui UI
 * @{
 */

namespace AppUtil
{
    /************************************************************************
     * Widget visibility helper
     ************************************************************************/
    /**
     * This helper makes sure that the given widget is within the widget's
     * parent's visible area (if the parent exists). If there is no parent,
     * it ensures that the widget is exposed on an existing screen. The widget
     * is automatically moved to a safe and visible place ONLY if necessary.
     *
     * @param widget The widget to expose
     */
    void ensureWidgetIsVisible(QWidget* widget);

    /*********************************************************************
     * Sane style
     *********************************************************************/
    /**
     * Attempt to get a sane style that replaces Windows' & OSX's crappy
     * sliders as well as buttons that don't obey background color setting.
     */
    QStyle* saneStyle();
};

class NoEditDelegate: public QStyledItemDelegate
{
public:
    NoEditDelegate(QObject* parent=0): QStyledItemDelegate(parent) {}
    virtual QWidget* createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const
    {
        return 0;
    }
};

/** @} */

#endif
