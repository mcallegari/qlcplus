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

    /*********************************************************************
     * Stylesheets
     *********************************************************************/
    /**
     * Search the requested $component in a pre-determined
     * stylesheet file on disk
     */
    QString getStyleSheet(QString component);

    /*********************************************************************
     * Digits
     *********************************************************************/
    /**
     * Helper: get the number of digits in an unsigned int (base 10)
     */
    unsigned int digits(unsigned int num);
};

/*****************************************************************************
 * NoEditDelegate
 *****************************************************************************/
/**
 * Set as Item Delegate to make sure that the item will never be editable.
 */
class NoEditDelegate: public QStyledItemDelegate
{
public:
    NoEditDelegate(QObject* parent=0): QStyledItemDelegate(parent) {}
    virtual QWidget* createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const
    {
        return 0;
    }
};

/*****************************************************************************
 * ComboBoxDelegate
 *****************************************************************************/
/**
 * Set as ItemDelegate to make the item editable by a combobox.
 * Value is the index of the selected string.
 */
class ComboBoxDelegate : public QStyledItemDelegate
{
private:
    const QStringList m_strings;

public:
    /**
     * Create a combobox delegate.
     * @param strings The selectable strings in the combobox, arranged in order.
     */
    ComboBoxDelegate(const QStringList &strings, QWidget *parent = 0);

    QWidget *createEditor(QWidget *parent,
            const QStyleOptionViewItem &option,
            const QModelIndex &index) const;

    void setEditorData(QWidget *editor,
            const QModelIndex &index) const;

    void setModelData(QWidget *editor, QAbstractItemModel *model,
            const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
            const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

/** @} */

#endif
