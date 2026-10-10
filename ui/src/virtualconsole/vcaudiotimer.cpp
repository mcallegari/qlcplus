/*
  Q Light Controller Plus
  vcaudiotimer.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QInputDialog>
#include <QFontMetrics>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPolygonF>
#include <QLineEdit>
#include <QPainter>
#include <QString>
#include <QTimer>
#include <QDebug>
#include <QSize>

#include "vcaudiotimer.h"
#include "function.h"
#include "doc.h"

/* Same resolution as the MasterTimer tick (20ms at the default 50Hz) */
#define UPDATE_INTERVAL 20

struct AudioTimerEntry
{
    quint32 id;
    bool paused;
    QString name;
    quint32 elapsed;
    quint32 total;
};

static QString msToString(quint32 ms)
{
    quint32 secs = ms / 1000;
    return QString("%1:%2.%3").arg(secs / 60, 2, 10, QChar('0'))
                              .arg(secs % 60, 2, 10, QChar('0'))
                              .arg(ms % 1000, 3, 10, QChar('0'));
}

static QList<AudioTimerEntry> runningAudio(Doc *doc)
{
    QList<AudioTimerEntry> list;

    foreach (Function *f, doc->functionsByType(Function::AudioType))
    {
        if (f == NULL || f->isRunning() == false)
            continue;

        AudioTimerEntry entry;
        entry.id = f->id();
        entry.paused = f->isPaused();
        entry.name = f->name();
        entry.total = f->totalDuration();
        entry.elapsed = entry.total ? qMin(f->elapsed(), entry.total) : f->elapsed();
        list.append(entry);
    }

    return list;
}

VCAudioTimer::VCAudioTimer(QWidget* parent, Doc* doc) : VCWidget(parent, doc)
{
    /* Set the class name "VCAudioTimer" as the object name as well */
    setObjectName(VCAudioTimer::staticMetaObject.className());

    setType(VCWidget::AudioTimerWidget);
    setCaption(tr("Audio Timer"));
    resize(QSize(300, 60));

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotUpdate()));
    m_timer->start(UPDATE_INTERVAL);
}

VCAudioTimer::~VCAudioTimer()
{
}

void VCAudioTimer::slotUpdate()
{
    QString state;
    foreach (AudioTimerEntry entry, runningAudio(m_doc))
        state.append(QString("%1|%2|%3|%4;").arg(entry.name).arg(entry.elapsed)
                                            .arg(entry.total).arg(entry.paused));

    if (state != m_lastState)
    {
        m_lastState = state;
        update();
    }
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

VCWidget* VCAudioTimer::createCopy(VCWidget* parent) const
{
    Q_ASSERT(parent != NULL);

    VCAudioTimer* timer = new VCAudioTimer(parent, m_doc);
    if (timer->copyFrom(this) == false)
    {
        delete timer;
        timer = NULL;
    }

    return timer;
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCAudioTimer::editProperties()
{
    bool ok = false;
    QString text = QInputDialog::getText(NULL, tr("Rename Audio Timer"), tr("Caption:"),
                                         QLineEdit::Normal, caption(), &ok);
    if (ok == true)
        setCaption(text);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCAudioTimer::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCAudioTimer)
    {
        qWarning() << Q_FUNC_INFO << "Audio timer node not found";
        return false;
    }

    /* Widget commons */
    loadXMLCommon(root);

    /* Children */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCWindowState)
        {
            int x = 0, y = 0, w = 0, h = 0;
            bool visible = false;
            loadXMLWindowState(root, &x, &y, &w, &h, &visible);
            setGeometry(x, y, w, h);
        }
        else if (root.name() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(root);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown audio timer tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool VCAudioTimer::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* VC Audio Timer entry */
    doc->writeStartElement(KXMLQLCVCAudioTimer);

    saveXMLCommon(doc);

    /* Window state */
    saveXMLWindowState(doc);

    /* Appearance */
    saveXMLAppearance(doc);

    /* End the <AudioTimer> tag */
    doc->writeEndElement();

    return true;
}

/****************************************************************************
 * Drawing
 ****************************************************************************/

void VCAudioTimer::paintEvent(QPaintEvent* e)
{
    QPainter painter(this);
    painter.setFont(font());

    QColor fg = palette().color(foregroundRole());
    QRect area = rect().adjusted(4, 2, -4, -2);
    QList<AudioTimerEntry> list = runningAudio(m_doc);
    m_buttons.clear();

    if (list.isEmpty())
    {
        QColor dim = fg;
        dim.setAlpha(120);
        painter.setPen(dim);
        painter.drawText(area, Qt::AlignCenter | Qt::TextWordWrap, caption());
    }
    else
    {
        QFontMetrics fm(font());
        int barHeight = qMax(4, fm.height() / 4);
        int rowHeight = qMax(area.height() / list.count(), fm.height() + barHeight + 4);
        int y = area.top();

        foreach (AudioTimerEntry entry, list)
        {
            QRect textRect(area.left(), y, area.width(), fm.height());
            QString times;
            if (entry.total)
                times = QString("%1 / %2   -%3").arg(msToString(entry.elapsed),
                                                     msToString(entry.total),
                                                     msToString(entry.total - entry.elapsed));
            else
                times = msToString(entry.elapsed);

            /* Play/Pause and Stop buttons */
            int btnSize = fm.height();
            QRect playRect(textRect.left(), textRect.top(), btnSize, btnSize);
            QRect stopRect(playRect.right() + 4, textRect.top(), btnSize, btnSize);
            drawButton(painter, playRect, entry.paused ? PlayButton : PauseButton, fg);
            drawButton(painter, stopRect, StopButton, fg);
            m_buttons.append(ButtonArea(entry.id, PauseButton, playRect));
            m_buttons.append(ButtonArea(entry.id, StopButton, stopRect));

            int timesWidth = fm.horizontalAdvance(times);
            painter.setPen(fg);
            painter.drawText(textRect, Qt::AlignRight | Qt::AlignVCenter, times);
            QRect nameRect = textRect.adjusted(stopRect.right() + 8 - textRect.left(), 0, -(timesWidth + 10), 0);
            painter.drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter,
                             fm.elidedText(entry.name, Qt::ElideRight, nameRect.width()));

            if (entry.total)
            {
                QRect barRect(area.left(), textRect.bottom() + 2, area.width(), barHeight);
                painter.setPen(Qt::NoPen);
                QColor groove = fg;
                groove.setAlpha(50);
                painter.setBrush(groove);
                painter.drawRect(barRect);

                // the last 10 seconds are highlighted in red, a paused track in orange
                bool ending = entry.total - entry.elapsed <= 10000;
                if (entry.paused)
                    painter.setBrush(QColor(230, 160, 40));
                else
                    painter.setBrush(ending ? QColor(220, 50, 50) : QColor(80, 200, 80));
                barRect.setWidth((int)((double)barRect.width() * entry.elapsed / entry.total));
                painter.drawRect(barRect);
            }

            y += rowHeight;
        }
    }

    painter.end();

    VCWidget::paintEvent(e);
}

void VCAudioTimer::drawButton(QPainter &painter, const QRect &r, ButtonType type, const QColor &color)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);

    QColor bg = color;
    bg.setAlpha(40);
    painter.setPen(QPen(color, 1));
    painter.setBrush(bg);
    painter.drawRoundedRect(QRectF(r).adjusted(0.5, 0.5, -0.5, -0.5), 3, 3);

    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    QRectF g = QRectF(r).adjusted(r.width() * 0.3, r.height() * 0.28, -r.width() * 0.3, -r.height() * 0.28);

    if (type == PlayButton)
    {
        QPolygonF triangle;
        triangle << g.topLeft() << QPointF(g.right(), g.center().y()) << g.bottomLeft();
        painter.drawPolygon(triangle);
    }
    else if (type == PauseButton)
    {
        qreal w = g.width() / 3;
        painter.drawRect(QRectF(g.left(), g.top(), w, g.height()));
        painter.drawRect(QRectF(g.right() - w, g.top(), w, g.height()));
    }
    else
    {
        painter.drawRect(g);
    }

    painter.restore();
}

void VCAudioTimer::mousePressEvent(QMouseEvent *e)
{
    if (mode() != Doc::Operate || isDisabled())
    {
        VCWidget::mousePressEvent(e);
        return;
    }

    foreach (ButtonArea area, m_buttons)
    {
        if (area.rect.contains(e->pos()) == false)
            continue;

        Function *f = m_doc->function(area.functionID);
        if (f == NULL || f->isRunning() == false)
            break;

        if (area.type == StopButton)
            f->stop(FunctionParent::master());
        else
            f->setPause(!f->isPaused());

        update();
        break;
    }
}
