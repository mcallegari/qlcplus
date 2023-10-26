/*
  Q Light Controller
  apputil.cpp

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

#include <QComboBox>
#include <QStyleFactory>
#include <QApplication>
#include <QTextStream>
#include <QSettings>
#include <QLocale>
#include <QWidget>
#include <QScreen>
#include <QStyle>
#include <QRect>
#include <QFile>
#include <QDir>

#if defined(WIN32) || defined(Q_OS_WIN)
#include <windows.h>
#endif

#include "apputil.h"
#include "qlcconfig.h"

/****************************************************************************
 * Widget visibility helper
 ****************************************************************************/

void AppUtil::ensureWidgetIsVisible(QWidget *widget)
{
    if (widget == NULL)
        return;

    QWidget *parent = widget->parentWidget();
    if (widget->windowFlags() & Qt::Window)
    {
        // The widget is a top-level window (a dialog, for instance)
        // @todo Use the screen where the main application currently is?
        QScreen *screen = QGuiApplication::screens().first();
        if (screen != NULL)
        {
            // Move the widget to the center of the default screen
            const QRect screenRect(screen->availableGeometry());
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
        QVariant var = settings.value(SETTINGS_SLIDERSTYLE, QString("Fusion"));
        QStringList keys(QStyleFactory::keys());

        if (keys.contains(var.toString()) == true)
            s_saneStyle = QStyleFactory::create(var.toString());
        else
            s_saneStyle = QApplication::style();
    }

    return s_saneStyle;
}

/*********************************************************************
 * Stylesheets
 *********************************************************************/

#define USER_STYLESHEET_FILE "qlcplusStyle.qss"

QString AppUtil::getStyleSheet(QString component)
{
    QString ssDir;
    QString result;

#if defined(WIN32) || defined(Q_OS_WIN)
    /* User's input profile directory on Windows */
    LPTSTR home = (LPTSTR) malloc(256 * sizeof(TCHAR));
    GetEnvironmentVariable(TEXT("UserProfile"), home, 256);
    ssDir = QString("%1/%2").arg(QString::fromUtf16(reinterpret_cast<char16_t*> (home)))
                .arg(USERQLCPLUSDIR);
    free(home);
    HotPlugMonitor::setWinId(winId());
#else
    /* User's input profile directory on *NIX systems */
    ssDir = QString("%1/%2").arg(getenv("HOME")).arg(USERQLCPLUSDIR);
#endif

    QFile ssFile(ssDir + QDir::separator() + USER_STYLESHEET_FILE);
    if (ssFile.exists() == false)
        return result;

    if (ssFile.open(QIODevice::ReadOnly) == false)
        return result;

    bool found = false;
    QTextStream in(&ssFile);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        if (line.startsWith("====="))
        {
            if (found == true)
                break;

            QString comp = line.replace("=", "");
            if (comp.simplified() == component)
                found = true;
        }
        else if (found == true)
        {
            result.append(line);
        }
    }
    ssFile.close();

    return result;
}

/*****************************************************************************
 * Digits
 *****************************************************************************/

unsigned int AppUtil::digits(unsigned int n)
{
    unsigned int res = 1;
    while (n /= 10)
        ++res;
    return res;
}

/*****************************************************************************
 * ComboBoxDelegate
 *****************************************************************************/

ComboBoxDelegate::ComboBoxDelegate(const QStringList &strings, QWidget *parent)
    : QStyledItemDelegate(parent)
    , m_strings(strings)
{
}

QWidget *ComboBoxDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &/*option*/,
        const QModelIndex &/*index*/) const
{
    QComboBox *comboBox = new QComboBox(parent);
    comboBox->addItems(m_strings);
    return comboBox;
}

void ComboBoxDelegate::setEditorData(QWidget *editor,
        const QModelIndex &index) const
{
    int value = index.model()->data(index, Qt::UserRole).toInt();
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    comboBox->setCurrentIndex(value);
}

void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    int value = comboBox->currentIndex();
    model->setData(index, value, Qt::UserRole);
    model->setData(index, comboBox->currentText(), Qt::DisplayRole);
}

void ComboBoxDelegate::updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
{
    editor->setGeometry(option.rect);
}
