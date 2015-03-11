/*
  Q Light Controller
  consolechannel.cpp

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

#include <QContextMenuEvent>
#include <QIntValidator>
#include <QWidgetAction>
#include <QVBoxLayout>
#include <QToolButton>
#include <QSpinBox>
#include <QLabel>
#include <QMenu>
#include <QList>
#include <QtXml>

#include "qlcchannel.h"
#include "qlccapability.h"

#include "doc.h"
#include "fixture.h"
#include "apputil.h"
#include "mastertimer.h"
#include "consolechannel.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

ConsoleChannel::ConsoleChannel(QWidget* parent, Doc* doc, quint32 fixture, quint32 channel, bool isCheckable)
    : QGroupBox(parent)
    , m_doc(doc)
    , m_fixture(fixture)
    , m_channel(channel)
    , m_group(Fixture::invalidId())
    , m_presetButton(NULL)
    , m_cngWidget(NULL)
    , m_spin(NULL)
    , m_slider(NULL)
    , m_label(NULL)
    , m_menu(NULL)
    , m_selected(false)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(channel != QLCChannel::invalid());

    if (isCheckable == true)
        setCheckable(true);
    setFocusPolicy(Qt::NoFocus);
    init();
}

ConsoleChannel::~ConsoleChannel()
{
}

void ConsoleChannel::init()
{
    Fixture* fxi = m_doc->fixture(m_fixture);
    //Q_ASSERT(fxi != NULL);

    new QVBoxLayout(this);
    layout()->setSpacing(0);
    layout()->setContentsMargins(0, 2, 0, 2);

    if (fxi != NULL)
    {
        m_presetButton = new QToolButton(this);
        m_presetButton->setStyle(AppUtil::saneStyle());
        layout()->addWidget(m_presetButton);
        layout()->setAlignment(m_presetButton, Qt::AlignHCenter);
        m_presetButton->setIconSize(QSize(32, 32));
        m_presetButton->setMinimumSize(QSize(32, 32));
        m_presetButton->setMaximumSize(QSize(32, 32));
        m_presetButton->setFocusPolicy(Qt::NoFocus);

        /* Create a menu only if channel has sophisticated contents */
        if (fxi->fixtureDef() != NULL && fxi->fixtureMode() != NULL)
            initMenu();
        else
            m_presetButton->setStyleSheet("QToolButton { border-image: url(:/intensity.png) 0 0 0 0 stretch stretch; }");
    }

    /* Value edit */
    m_spin = new QSpinBox(this);
    m_spin->setRange(0, UCHAR_MAX);
    m_spin->setValue(0);
    m_spin->setMinimumWidth(25);
    m_spin->setMaximumWidth(40);
    m_spin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_spin->setStyle(AppUtil::saneStyle());
    layout()->addWidget(m_spin);
    m_spin->setAlignment(Qt::AlignCenter);
    m_spin->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    layout()->setAlignment(m_spin, Qt::AlignHCenter);

    /* Value slider */
    m_slider = new ClickAndGoSlider(this);
    m_slider->setInvertedAppearance(false);
    m_slider->setRange(0, UCHAR_MAX);
    m_slider->setPageStep(1);
    m_slider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_slider->setFocusPolicy(Qt::NoFocus);
    connect(m_slider, SIGNAL(controlClicked()),
            this, SLOT(slotControlClicked()));

    m_slider->setMinimumWidth(25);
    m_slider->setMaximumWidth(40);
    m_slider->setStyleSheet(
        "QSlider::groove:vertical { background: transparent; width: 32px; } "

        "QSlider::handle:vertical { "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ddd, stop:0.45 #888, stop:0.50 #000, stop:0.55 #888, stop:1 #999);"
        "border: 1px solid #5c5c5c;"
        "border-radius: 4px; margin: 0 -1px; height: 20px; }"

        "QSlider::handle:vertical:hover {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #eee, stop:0.45 #999, stop:0.50 #ff0000, stop:0.55 #999, stop:1 #ccc);"
        "border: 1px solid #000; }"

        "QSlider::add-page:vertical { background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #78d, stop: 1 #97CDEC );"
        "border: 1px solid #5288A7; margin: 0 13px; }"

        "QSlider::sub-page:vertical { background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #888, stop: 1 #ddd );"
        "border: 1px solid #8E8A86; margin: 0 13px; }"

        "QSlider::handle:vertical:disabled { background: QLinearGradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ddd, stop:0.45 #888, stop:0.50 #444, stop:0.55 #888, stop:1 #999);"
        "border: 1px solid #666; }"
        );
    layout()->addWidget(m_slider);
    //layout()->setAlignment(m_slider, Qt::AlignHCenter);

    /* Channel number label */
    m_label = new QLabel(this);
    m_label->setMinimumWidth(25);
    m_label->setMaximumWidth(80);
    layout()->addWidget(m_label);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setText(QString::number(m_channel + 1));
    m_label->setFocusPolicy(Qt::NoFocus);
    m_label->setWordWrap(true);

    /* Set tooltip */
    if (fxi == NULL || fxi->isDimmer() == true)
    {
        setToolTip(tr("Intensity"));
    }
    else
    {
        const QLCChannel* ch = fxi->channel(m_channel);
        Q_ASSERT(ch != NULL);
        setToolTip(QString("%1").arg(ch->name()));
    }

    connect(m_spin, SIGNAL(valueChanged(int)), this, SLOT(slotSpinChanged(int)));
    connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(slotSliderChanged(int)));
    connect(this, SIGNAL(toggled(bool)), this, SLOT(slotChecked(bool)));
}

/*****************************************************************************
 * Fixture & Channel
 *****************************************************************************/

quint32 ConsoleChannel::fixture() const
{
    return m_fixture;
}

quint32 ConsoleChannel::channel() const
{
    return m_channel;
}

/*************************************************************************
 * Group
 *************************************************************************/
void ConsoleChannel::setLabel(QString label)
{
    m_label->setText(label);
}

void ConsoleChannel::setChannelsGroup(quint32 grpid)
{
    m_group = grpid;
    ChannelsGroup *grp = m_doc->channelsGroup(grpid);
    connect(grp, SIGNAL(valueChanged(quint32,uchar)),
            this, SLOT(slotInputValueChanged(quint32,uchar)));
}

void ConsoleChannel::slotInputValueChanged(quint32 channel, uchar value)
{
    Q_UNUSED(channel)
    setValue(value);
}

/*****************************************************************************
 * Value
 *****************************************************************************/

void ConsoleChannel::setValue(uchar value, bool apply)
{
    if (m_slider->value() == value &&
        m_spin->value() == value)
            return;

    if (apply == false)
    {
        m_spin->blockSignals(true);
        m_slider->blockSignals(true);
    }

    m_slider->setValue(int(value));
    m_spin->setValue(int(value));

    if (apply == false)
    {
        m_spin->blockSignals(false);
        m_slider->blockSignals(false);
    }
}

uchar ConsoleChannel::value() const
{
    return uchar(m_slider->value());
}

void ConsoleChannel::slotSpinChanged(int value)
{
    if (value != m_slider->value())
        m_slider->setValue(value);

    if (m_group == Fixture::invalidId())
        emit valueChanged(m_fixture, m_channel, value);
    else
        emit groupValueChanged(m_group, value);
}

void ConsoleChannel::slotSliderChanged(int value)
{
    if (value != m_spin->value())
        m_spin->setValue(value);
}
void ConsoleChannel::slotChecked(bool state)
{
    emit checked(m_fixture, m_channel, state);

    // Emit the current value also when turning the channel back on
    if (state == true)
        emit valueChanged(m_fixture, m_channel, m_slider->value());
}

/*****************************************************************************
 * Menu
 *****************************************************************************/

void ConsoleChannel::initMenu()
{
    Fixture* fxi = m_doc->fixture(fixture());
    Q_ASSERT(fxi != NULL);

    const QLCChannel* ch = fxi->channel(m_channel);
    Q_ASSERT(ch != NULL);

    // Get rid of a possible previous menu
    if (m_menu != NULL)
    {
        delete m_menu;
        m_menu = NULL;
    }

    // Create a popup menu and set the channel name as its title
    m_menu = new QMenu(this);
    m_presetButton->setMenu(m_menu);
    m_presetButton->setPopupMode(QToolButton::InstantPopup);

    QString btnIconStr = ch->getIconNameFromGroup(ch->group());
    if (btnIconStr.startsWith(":"))
        m_presetButton->setStyleSheet("QToolButton { border-image: url(" + btnIconStr + ") 0 0 0 0 stretch stretch; }");
    else
    {
        m_presetButton->setStyleSheet("QToolButton { background: " + btnIconStr + "; }");
        setIntensityButton(ch);
    }

    switch(ch->group())
    {
    case QLCChannel::Colour:
        m_cngWidget = new ClickAndGoWidget();
        m_cngWidget->setType(ClickAndGoWidget::Preset, ch);
        break;
    case QLCChannel::Effect:
        m_cngWidget = new ClickAndGoWidget();
        m_cngWidget->setType(ClickAndGoWidget::Preset, ch);
        break;
    case QLCChannel::Gobo:
        m_cngWidget = new ClickAndGoWidget();
        m_cngWidget->setType(ClickAndGoWidget::Preset, ch);
        break;
    default:
        break;
    }

    if (m_cngWidget != NULL)
    {
        QWidgetAction* action = new QWidgetAction(this);
        action->setDefaultWidget(m_cngWidget);
        m_menu->addAction(action);
        connect(m_cngWidget, SIGNAL(levelChanged(uchar)),
                this, SLOT(slotClickAndGoLevelChanged(uchar)));
        connect(m_cngWidget, SIGNAL(levelAndPresetChanged(uchar,QImage)),
                this, SLOT(slotClickAndGoLevelAndPresetChanged(uchar, QImage)));
    }
    else
    {
        QAction* action = m_menu->addAction(m_presetButton->icon(), ch->name());
        m_menu->setTitle(ch->name());
        action->setEnabled(false);
        m_menu->addSeparator();

        // Initialize the preset menu only for intelligent fixtures
        if (fxi->isDimmer() == false)
            initCapabilityMenu(ch);
    }
}

void ConsoleChannel::setIntensityButton(const QLCChannel* channel)
{
    QFont fnt = m_presetButton->font();
    fnt.setBold(true);
    m_presetButton->setFont(fnt);

    if (channel->colour() == QLCChannel::Red)
    {
        m_presetButton->setText("R"); // Don't localize
        m_cngWidget = new ClickAndGoWidget();
        m_cngWidget->setType(ClickAndGoWidget::Red);
    }
    else if (channel->colour() == QLCChannel::Green)
    {
        m_presetButton->setText("G"); // Don't localize
        m_cngWidget = new ClickAndGoWidget();
        m_cngWidget->setType(ClickAndGoWidget::Green);
    }
    else if (channel->colour() == QLCChannel::Blue)
    {
        QPalette pal = m_presetButton->palette();
        pal.setColor(QPalette::ButtonText, Qt::white); // Improve contrast
        m_presetButton->setPalette(pal);
        m_presetButton->setText("B"); // Don't localize
        m_cngWidget = new ClickAndGoWidget();
        m_cngWidget->setType(ClickAndGoWidget::Blue);
    }
    else if (channel->colour() == QLCChannel::Cyan)
    {
        m_presetButton->setText("C"); // Don't localize
        m_cngWidget = new ClickAndGoWidget();
        m_cngWidget->setType(ClickAndGoWidget::Cyan);
    }
    else if (channel->colour() == QLCChannel::Magenta)
    {
        m_presetButton->setText("M"); // Don't localize
        m_cngWidget = new ClickAndGoWidget();
        m_cngWidget->setType(ClickAndGoWidget::Magenta);
    }
    else if (channel->colour() == QLCChannel::Yellow)
    {
        m_presetButton->setText("Y"); // Don't localize
        m_cngWidget = new ClickAndGoWidget();
        m_cngWidget->setType(ClickAndGoWidget::Yellow);
    }
    else if (channel->colour() == QLCChannel::Amber)
    {
        m_presetButton->setText("A"); // Don't localize
        m_cngWidget = new ClickAndGoWidget();
        m_cngWidget->setType(ClickAndGoWidget::Amber);
    }
    else if (channel->colour() == QLCChannel::White)
    {
        m_presetButton->setText("W"); // Don't localize
        m_cngWidget = new ClickAndGoWidget();
        m_cngWidget->setType(ClickAndGoWidget::White);
    }
    else if (channel->colour() == QLCChannel::UV)
    {
        m_presetButton->setText("UV"); // Don't localize
        m_cngWidget = new ClickAndGoWidget();
        m_cngWidget->setType(ClickAndGoWidget::UV);
    }
    else
    {
        // None of the primary colours matched and since this is an
        // intensity channel, it must be controlling a plain dimmer OSLT.
        m_presetButton->setStyleSheet("QToolButton { border-image: url(:/intensity.png) 0 0 0 0 stretch stretch; }");
    }
}

void ConsoleChannel::initCapabilityMenu(const QLCChannel* ch)
{
    QLCCapability* cap;
    QMenu* valueMenu;
    QAction* action;
    QString s;
    QString t;

    QListIterator <QLCCapability*> it(ch->capabilities());
    while (it.hasNext() == true)
    {
        cap = it.next();

        // Set the value range and name as the menu item's name
        s = QString("%1: %2 - %3").arg(cap->name())
            .arg(cap->min()).arg(cap->max());

        if (cap->max() - cap->min() > 0)
        {
            // Create submenu for ranges of more than one value
            valueMenu = new QMenu(m_menu);
            valueMenu->setTitle(s);

            /* Add a color icon */
            if (ch->group() == QLCChannel::Colour)
                valueMenu->setIcon(colorIcon(cap->name()));

            for (int i = cap->min(); i <= cap->max(); i++)
            {
                action = valueMenu->addAction(
                             t.sprintf("%.3d", i));
                action->setData(i);
            }

            m_menu->addMenu(valueMenu);
        }
        else
        {
            // Just one value in this range, put that into the menu
            action = m_menu->addAction(s);
            action->setData(cap->min());

            /* Add a color icon */
            if (ch->group() == QLCChannel::Colour)
                action->setIcon(colorIcon(cap->name()));
        }
    }

    // Connect menu item activation signal to this
    connect(m_menu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotContextMenuTriggered(QAction*)));

    // Set the menu also as the preset button's popup menu
    m_presetButton->setMenu(m_menu);
}

QIcon ConsoleChannel::colorIcon(const QString& name)
{
    /* Return immediately with a rainbow icon -- if appropriate */
    if (name.toLower().contains("rainbow") ||
            name.toLower().contains("cw") == true)
    {
        return QIcon(":/rainbow.png");
    }
    else if (name.toLower().contains("cto") == true)
    {
        QColor color(255, 201, 0);
        QPixmap pm(32, 32);
        pm.fill(color);
        return QIcon(pm);
    }
    else if (name.toLower().contains("ctb") == true)
    {
        QColor color(0, 128, 190);
        QPixmap pm(32, 32);
        pm.fill(color);
        return QIcon(pm);
    }
    else if (name.toLower().contains("uv") == true)
    {
        QColor color(37, 0, 136);
        QPixmap pm(32, 32);
        pm.fill(color);
        return QIcon(pm);
    }

#ifdef Q_WS_X11
    QColor::setAllowX11ColorNames(true);
#endif
    QStringList colorList(QColor::colorNames());
    QString colname;
    QColor color;
    int index;

    colname = name.toLower().remove(QRegExp("[0-9]")).remove(' ');
    index = colorList.indexOf(colname);
    if (index != -1)
    {
        color.setNamedColor(colname);
    }
    else
    {
        QString re("(");
        QListIterator <QString> it(name.toLower().split(" "));
        while (it.hasNext() == true)
        {
            re += it.next();
            if (it.hasNext() == true)
                re += "|";
        }
        re += ")";

        QRegExp regex(re, Qt::CaseInsensitive);
        index = colorList.indexOf(regex);
        if (index != -1)
            color.setNamedColor(colorList.at(index));
    }

    if (color.isValid() == true)
    {
        QPixmap pm(32, 32);
        pm.fill(color);
        return QIcon(pm);
    }
    else
    {
        return QIcon();
    }
}

void ConsoleChannel::contextMenuEvent(QContextMenuEvent* e)
{
    // Show the preset menu only of it has been created.
    // Generic dimmer fixtures don't have capabilities and so
    // they will not have these menus either.
    if (m_menu != NULL)
    {
        m_menu->exec(e->globalPos());
        e->accept();
    }
}

void ConsoleChannel::slotContextMenuTriggered(QAction* action)
{
    Q_ASSERT(action != NULL);

    // The menuitem's data contains a valid DMX value
    setValue(action->data().toInt());
}

void ConsoleChannel::slotClickAndGoLevelChanged(uchar level)
{
    setValue(level);
}

void ConsoleChannel::slotClickAndGoLevelAndPresetChanged(uchar level, QImage img)
{
    Q_UNUSED(img)
    setValue(level);
}

/*************************************************************************
 * Selection
 *************************************************************************/

bool ConsoleChannel::isSelected()
{
    return m_selected;
}

void ConsoleChannel::slotControlClicked()
{
    qDebug() << "CONTROL modifier + click";
    if (m_selected == false)
    {
        m_originalStyle = this->styleSheet();
        int topMargin = isCheckable()?16:1;

        QString common = "QGroupBox::title {top:-15px; left: 12px; subcontrol-origin: border; background-color: transparent; } "
                         "QGroupBox::indicator { width: 18px; height: 18px; } "
                         "QGroupBox::indicator:checked { image: url(:/checkbox_full.png) } "
                         "QGroupBox::indicator:unchecked { image: url(:/checkbox_empty.png) }";
        QString ssSelected = QString("QGroupBox { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #D9D730, stop: 1 #AFAD27); "
                                 "border: 1px solid gray; border-radius: 4px; margin-top: %1px; margin-right: 1px; } " +
                                 (isCheckable()?common:"")).arg(topMargin);
        setStyleSheet(ssSelected);
        m_selected = true;
    }
    else
    {
        this->setStyleSheet(m_originalStyle);
        m_selected = false;
    }
}
