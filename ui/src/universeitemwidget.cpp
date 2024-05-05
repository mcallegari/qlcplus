/*
  Q Light Controller Plus
  universeitemwidget.cpp

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

#include <QApplication>
#include <QWidget>
#include <QFont>
#include <QDebug>

#include "universeitemwidget.h"

UniverseItemWidget::UniverseItemWidget(QWidget *parent)
    : QItemDelegate(parent)
{
    setClipping(false);
}

UniverseItemWidget::~UniverseItemWidget()
{
}

void UniverseItemWidget::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QWidget *list = qobject_cast<QWidget *>(parent());
    qreal pWidth = list->geometry().width();
    QRect r = option.rect;
    QFont font = qApp->font();
    font.setBold(true);
    font.setPixelSize(18);
    painter->setRenderHint(QPainter::Antialiasing, true);

    // draw background gradient
    QLinearGradient linearGrad(r.left(), r.top(), r.left(), r.height() + r.top());

    if (option.state & QStyle::State_Selected)
    {
        linearGrad.setColorAt(0, QColor(50, 64, 75, 255));
        linearGrad.setColorAt(1, QColor(76, 98, 115, 255));
        painter->setPen(QPen(QColor(48, 61, 72, 255), 2));
    }
    else
    {
        linearGrad.setColorAt(0, QColor(255, 255, 255, 255));
        linearGrad.setColorAt(1, QColor(128, 128, 128, 255));
        painter->setPen(QPen(QColor(30, 30, 30, 255), 2));
    }
    painter->setBrush(linearGrad);
    painter->drawRoundedRect(r.left() + 2, r.top() + 2, pWidth - 6, r.height() - 4, 5, 5);

    if (option.state & QStyle::State_Selected)
        painter->setPen(QPen(QColor(200, 200, 200, 255), 2));
    else
        painter->setPen(QPen(QColor(0, 0, 0, 255), 2));

    // draw universe name
    painter->setFont(font);
    painter->drawText(QRect(10, r.top() + 5, 150, r.height() - 10),
                      Qt::AlignLeft | Qt::TextWordWrap | Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());

    font.setPixelSize(12);
    painter->setFont(font);

    QVariant var = index.data(Qt::DecorationRole);
    if (var.isValid())
    {
        QIcon icon = var.value<QIcon>();
        if (icon.isNull() == false)
            painter->drawPixmap(pWidth - 36, r.top() + 9, 32, 32, icon.pixmap(32, 32));
    }

    // draw input output labels
    int midPos = (pWidth - 10 - 150) / 2;
    midPos += 170;
    QString inStr = tr("Input:");
    QString proStr = tr("Profile:");
    QString outStr = tr("Output:");
    QString fbStr = tr("Feedback:");
    painter->drawText(QRect(170, r.top() + 10, 150, 20), Qt::AlignLeft, inStr);
    painter->drawText(QRect(midPos, r.top() + 10, 150, 20), Qt::AlignLeft, proStr);
    painter->drawText(QRect(170, r.top() + 30, 150, 20), Qt::AlignLeft, outStr);
    painter->drawText(QRect(midPos, r.top() + 30, 150, 20), Qt::AlignLeft, fbStr);

    QFontMetrics fm(font);
#if (QT_VERSION < QT_VERSION_CHECK(5, 11, 0))
    int inPos = fm.width(inStr) + 170 + 5;
    int proPos = fm.width(proStr) + midPos + 5;
    int outPos = fm.width(outStr) + 170 + 5;
    int fbPos = fm.width(fbStr) + midPos + 5;
#else
    int inPos = fm.horizontalAdvance(inStr) + 170 + 5;
    int proPos = fm.horizontalAdvance(proStr) + midPos + 5;
    int outPos = fm.horizontalAdvance(outStr) + 170 + 5;
    int fbPos = fm.horizontalAdvance(fbStr) + midPos + 5;
#endif
    font.setBold(false);
    painter->setFont(font);

    // draw input output plugin/profile names
    QString inputName = index.data(Qt::UserRole + 1).toString();
    if (inputName == "None") inputName = tr("None");
    QString profileName = index.data(Qt::UserRole + 2).toString();
    if (profileName == "None") profileName = tr("None");
    QString outputName = index.data(Qt::UserRole + 3).toString();
    if (outputName == "None") outputName = tr("None");
    QString fbName = index.data(Qt::UserRole + 4).toString();
    if (fbName == "None") fbName = tr("None");

    painter->drawText(QRect(inPos, r.top() + 10, midPos - inPos, 20),
                      Qt::AlignLeft, inputName);
    painter->drawText(QRect(proPos, r.top() + 10, pWidth - proPos, 20),
                      Qt::AlignLeft, profileName);
    painter->drawText(QRect(outPos, r.top() + 30, midPos - outPos, 20),
                      Qt::AlignLeft, outputName);
    painter->drawText(QRect(fbPos, r.top() + 30, pWidth - fbPos, 20),
                      Qt::AlignLeft, fbName);
}



