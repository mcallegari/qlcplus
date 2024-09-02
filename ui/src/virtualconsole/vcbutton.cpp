/*
  Q Light Controller Plus
  vcbutton.cpp

  Copyright (c) Heikki Junnila
                Massimo Callegari

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
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QWidgetAction>
#include <QColorDialog>
#include <QImageReader>
#include <QFileDialog>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QMessageBox>
#include <QByteArray>
#include <QSettings>
#include <QPainter>
#include <QString>
#include <QDebug>
#include <QEvent>
#include <QTimer>
#include <QBrush>
#include <QStyle>
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
#include "virtualconsole.h"
#include "chaseraction.h"
#include "mastertimer.h"
#include "vcsoloframe.h"
#include "vcbutton.h"
#include "function.h"
#include "apputil.h"
#include "chaser.h"
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
    , m_flashOverrides(false)
    , m_flashForceLTP(false)
{
    /* Set the class name "VCButton" as the object name as well */
    setObjectName(VCButton::staticMetaObject.className());

    /* No function is initially attached to the button */
    m_function = Function::invalidId();

    setType(VCWidget::ButtonWidget);
    setCaption(QString());
    setState(Inactive);
    m_action = Action(-1); // avoid use of uninitialized value
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
    setStopAllFadeOutTime(button->stopAllFadeTime());
    setAction(button->action());
    m_state = button->m_state;

    m_flashForceLTP = button->flashForceLTP();
    m_flashOverrides = button->flashOverrides();

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
    m_bgPixmap = QPixmap(path);
    m_backgroundImage = path;
    m_doc->setModified();
    update();
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
    VirtualConsole *vc = VirtualConsole::instance();
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
        foreach (VCWidget *widget, vc->selectedWidgets())
        {
            VCButton *button = qobject_cast<VCButton*> (widget);
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

void VCButton::adjustFunctionIntensity(Function *f, qreal value)
{
    qreal finalValue = isStartupIntensityEnabled() ? startupIntensity() * value : value;

    VCWidget::adjustFunctionIntensity(f, finalValue);
}

void VCButton::notifyFunctionStarting(quint32 fid, qreal intensity)
{
    Q_UNUSED(intensity);

    if (mode() == Doc::Design)
        return;

    if (fid == m_function)
        return;

    if (m_function != Function::invalidId() && action() == VCButton::Toggle)
    {
        Function *f = m_doc->function(m_function);
        if (f != NULL)
        {
            f->stop(functionParent());
            resetIntensityOverrideAttribute();
        }
    }
}

void VCButton::slotFunctionRemoved(quint32 fid)
{
    /* Invalidate the button's function if it's the one that was removed */
    if (fid == m_function)
    {
        setFunction(Function::invalidId());
        resetIntensityOverrideAttribute();
    }
}

/*****************************************************************************
 * Button state
 *****************************************************************************/

VCButton::ButtonState VCButton::state() const
{
    return m_state;
}

void VCButton::setState(ButtonState state)
{
    if (state == m_state)
        return;

    m_state = state;

    emit stateChanged(m_state);

    updateFeedback();

    update();
}

void VCButton::updateState()
{
    ButtonState state = Inactive;

    if (m_action == Blackout)
    {
        if (m_doc->inputOutputMap()->blackout())
            state = Active;
    }
    else if (m_action == Toggle)
    {
        Function* function = m_doc->function(m_function);
        if (function != NULL && function->isRunning())
            state = Active;
    }

    if (m_state != state)
        setState(state);
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
    if (acceptsInput() == false)
        return;

    if (m_keySequence == keySequence)
        pressFunction();
}

void VCButton::slotKeyReleased(const QKeySequence& keySequence)
{
    if (acceptsInput() == false)
        return;

    if (m_keySequence == keySequence)
        releaseFunction();
}

void VCButton::updateFeedback()
{
    //if (m_state == Monitoring)
    //    return;

    QSharedPointer<QLCInputSource> src = inputSource();
    if (!src.isNull() && src->isValid() == true)
    {
        if (m_state == Inactive)
            sendFeedback(src->feedbackValue(QLCInputFeedback::LowerValue), src, src->feedbackExtraParams(QLCInputFeedback::LowerValue));
        else if (m_state == Monitoring)
            sendFeedback(src->feedbackValue(QLCInputFeedback::MonitorValue), src, src->feedbackExtraParams(QLCInputFeedback::MonitorValue));
        else
            sendFeedback(src->feedbackValue(QLCInputFeedback::UpperValue), src, src->feedbackExtraParams(QLCInputFeedback::UpperValue));
    }
}

/*****************************************************************************
 * External input
 *****************************************************************************/

void VCButton::slotInputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    /* Don't let input data through in design mode or if disabled */
    if (acceptsInput() == false)
        return;

    if (checkInputSource(universe, (page() << 16) | channel, value, sender()))
    {
        if (m_action == Flash)
        {
            // Keep the button depressed only while the external button is kept down.
            // Raise the button when the external button is raised.
            if (state() == Inactive && value > 0)
                pressFunction();
            else if (state() == Active && value == 0)
                releaseFunction();
        }
        else
        {
            if (value > 0)
            {
                // Only toggle when the external button is pressed.
                pressFunction();
            }
            else
            {
                // Work around the "internal" feedback of some controllers
                // by updating feedback state after button release.
                updateFeedback();
            }
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

int VCButton::stopAllFadeTime() const
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
#if 0
    ClickAndGoSlider *slider = (ClickAndGoSlider *)sender();
    int idx = slider->property("attrIdx").toInt();

    Function* func = m_doc->function(m_function);
    if (func != NULL)
        func->adjustAttribute((qreal)value / 100, idx);
#else
    Q_UNUSED(value)
#endif
}



/*****************************************************************************
 * Flash Properties
 *****************************************************************************/

bool VCButton::flashOverrides() const
{
    return m_flashOverrides;
}

void VCButton::setFlashOverride(bool shouldOverride)
{
    m_flashOverrides = shouldOverride;
}

bool VCButton::flashForceLTP() const
{
    return m_flashForceLTP;
}

void VCButton::setFlashForceLTP(bool forceLTP)
{
    m_flashForceLTP = forceLTP;
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

        // if the button is in a SoloFrame and the function is running but was
        // started by a different function (a chaser or collection), turn other
        // functions off and start this one.
        if (state() == Active && !(isChildOfSoloFrame() && f->startedAsChild()))
        {
            f->stop(functionParent());
            resetIntensityOverrideAttribute();
        }
        else
        {
            adjustFunctionIntensity(f, intensity());

            // starting a Chaser is a special case, since it is necessary
            // to use Chaser Actions to properly start the first
            // Chaser step with the right intensity
            if (f->type() == Function::ChaserType || f->type() == Function::SequenceType)
            {
                ChaserAction action;
                action.m_action = ChaserSetStepIndex;
                action.m_stepIndex = 0;
                action.m_masterIntensity = intensity();
                action.m_stepIntensity = 1.0;
                action.m_fadeMode = Chaser::FromFunction;

                Chaser *chaser = qobject_cast<Chaser*>(f);
                chaser->setAction(action);
            }

            f->start(m_doc->masterTimer(), functionParent());
            setState(Active);
            emit functionStarting(m_function);
        }
    }
    else if (m_action == Flash && state() == Inactive)
    {
        f = m_doc->function(m_function);
        if (f != NULL)
        {
            adjustFunctionIntensity(f, intensity());
            f->flash(m_doc->masterTimer(), flashOverrides(), flashForceLTP());
            setState(Active);
        }
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

FunctionParent VCButton::functionParent() const
{
    return FunctionParent(FunctionParent::ManualVCWidget, id());
}

void VCButton::releaseFunction()
{
    /* Don't allow operation during design mode */
    if (mode() == Doc::Design)
        return;

    if (m_action == Flash && state() == Active)
    {
        Function* f = m_doc->function(m_function);
        if (f != NULL)
        {
            f->unFlash(m_doc->masterTimer());
            resetIntensityOverrideAttribute();
            setState(Inactive);
        }
    }
}

void VCButton::slotFunctionRunning(quint32 fid)
{
    if (fid == m_function && m_action == Toggle)
    {
        if (state() == Inactive)
            setState(Monitoring);
        emit functionStarting(m_function);
    }
}

void VCButton::slotFunctionStopped(quint32 fid)
{
    if (fid == m_function && m_action == Toggle)
    {
        resetIntensityOverrideAttribute();
        setState(Inactive);
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

    setState(state ? Active : Inactive);
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
    setState(state ? Active : Inactive);
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
    if (state() == Active)
    {
        Function* func = m_doc->function(m_function);
        if (func != NULL)
            adjustFunctionIntensity(func, val);
    }

    VCWidget::adjustIntensity(val);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCButton::loadXML(QXmlStreamReader &root)
{
    bool visible = false;
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    if (root.name() != KXMLQLCVCButton)
    {
        qWarning() << Q_FUNC_INFO << "Button node not found";
        return false;
    }

    /* Widget commons */
    loadXMLCommon(root);

    /* Icon */
    setIconPath(m_doc->denormalizeComponentPath(root.attributes().value(KXMLQLCVCButtonIcon).toString()));

    /* Children */
    while (root.readNextStartElement())
    {
        //qDebug() << "VC Button tag:" << root.name();
        if (root.name() == KXMLQLCWindowState)
        {
            loadXMLWindowState(root, &x, &y, &w, &h, &visible);
            setGeometry(x, y, w, h);
        }
        else if (root.name() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(root);
        }
        else if (root.name() == KXMLQLCVCButtonFunction)
        {
            QString str = root.attributes().value(KXMLQLCVCButtonFunctionID).toString();
            setFunction(str.toUInt());
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLQLCVCWidgetInput)
        {
            loadXMLInput(root);
        }
        else if (root.name() == KXMLQLCVCButtonAction)
        {
            QXmlStreamAttributes attrs = root.attributes();
            setAction(stringToAction(root.readElementText()));
            if (attrs.hasAttribute(KXMLQLCVCButtonStopAllFadeTime))
                setStopAllFadeOutTime(attrs.value(KXMLQLCVCButtonStopAllFadeTime).toString().toInt());

            if (attrs.hasAttribute(KXMLQLCVCButtonFlashOverride))
                setFlashOverride(attrs.value(KXMLQLCVCButtonFlashOverride).toInt());

            if (attrs.hasAttribute(KXMLQLCVCButtonFlashForceLTP))
                setFlashForceLTP(attrs.value(KXMLQLCVCButtonFlashForceLTP).toInt());
        }
        else if (root.name() == KXMLQLCVCButtonKey)
        {
            setKeySequence(stripKeySequence(QKeySequence(root.readElementText())));
        }
        else if (root.name() == KXMLQLCVCButtonIntensity)
        {
            bool adjust;
            if (root.attributes().value(KXMLQLCVCButtonIntensityAdjust).toString() == KXMLQLCTrue)
                adjust = true;
            else
                adjust = false;
            setStartupIntensity(qreal(root.readElementText().toInt()) / qreal(100));
            enableStartupIntensity(adjust);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown button tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    /* All buttons start raised... */
    setState(Inactive);

    return true;
}

bool VCButton::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* VC button entry */
    doc->writeStartElement(KXMLQLCVCButton);

    saveXMLCommon(doc);

    /* Icon */
    doc->writeAttribute(KXMLQLCVCButtonIcon, m_doc->normalizeComponentPath(iconPath()));

    /* Window state */
    saveXMLWindowState(doc);

    /* Appearance */
    saveXMLAppearance(doc);

    /* Function */
    doc->writeStartElement(KXMLQLCVCButtonFunction);
    doc->writeAttribute(KXMLQLCVCButtonFunctionID, QString::number(function()));
    doc->writeEndElement();

    /* Action */
    doc->writeStartElement(KXMLQLCVCButtonAction);

    if (action() == StopAll && stopAllFadeTime() != 0)
    {
        doc->writeAttribute(KXMLQLCVCButtonStopAllFadeTime, QString::number(stopAllFadeTime()));
    }
    else if (action() == Flash)
    {
        doc->writeAttribute(KXMLQLCVCButtonFlashOverride, QString::number(flashOverrides()));
        doc->writeAttribute(KXMLQLCVCButtonFlashForceLTP, QString::number(flashForceLTP()));
    }
    doc->writeCharacters(actionToString(action()));
    doc->writeEndElement();

    /* Key sequence */
    if (m_keySequence.isEmpty() == false)
        doc->writeTextElement(KXMLQLCVCButtonKey, m_keySequence.toString());

    /* Intensity adjustment */
    doc->writeStartElement(KXMLQLCVCButtonIntensity);
    doc->writeAttribute(KXMLQLCVCButtonIntensityAdjust,
                     isStartupIntensityEnabled() ? KXMLQLCTrue : KXMLQLCFalse);
    doc->writeCharacters(QString::number(int(startupIntensity() * 100)));
    doc->writeEndElement();

    /* External input */
    saveXMLInput(doc);

    /* End the <Button> tag */
    doc->writeEndElement();

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

    /* Sunken or raised based on state() status */
    if (state() == Inactive)
        option.state = QStyle::State_Raised;
    else
        option.state = QStyle::State_Sunken;

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

    if (m_backgroundImage.isEmpty() == false)
    {
        QRect pxRect = m_bgPixmap.rect();
        // if the pixmap is bigger than the button, then paint a scaled version of it
        // covering the whole button surface
        // if the pixmap is smaller than the button, draw a centered pixmap
        if (pxRect.contains(rect()))
        {
            if (m_ledStyle == true)
                painter.drawPixmap(rect(), m_bgPixmap);
            else
                painter.drawPixmap(3, 3, width() - 6, height() - 6, m_bgPixmap);
        }
        else
        {
            painter.drawPixmap((width() - pxRect.width()) / 2,
                               (height() - pxRect.height()) / 2,
                               m_bgPixmap);
        }
    }

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

        if (state() == Active)
        {
            if (m_flashForceLTP || m_flashOverrides)
                painter.setBrush(QBrush(QColor(230, 0, 0, 255)));
            else
                painter.setBrush(QBrush(QColor(0, 230, 0, 255)));
        }
        else if (state() == Monitoring)
            painter.setBrush(QBrush(QColor(255, 170, 0, 255)));
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

        if (state() != Inactive)
        {
            int borderWidth = (rect().width() > 80)?3:2;
            painter.setPen(QPen(QColor(20, 20, 20, 255), borderWidth * 2));
            painter.drawRoundedRect(borderWidth, borderWidth,
                                    rect().width() - borderWidth * 2, rect().height() - (borderWidth * 2),
                                    borderWidth + 1,  borderWidth + 1);
            if (state() == Monitoring)
                painter.setPen(QPen(QColor(255, 170, 0, 255), borderWidth));
            else
            {
                if (m_flashForceLTP || m_flashOverrides)
                    painter.setPen(QPen(QColor(230, 0, 0, 255), borderWidth));
                else
                    painter.setPen(QPen(QColor(0, 230, 0, 255), borderWidth));
            }
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
#if 0
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
            foreach (Attribute attr, func->attributes())
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
                QLabel *label = new QLabel(attr.m_name);
                label->setAlignment(Qt::AlignLeft);
                label->setFixedWidth(100);
                ClickAndGoSlider *slider = new ClickAndGoSlider(menu);
                slider->setOrientation(Qt::Horizontal);
                slider->setSliderStyleSheet(slStyle);
                slider->setFixedSize(QSize(100, 18));
                slider->setMinimum(0);
                slider->setMaximum(100);
                slider->setValue(attr.m_value * 100);
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
#endif
}

void VCButton::mouseReleaseEvent(QMouseEvent* e)
{
    if (mode() == Doc::Design)
        VCWidget::mouseReleaseEvent(e);
    else
        releaseFunction();
}
