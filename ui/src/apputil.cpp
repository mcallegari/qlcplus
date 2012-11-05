/*
  Q Light Controller
  apputil.cpp

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

#include <QDesktopWidget>
#include <QStyleFactory>
#include <QApplication>
#include <QSettings>
#include <QLocale>
#include <QWidget>
#include <QStyle>
#include <QRect>

#include "apputil.h"

/****************************************************************************
 * Widget visibility helper
 ****************************************************************************/

void AppUtil::ensureWidgetIsVisible(QWidget* widget)
{
    if (widget == NULL)
        return;

    QWidget* parent = widget->parentWidget();
    if (widget->windowFlags() & Qt::Window)
    {
        // The widget is a top-level window (a dialog, for instance)
        // @todo Use the screen where the main application currently is?
        QDesktopWidget dw;
        QWidget* screen(dw.screen());
        if (screen != NULL)
        {
            // Move the widget to the center of the default screen
            const QRect screenRect(screen->rect());
            if (screenRect.contains(widget->pos()) == false)
            {
                QRect widgetRect(widget->rect());
                widgetRect.moveCenter(screenRect.center());
                widget->setGeometry(widgetRect);
            }
        }
        else
        {
            // Last resort: move to top left and hope the widget is visible
            widget->move(0, 0);
        }
    }
    else if (parent != NULL)
    {
        // The widget's placement is bounded by a parent
        const QRect parentRect(parent->rect());
        if (parentRect.contains(widget->pos()) == false)
        {
            // Move the widget to the center of the parent if wouldn't
            // otherwise be visible
            QRect widgetRect(widget->rect());
            widgetRect.moveCenter(parentRect.center());
            widget->setGeometry(widgetRect);
        }
    }
}

/*****************************************************************************
 * Sane style
 *****************************************************************************/

#define SETTINGS_SLIDERSTYLE "workspace/sliderstyle"

static QStyle* s_saneStyle = NULL;

QStyle* AppUtil::saneStyle()
{
    if (s_saneStyle == NULL)
    {
        QSettings settings;
        QVariant var = settings.value(SETTINGS_SLIDERSTYLE, QString("Cleanlooks"));
        QStringList keys(QStyleFactory::keys());
        if (keys.contains(var.toString()) == true)
            s_saneStyle = QStyleFactory::create(var.toString());
        else
            s_saneStyle = QApplication::style();
    }

    return s_saneStyle;
}
