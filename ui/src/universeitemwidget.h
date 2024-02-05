/*
  Q Light Controller Plus
  universeitemwidget.h

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

#ifndef UNIVERSEITEMWIDGET_H
#define UNIVERSEITEMWIDGET_H

#include <QPainter>
#include <QItemDelegate>

/** @addtogroup ui_io
 * @{
 */

class UniverseItemWidget : public QItemDelegate
{
    Q_OBJECT

public:
    UniverseItemWidget(QWidget *parent = 0);

    virtual ~UniverseItemWidget();

    /*********************************************************************
     * Painting
     *********************************************************************/

protected:
    void paint (QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
    //QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;

};

/** @} */

#endif
