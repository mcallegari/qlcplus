/*
  Q Light Controller
  vcbutton.cpp

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

#include <QStyleOptionButton>
#include <QWidgetAction>
#include <QColorDialog>
#include <QImageReader>
#include <QFileDialog>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QMessageBox>
#include <QByteArray>
#include <QPainter>
#include <QString>
#include <QDebug>
#include <QEvent>
#include <QTimer>
#include <QBrush>
#include <QStyle>
#include <QtXml>
#include <QMenu>
#include <QSize>
#include <QPen>

#if defined(WIN32) || defined(Q_OS_WIN)
 #include <QStyleFactory>
#endif

#include "qlcinputsource.h"
#include "qlcmacros.h"
#include "qlcfile.h"

#include "vcbuttonproperties.h"
#include "vcpropertieseditor.h"
#include "functionselection.h"
#include "clickandgoslider.h"
#include "qlcinputchannel.h"
#include "virtualconsole.h"
#include "mastertimer.h"
#include "vcsoloframe.h"
#include "inputpatch.h"
#include "vcbutton.h"
#include "function.h"
#include "fixture.h"
#include "apputil.h"
#include "doc.h"

const QSize VCButton::defaultSize(QSize(50, 50));

/*****************************************************************************
 * Initialization
 *****************************************************************************/

VCButton::VCButton(QWidget* parent, Doc* doc) : VCWidget(parent, doc)
    , m_iconPath()
    , m_blackoutFadeOutTime(0)
    , m_startupIntensityEnabled(false)
    , m_startupIntensity(1.0)
{
    /* Set the class name "VCButton" as the object name as well */
    setObjectName(VCButton::staticMetaObject.className());

    /* No function is initially attached to the button */
    m_function = Function::invalidId();

    setType(VCWidget::ButtonWidget);
    setCaption(QString());
    setOn(false);
    setAction(Toggle);
    setFrameStyle(KVCFrameStyleNone);

    /* Menu actions */
    m_chooseIconAction = new QAction(QIcon(":/image.png"), tr("Choose..."),
                                     this);
    m_chooseIconAction->setShortcut(QKeySequence("SHIFT+C"));

    m_resetIconAction = new QAction(QIcon(":/undo.png"), tr("None"), this);
    m_resetIconAction->setShortcut(QKeySequence("SHIFT+ALT+C"));

    connect(m_chooseIconAction, SIGNAL(triggered(bool)),
            this, SLOT(slotChooseIcon()));
    connect(m_resetIconAction, SIGNAL(triggered(bool)),
            this, SLOT(slotResetIcon()));

    /* Initial size */
    QSettings settings;
    QVariant var = settings.value(SETTINGS_BUTTON_SIZE);
    if (var.isValid() == true)
        resize(var.toSize());
    else
        resize(defaultSize);

    var = settings.value(SETTINGS_BUTTON_STATUSLED);
    if (var.isValid() == true && var.toBool() == true)
        m_ledStyle = true;
    else
        m_ledStyle = false;

    setStyle(AppUtil::saneStyle());

    /* Listen to function removals */
    connect(m_doc, SIGNAL(functionRemoved(quint32)),
            this, SLOT(slotFunctionRemoved(quint32)));
}

VCButton::~VCButton()
{
}

void VCButton::setID(quint32 id)
{
    VCWidget::setID(id);

    if (caption().isEmpty())
        setCaption(tr("Button %1").arg(id));
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

VCWidget* VCButton::createCopy(VCWidget* parent)
{
    Q_ASSERT(parent != NULL);

    VCButton* button = new VCButton(parent, m_doc);
    if (button->copyFrom(this) == false)
    {
        delete button;
        button = NULL;
    }

    return button;
}

bool VCButton::copyFrom(const VCWidget* widget)
{
    const VCButton* button = qobject_cast <const VCButton*> (widget);
    if (button == NULL)
        return false;

    /* Copy button-specific stuff */
    setIconPath(button->iconPath());
    setKeySequence(button->keySequence());
    setFunction(button->function());
    enableStartupIntensity(button->isStartupIntensityEnabled());
    setStartupIntensity(button->startupIntensity());
    setAction(button->action());
    m_on = button->m_on;

    /* Copy common stuff */
    return VCWidget::copyFrom(widget);
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCButton::editProperties()
{
    VCButtonProperties prop(this, m_doc);
    if (prop.exec() == QDialog::Accepted)
        m_doc->setModified();
}

/*****************************************************************************
 * Background color
 *****************************************************************************/

void VCButton::setBackgroundImage(const QString& path)
{
    Q_UNUSED(path);
}

void VCButton::setBackgroundColor(const QColor& color)
{
    QPalette pal = palette();

    m_hasCustomBackgroundColor = true;
    m_backgroundImage = QString();
    pal.setColor(QPalette::Button, color);
    setPalette(pal);

    m_doc->setModified();
}

void VCButton::resetBackgroundColor()
{
    QColor fg;

    m_hasCustomBackgroundColor = false;
    m_backgroundImage = QString();

    /* Store foreground color */
    if (m_hasCustomForegroundColor == true)
        fg = palette().color(QPalette::ButtonText);

    /* Reset the whole palette to application palette */
    setPalette(QApplication::palette());

    /* Restore foreground color */
    if (fg.isValid() == true)
    {
        QPalette pal = palette();
        pal.setColor(QPalette::ButtonText, fg);
        setPalette(pal);
    }

    m_doc->setModified();
}

QColor VCButton::backgroundColor() const
{
    return palette().color(QPalette::Button);
}

/*****************************************************************************
 * Foreground color
 *****************************************************************************/

void VCButton::setForegroundColor(const QColor& color)
{
    QPalette pal = palette();

    m_hasCustomForegroundColor = true;

    pal.setColor(QPalette::WindowText, color);
    pal.setColor(QPalette::ButtonText, color);
    setPalette(pal);

    m_doc->setModified();
}

void VCButton::resetForegroundColor()
{
    QColor bg;

    m_hasCustomForegroundColor = false;

    /* Store background color */
    if (m_hasCustomBackgroundColor == true)
        bg = palette().color(QPalette::Button);

    /* Reset the whole palette to application palette */
    setPalette(QApplication::palette());

    /* Restore background color */
    if (bg.isValid() == true)
        setBackgroundColor(bg);

    m_doc->setModified();
}

QColor VCButton::foregroundColor() const
{
    return palette().color(QPalette::ButtonText);
}

/*****************************************************************************
 * Button icon
 *****************************************************************************/

QString VCButton::iconPath() const
{
    return m_iconPath;
}

void VCButton::setIconPath(const QString& iconPath)
{
    m_iconPath = iconPath;

    updateIcon();
    m_doc->setModified();
    update();
}

void VCButton::slotChooseIcon()
{
    /* No point coming here if there is no VC */
    VirtualConsole* vc = VirtualConsole::instance();
    if (vc == NULL)
        return;

    QString formats;
    QListIterator <QByteArray> it(QImageReader::supportedImageFormats());
    while (it.hasNext() == true)
        formats += QString("*.%1 ").arg(QString(it.next()).toLower());

    QString path;
    path = QFileDialog::getOpenFileName(this, tr("Select button icon"),
                                        iconPath(), tr("Images (%1)").arg(formats));
    if (path.isEmpty() == false)
    {
        VCWidget* widget;
        foreach(widget, vc->selectedWidgets())
        {
            VCButton* button = qobject_cast<VCButton*> (widget);
            if (button != NULL)
                button->setIconPath(path);
        }
    }
}

void VCButton::updateIcon()
{
    if (m_action == Blackout)
    {
        m_icon = QIcon(":/blackout.png");
        m_iconSize = QSize(26, 26);
    }
    else if (m_action == StopAll)
    {
        m_icon = QIcon(":/panic.png");
        m_iconSize = QSize(26, 26);
    }
    else if (iconPath().isEmpty() == false)
    {
        m_icon = QIcon(iconPath());
        m_iconSize = QSize(26, 26);
    }
    else
    {
        m_icon = QIcon();
        m_iconSize = QSize(-1, -1);
    }
}

void VCButton::slotResetIcon()
{
    setIconPath(QString());
    update();
}

/*****************************************************************************
 * Function attachment
 *****************************************************************************/

void VCButton::setFunction(quint32 fid)
{
    Function* old = m_doc->function(m_function);
    if (old != NULL)
    {
        /* Get rid of old function connections */
        disconnect(old, SIGNAL(running(quint32)),
                this, SLOT(slotFunctionRunning(quint32)));
        disconnect(old, SIGNAL(stopped(quint32)),
                this, SLOT(slotFunctionStopped(quint32)));
        disconnect(old, SIGNAL(flashing(quint32,bool)),
                this, SLOT(slotFunctionFlashing(quint32,bool)));
    }

    Function* function = m_doc->function(fid);
    if (function != NULL)
    {
        /* Connect to the new function */
        connect(function, SIGNAL(running(quint32)),
                this, SLOT(slotFunctionRunning(quint32)));
        connect(function, SIGNAL(stopped(quint32)),
                this, SLOT(slotFunctionStopped(quint32)));
        connect(function, SIGNAL(flashing(quint32,bool)),
                this, SLOT(slotFunctionFlashing(quint32,bool)));

        m_function = fid;
        setToolTip(function->name());
    }
    else
    {
        /* No function attachment */
        m_function = Function::invalidId();
        setToolTip(QString());
    }
}

quint32 VCButton::function() const
{
    return m_function;
}

void VCButton::notifyFunctionStarting(quint32 fid)
{
    if (mode() == Doc::Design)
        return;

    if (fid == m_function)
        return;

    if (m_function != Function::invalidId() && action() == VCButton::Toggle)
    {
        Function *f = m_doc->function(m_function);
        if (f != NULL && !f->stopped())
            f->stop();
    }
}

void VCButton::slotFunctionRemoved(quint32 fid)
{
    /* Invalidate the button's function if it's the one that was removed */
    if (fid == m_function)
        setFunction(Function::invalidId());
}

/*****************************************************************************
 * Button state
 *****************************************************************************/

bool VCButton::isOn() const
{
    return m_on;
}

void VCButton::setOn(bool on)
{
    m_on = on;

    emit pressedState(m_on);

    updateFeedback();

    update();
}

void VCButton::updateOnState()
{
    bool on;
    if (m_action == Blackout)
    {
        on = m_doc->inputOutputMap()->blackout();
    }
    else if (m_action == StopAll)
    {
        on = false;
    }
    else if (m_action == Flash)
    {
        on = false;
    }
    else // (m_action == Toggle)
    {
        on = false;
        Function* function = m_doc->function(m_function);
        if (function != NULL)
            on = function->isRunning();
    }
    if (m_on != on)
        setOn(on);
}

/*****************************************************************************
 * Key sequence handler
 *****************************************************************************/

void VCButton::setKeySequence(const QKeySequence& keySequence)
{
    m_keySequence = QKeySequence(keySequence);
}

QKeySequence VCButton::keySequence() const
{
    return m_keySequence;
}

void VCButton::slotKeyPressed(const QKeySequence& keySequence)
{
    if (isEnabled() == false)
        return;

    if (m_keySequence == keySequence)
        pressFunction();
}

void VCButton::slotKeyReleased(const QKeySequence& keySequence)
{
    if (isEnabled() == false)
        return;

    if (m_keySequence == keySequence)
        releaseFunction();
}

void VCButton::updateFeedback()
{
    if (m_on == true)
        sendFeedback(UCHAR_MAX);
    else
        sendFeedback(0);
}

/*****************************************************************************
 * External input
 *****************************************************************************/

void VCButton::slotInputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    if (isEnabled() == false)
        return;

    if (checkInputSource(universe, (page() << 16) | channel, value, sender()))
    {
        if (m_action == Flash)
        {
            // Keep the button depressed only while the external button is kept down.
            // Raise the button when the external button is raised.
            if (isOn() == false && value > 0)
                pressFunction();
            else if (isOn() == true && value == 0)
                releaseFunction();
        }
        else if (value > 0)
        {
            // Only toggle when the external button is pressed.
            // Releasing the button does nothing.
            pressFunction();
        }
    }
}

/*****************************************************************************
 * Button action
 *****************************************************************************/

void VCButton::setAction(Action action)
{
    // Blackout signal connection
    if (m_action == Blackout && action != Blackout)
        disconnect(m_doc->inputOutputMap(), SIGNAL(blackoutChanged(bool)),
                this, SLOT(slotBlackoutChanged(bool)));
    else if (m_action != Blackout && action == Blackout)
        connect(m_doc->inputOutputMap(), SIGNAL(blackoutChanged(bool)),
                this, SLOT(slotBlackoutChanged(bool)));

    // Action update
    m_action = action;
    updateIcon();

    // Update tooltip
    if (m_action == Blackout)
        setToolTip(tr("Toggle Blackout"));
    else if (m_action == StopAll)
        setToolTip(tr("Stop ALL functions!"));
}

VCButton::Action VCButton::action() const
{
    return m_action;
}

QString VCButton::actionToString(VCButton::Action action)
{
    if (action == Flash)
        return QString(KXMLQLCVCButtonActionFlash);
    else if (action == Blackout)
        return QString(KXMLQLCVCButtonActionBlackout);
    else if (action == StopAll)
        return QString(KXMLQLCVCButtonActionStopAll);
    else
        return QString(KXMLQLCVCButtonActionToggle);
}

VCButton::Action VCButton::stringToAction(const QString& str)
{
    if (str == KXMLQLCVCButtonActionFlash)
        return Flash;
    else if (str == KXMLQLCVCButtonActionBlackout)
        return Blackout;
    else if (str == KXMLQLCVCButtonActionStopAll)
        return StopAll;
    else
        return Toggle;
}

void VCButton::setStopAllFadeOutTime(int ms)
{
    m_blackoutFadeOutTime = ms;
}

int VCButton::stopAllFadeTime()
{
    return m_blackoutFadeOutTime;
}

/*****************************************************************************
 * Intensity adjustment
 *****************************************************************************/

void VCButton::enableStartupIntensity(bool enable)
{
    m_startupIntensityEnabled = enable;
}

bool VCButton::isStartupIntensityEnabled() const
{
    return m_startupIntensityEnabled;
}

void VCButton::setStartupIntensity(qreal fraction)
{
    m_startupIntensity = CLAMP(fraction, qreal(0), qreal(1));
}

qreal VCButton::startupIntensity() const
{
    return m_startupIntensity;
}

void VCButton::slotAttributeChanged(int value)
{
    ClickAndGoSlider *slider = (ClickAndGoSlider *)sender();
    int idx = slider->property("attrIdx").toInt();

    Function* func = m_doc->function(m_function);
    if (func != NULL)
        func->adjustAttribute((qreal)value / 100, idx);
}

/*****************************************************************************
 * Button press / release handlers
 *****************************************************************************/

void VCButton::pressFunction()
{
    /* Don't allow pressing during design mode */
    if (mode() == Doc::Design)
        return;

    Function* f = NULL;
    if (m_action == Toggle)
    {
        f = m_doc->function(m_function);
        if (f == NULL)
            return;

        if (VirtualConsole::instance() != NULL &&
            VirtualConsole::instance()->isTapModifierDown() == true)
        {
            // Produce a tap when the tap modifier key is down
            f->tap();
            blink(50);
        }
        else
        {
            // if the button is in a SoloFrame and the function is running but was
            // started by a different function (a chaser or collection), turn other
            // functions off and start this one.
            //
            if (isOn() == true && !(isChildOfSoloFrame() && f->startedAsChild()))
            {
                f->stop();
            }
            else
            {
                if (isStartupIntensityEnabled() == true)
                    f->adjustAttribute(startupIntensity() * intensity(), Function::Intensity);
                else
                    f->adjustAttribute(intensity(), Function::Intensity);

                f->start(m_doc->masterTimer());
                emit functionStarting(m_function);
            }
        }
    }
    else if (m_action == Flash && isOn() == false)
    {
        f = m_doc->function(m_function);
        if (f != NULL)
            f->flash(m_doc->masterTimer());
    }
    else if (m_action == Blackout)
    {
        m_doc->inputOutputMap()->toggleBlackout();
    }
    else if (m_action == StopAll)
    {
        if (stopAllFadeTime() == 0)
            m_doc->masterTimer()->stopAllFunctions();
        else
            m_doc->masterTimer()->fadeAndStopAll(stopAllFadeTime());
    }
}

void VCButton::releaseFunction()
{
    /* Don't allow operation during design mode */
    if (mode() == Doc::Design)
        return;

    if (m_action == Flash && isOn() == true)
    {
        Function* f = m_doc->function(m_function);
        if (f != NULL)
            f->unFlash(m_doc->masterTimer());
    }
}

void VCButton::slotFunctionRunning(quint32 fid)
{
    if (fid == m_function && m_action == Toggle)
        setOn(true);
}

void VCButton::slotFunctionStopped(quint32 fid)
{
    if (fid == m_function && m_action == Toggle)
    {
        setOn(false);
        blink(250);
    }
}

void VCButton::slotFunctionFlashing(quint32 fid, bool state)
{
    // Do not change the state of the button for Blackout or Stop All Functions buttons
    if (m_action != Toggle && m_action != Flash)
        return;

    if (fid != m_function)
        return;

    // if the function was flashed by another button, and the function is still running, keep the button pushed
    Function* f = m_doc->function(m_function);
    if (state == false && m_action == Toggle && f != NULL && f->isRunning())
    {
        return;
    }

    setOn(state);
}

void VCButton::blink(int ms)
{
    slotBlink();
    QTimer::singleShot(ms, this, SLOT(slotBlink()));
}

void VCButton::slotBlink()
{
    // This function is called twice with same XOR mask,
    // thus creating a brief opposite-color -- normal-color blink
    QPalette pal = palette();
    QColor color(pal.color(QPalette::Button));
    color.setRgb(color.red()^0xff, color.green()^0xff, color.blue()^0xff);
    pal.setColor(QPalette::Button, color);
    setPalette(pal);
}

void VCButton::slotBlackoutChanged(bool state)
{
    setOn(state);
}

bool VCButton::isChildOfSoloFrame() const
{
    QWidget* parent = parentWidget();
    while (parent != NULL)
    {
        if (qobject_cast<VCSoloFrame*>(parent) != NULL)
            return true;
        parent = parent->parentWidget();
    }
    return false;
}

/*****************************************************************************
 * Custom menu
 *****************************************************************************/

QMenu* VCButton::customMenu(QMenu* parentMenu)
{
    QMenu* menu = new QMenu(parentMenu);
    menu->setTitle(tr("Icon"));
    menu->addAction(m_chooseIconAction);
    menu->addAction(m_resetIconAction);

    return menu;
}

void VCButton::adjustIntensity(qreal val)
{
    Function* func = m_doc->function(m_function);
    if (func != NULL)
    {
        if (isStartupIntensityEnabled())
            func->adjustAttribute(startupIntensity() * val, Function::Intensity);
        else
            func->adjustAttribute(val, Function::Intensity);
    }

    VCWidget::adjustIntensity(val);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCButton::loadXML(const QDomElement* root)
{
    bool visible = false;
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    QDomNode node;
    QDomElement tag;
    QString str;

    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCVCButton)
    {
        qWarning() << Q_FUNC_INFO << "Button node not found";
        return false;
    }

    /* Widget commons */
    loadXMLCommon(root);

    /* Icon */
    setIconPath(m_doc->denormalizeComponentPath(root->attribute(KXMLQLCVCButtonIcon)));

    /* Children */
    node = root->firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();
        if (tag.tagName() == KXMLQLCWindowState)
        {
            loadXMLWindowState(&tag, &x, &y, &w, &h, &visible);
            setGeometry(x, y, w, h);
        }
        else if (tag.tagName() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(&tag);
        }
        else if (tag.tagName() == KXMLQLCVCButtonFunction)
        {
            str = tag.attribute(KXMLQLCVCButtonFunctionID);
            setFunction(str.toUInt());
        }
        else if (tag.tagName() == KXMLQLCVCWidgetInput)
        {
            loadXMLInput(&tag);
        }
        else if (tag.tagName() == KXMLQLCVCButtonAction)
        {
            setAction(stringToAction(tag.text()));
            if (tag.hasAttribute(KXMLQLCVCButtonStopAllFadeTime))
                setStopAllFadeOutTime(tag.attribute(KXMLQLCVCButtonStopAllFadeTime).toInt());
        }
        else if (tag.tagName() == KXMLQLCVCButtonKey)
        {
            setKeySequence(stripKeySequence(QKeySequence(tag.text())));
        }
        else if (tag.tagName() == KXMLQLCVCButtonIntensity)
        {
            bool adjust;
            if (tag.attribute(KXMLQLCVCButtonIntensityAdjust) == KXMLQLCTrue)
                adjust = true;
            else
                adjust = false;
            setStartupIntensity(qreal(tag.text().toInt()) / qreal(100));
            enableStartupIntensity(adjust);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown button tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    /* All buttons start raised... */
    setOn(false);

    return true;
}

bool VCButton::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
    QDomElement root;
    QDomElement tag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(vc_root != NULL);

    /* VC button entry */
    root = doc->createElement(KXMLQLCVCButton);
    vc_root->appendChild(root);

    saveXMLCommon(doc, &root);

    /* Icon */
    root.setAttribute(KXMLQLCVCButtonIcon, m_doc->normalizeComponentPath(iconPath()));

    /* Function */
    tag = doc->createElement(KXMLQLCVCButtonFunction);
    root.appendChild(tag);
    str.setNum(function());
    tag.setAttribute(KXMLQLCVCButtonFunctionID, str);

    /* Action */
    tag = doc->createElement(KXMLQLCVCButtonAction);
    root.appendChild(tag);
    text = doc->createTextNode(actionToString(action()));
    if (action() == StopAll && stopAllFadeTime() != 0)
    {
        tag.setAttribute(KXMLQLCVCButtonStopAllFadeTime, stopAllFadeTime());
    }
    tag.appendChild(text);

    /* Key sequence */
    if (m_keySequence.isEmpty() == false)
    {
        tag = doc->createElement(KXMLQLCVCButtonKey);
        root.appendChild(tag);
        text = doc->createTextNode(m_keySequence.toString());
        tag.appendChild(text);
    }

    /* Intensity adjustment */
    tag = doc->createElement(KXMLQLCVCButtonIntensity);
    tag.setAttribute(KXMLQLCVCButtonIntensityAdjust,
                     isStartupIntensityEnabled() ? KXMLQLCTrue : KXMLQLCFalse);
    root.appendChild(tag);
    text = doc->createTextNode(QString::number(int(startupIntensity() * 100)));
    tag.appendChild(text);

    /* External input */
    saveXMLInput(doc, &root);

    /* Window state */
    saveXMLWindowState(doc, &root);

    /* Appearance */
    saveXMLAppearance(doc, &root);

    return true;
}

/*****************************************************************************
 * Event handlers
 *****************************************************************************/

void VCButton::paintEvent(QPaintEvent* e)
{
    QStyleOptionButton option;
    option.initFrom(this);

    /* This should look like a normal button */
    option.features = QStyleOptionButton::None;

    /* Sunken or raised based on isOn() status */
    if (isOn() == true)
        option.state = QStyle::State_Sunken;
    else
        option.state = QStyle::State_Raised;

    /* Custom icons are always enabled, to see them in full color also in design mode */
    if (m_action == Toggle || m_action == Flash)
        option.state |= QStyle::State_Enabled;

    /* Icon */
    option.icon = m_icon;
    option.iconSize = m_iconSize;

    /* Paint the button */
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    style()->drawControl(QStyle::CE_PushButton, &option, &painter, this);

    /* Paint caption with text wrapping */
    if (caption().isEmpty() == false)
    {
        style()->drawItemText(&painter,
                              rect(),
                              Qt::AlignCenter | Qt::TextWordWrap,
                              palette(),
                              (mode() == Doc::Operate),
                              caption());
    }

    /* Flash emblem */
    if (m_action == Flash)
    {
        QIcon icon(":/flash.png");
        painter.drawPixmap(rect().width() - 18, 2,
                           icon.pixmap(QSize(16, 16), QIcon::Normal, QIcon::On));
    }

    if (m_ledStyle == true)
    {
        painter.setPen(QPen(QColor(160, 160, 160, 255), 2));

        if (isOn() == true)
            painter.setBrush(QBrush(QColor(0, 230, 0, 255)));
        else
            painter.setBrush(QBrush(QColor(110, 110, 110, 255)));

        int dim = rect().width() / 6;
        if (dim > 14) dim = 14;

        painter.drawEllipse(6, 6, dim, dim);      // Style #1
        //painter.drawRoundedRect(-1, -1, dim, dim, 3, 3);   // Style #2
    }
    else
    {
        // Style #3
        painter.setBrush(Qt::NoBrush);

        if (isOn() == true)
        {
            int borderWidth = (rect().width() > 80)?3:2;
            painter.setPen(QPen(QColor(20, 20, 20, 255), borderWidth * 2));
            painter.drawRoundedRect(borderWidth, borderWidth,
                                    rect().width() - borderWidth * 2, rect().height() - (borderWidth * 2),
                                    borderWidth + 1,  borderWidth + 1);
            painter.setPen(QPen(QColor(0, 230, 0, 255), borderWidth));
            painter.drawRoundedRect(borderWidth, borderWidth,
                                    rect().width() - borderWidth * 2, rect().height() - (borderWidth * 2),
                                    borderWidth, borderWidth);
        }
        else
        {
            painter.setPen(QPen(QColor(160, 160, 160, 255), 3));
            painter.drawRoundedRect(1, 1, rect().width() - 2, rect().height() - 2, 3, 3);
        }
    }

    /* Stop painting here */
    painter.end();

    /* Draw a selection frame if appropriate */
    VCWidget::paintEvent(e);
}

void VCButton::mousePressEvent(QMouseEvent* e)
{
    if (mode() == Doc::Design)
        VCWidget::mousePressEvent(e);
    else if (e->button() == Qt::LeftButton)
        pressFunction();
    else if (e->button() == Qt::RightButton)
    {
        Function* func = m_doc->function(m_function);
        if (func != NULL)
        {
            QString menuStyle = "QMenu { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #B9D9E8, stop:1 #A4C0CE);"
                            "border: 1px solid black; border-radius: 4px; font:bold; }";
            QMenu *menu = new QMenu();
            menu->setStyleSheet(menuStyle);
            int idx = 0;
            foreach(Attribute attr, func->attributes())
            {
                QString slStyle = "QSlider::groove:horizontal { border: 1px solid #999999; margin: 0; border-radius: 2px;"
                        "height: 15px; background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B1B1B1, stop:1 #c4c4c4); }"

                        "QSlider::handle:horizontal {"
                        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f);"
                        "border: 1px solid #5c5c5c; width: 15px; border-radius: 2px; margin: -1px 0; }"

                        "QSlider::sub-page:horizontal { background: #114EA2; border-radius: 2px; }";

                QWidget *entryWidget = new QWidget();
                QHBoxLayout *hbox = new QHBoxLayout(menu);
                hbox->setMargin(3);
                QLabel *label = new QLabel(attr.name);
                label->setAlignment(Qt::AlignLeft);
                label->setFixedWidth(100);
                ClickAndGoSlider *slider = new ClickAndGoSlider(menu);
                slider->setOrientation(Qt::Horizontal);
                slider->setStyleSheet(slStyle);
                slider->setFixedSize(QSize(100, 18));
                slider->setMinimum(0);
                slider->setMaximum(100);
                slider->setValue(attr.value * 100);
                slider->setProperty("attrIdx", QVariant(idx));
                connect(slider, SIGNAL(valueChanged(int)), this, SLOT(slotAttributeChanged(int)));
                hbox->addWidget(label);
                hbox->addWidget(slider);
                entryWidget->setLayout(hbox);
                QWidgetAction *sliderBoxAction = new QWidgetAction(menu);
                sliderBoxAction->setDefaultWidget(entryWidget);
                menu->addAction(sliderBoxAction);
                idx++;
            }
            menu->exec(QCursor::pos());
        }
    }
}

void VCButton::mouseReleaseEvent(QMouseEvent* e)
{
    if (mode() == Doc::Design)
        VCWidget::mouseReleaseEvent(e);
    else
        releaseFunction();
}
