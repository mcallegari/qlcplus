/*
  Q Light Controller
  apputil.h

  Copyright (c) Heikki Junnila

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

#ifndef APPUTIL_H
#define APPUTIL_H

#include <QStyledItemDelegate>

class QWidget;

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

#endif
