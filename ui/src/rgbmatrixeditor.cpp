/*
  Q Light Controller
  rgbmatrixeditor.cpp

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

#include <QGraphicsEllipseItem>
#include <QGraphicsRectItem>
#include <QGraphicsEffect>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QColorDialog>
#include <QFileDialog>
#include <QFontDialog>
#include <QGradient>
#include <QSettings>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QTimer>
#include <QDebug>
#include <QMutex>

#include "fixtureselection.h"
#include "speeddialwidget.h"
#include "rgbmatrixeditor.h"
#include "qlcfixturehead.h"
#include "qlcmacros.h"
#include "rgbimage.h"
#include "sequence.h"
#include "rgbitem.h"
#include "rgbtext.h"
#include "scene.h"

#define SETTINGS_GEOMETRY "rgbmatrixeditor/geometry"
#define RECT_SIZE 30
#define RECT_PADDING 0
#define ITEM_SIZE 28
#define ITEM_PADDING 2

/****************************************************************************
 * Initialization
 ****************************************************************************/

RGBMatrixEditor::RGBMatrixEditor(QWidget* parent, RGBMatrix* mtx, Doc* doc)
    : QWidget(parent)
    , m_doc(doc)
    , m_matrix(mtx)
    , m_previewHandler(new RGBMatrixStep())
    , m_speedDials(NULL)
    , m_scene(new QGraphicsScene(this))
    , m_previewTimer(new QTimer(this))
    , m_previewIterator(0)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(mtx != NULL);

    setupUi(this);

    // Set a nice gradient backdrop
    m_scene->setBackgroundBrush(Qt::darkGray);
    QLinearGradient gradient(200, 200, 200, 2000);
    gradient.setSpread(QGradient::ReflectSpread);
    m_scene->setBackgroundBrush(gradient);

    connect(m_previewTimer, SIGNAL(timeout()), this, SLOT(slotPreviewTimeout()));
    connect(m_doc, SIGNAL(modeChanged(Doc::Mode)), this, SLOT(slotModeChanged(Doc::Mode)));
    connect(m_doc, SIGNAL(fixtureGroupAdded(quint32)), this, SLOT(slotFixtureGroupAdded()));
    connect(m_doc, SIGNAL(fixtureGroupRemoved(quint32)), this, SLOT(slotFixtureGroupRemoved()));
    connect(m_doc, SIGNAL(fixtureGroupChanged(quint32)), this, SLOT(slotFixtureGroupChanged(quint32)));

    init();

    slotModeChanged(m_doc->mode());

    // Set focus to the editor
    m_nameEdit->setFocus();
}

RGBMatrixEditor::~RGBMatrixEditor()
{
    m_previewTimer->stop();

    if (m_testButton->isChecked() == true)
        m_matrix->stopAndWait();

    delete m_previewHandler;
}

void RGBMatrixEditor::stopTest()
{
    if (m_testButton->isChecked() == true)
        m_testButton->click();
}

void RGBMatrixEditor::slotFunctionManagerActive(bool active)
{
    if (active == true)
    {
        if (m_speedDials == NULL)
            updateSpeedDials();
    }
    else
    {
        if (m_speedDials != NULL)
            m_speedDials->deleteLater();
        m_speedDials = NULL;
    }
}

void RGBMatrixEditor::init()
{
    /* Name */
    m_nameEdit->setText(m_matrix->name());
    m_nameEdit->setSelection(0, m_matrix->name().length());

    /* Running order */
    switch (m_matrix->runOrder())
    {
        default:
        case Function::Loop:
            m_loop->setChecked(true);
        break;
        case Function::PingPong:
            m_pingPong->setChecked(true);
        break;
        case Function::SingleShot:
            m_singleShot->setChecked(true);
        break;
    }

    /* Running direction */
    switch (m_matrix->direction())
    {
        default:
        case Function::Forward:
            m_forward->setChecked(true);
        break;
        case Function::Backward:
            m_backward->setChecked(true);
        break;
    }


    /* Blend mode */
    m_blendModeCombo->setCurrentIndex(m_matrix->blendMode());

    /* Color mode */
    m_controlModeCombo->setCurrentIndex(m_matrix->controlMode());

    /* Dimmer control */
    if (m_matrix->dimmerControl())
        m_dimmerControlCb->setChecked(m_matrix->dimmerControl());
    else
        m_intensityGroup->hide();

    fillPatternCombo();
    fillFixtureGroupCombo();
    fillAnimationCombo();
    fillImageAnimationCombo();

    QPixmap pm(50, 26);
    pm.fill(m_matrix->getColor(0));
    m_mtxColor1Button->setIcon(QIcon(pm));

    if (m_matrix->getColor(1).isValid())
        pm.fill(m_matrix->getColor(1));
    else
        pm.fill(Qt::transparent);
    m_mtxColor2Button->setIcon(QIcon(pm));

    if (m_matrix->getColor(2).isValid())
        pm.fill(m_matrix->getColor(2));
    else
        pm.fill(Qt::transparent);
    m_mtxColor3Button->setIcon(QIcon(pm));

    if (m_matrix->getColor(3).isValid())
        pm.fill(m_matrix->getColor(3));
    else
        pm.fill(Qt::transparent);
    m_mtxColor4Button->setIcon(QIcon(pm));

    if (m_matrix->getColor(4).isValid())
        pm.fill(m_matrix->getColor(4));
    else
        pm.fill(Qt::transparent);
    m_mtxColor5Button->setIcon(QIcon(pm));

    updateExtraOptions();
    updateSpeedDials();

    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameEdited(const QString&)));
    connect(m_speedDialButton, SIGNAL(toggled(bool)),
            this, SLOT(slotSpeedDialToggle(bool)));
    connect(m_saveToSequenceButton, SIGNAL(clicked()),
            this, SLOT(slotSaveToSequenceClicked()));
    connect(m_shapeButton, SIGNAL(toggled(bool)),
            this, SLOT(slotShapeToggle(bool)));
    connect(m_patternCombo, SIGNAL(activated(int)),
            this, SLOT(slotPatternActivated(int)));
    connect(m_fixtureGroupCombo, SIGNAL(activated(int)),
            this, SLOT(slotFixtureGroupActivated(int)));
    connect(m_blendModeCombo, SIGNAL(activated(int)),
            this, SLOT(slotBlendModeChanged(int)));
    connect(m_controlModeCombo, SIGNAL(activated(int)),
            this, SLOT(slotControlModeChanged(int)));
    connect(m_mtxColor1Button, SIGNAL(clicked()),
            this, SLOT(slotMtxColor1ButtonClicked()));
    connect(m_mtxColor2Button, SIGNAL(clicked()),
            this, SLOT(slotMtxColor2ButtonClicked()));
    connect(m_resetMtxColor2Button, SIGNAL(clicked()),
            this, SLOT(slotResetMtxColor2ButtonClicked()));
    connect(m_mtxColor3Button, SIGNAL(clicked()),
            this, SLOT(slotMtxColor3ButtonClicked()));
    connect(m_resetMtxColor3Button, SIGNAL(clicked()),
            this, SLOT(slotResetMtxColor3ButtonClicked()));
    connect(m_mtxColor4Button, SIGNAL(clicked()),
            this, SLOT(slotMtxColor4ButtonClicked()));
    connect(m_resetMtxColor4Button, SIGNAL(clicked()),
            this, SLOT(slotResetMtxColor4ButtonClicked()));
    connect(m_mtxColor5Button, SIGNAL(clicked()),
            this, SLOT(slotMtxColor5ButtonClicked()));
    connect(m_resetMtxColor5Button, SIGNAL(clicked()),
            this, SLOT(slotResetMtxColor5ButtonClicked()));
    connect(m_textEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotTextEdited(const QString&)));
    connect(m_fontButton, SIGNAL(clicked()),
            this, SLOT(slotFontButtonClicked()));
    connect(m_animationCombo, SIGNAL(activated(int)),
            this, SLOT(slotAnimationActivated(int)));
    connect(m_imageEdit, SIGNAL(editingFinished()),
            this, SLOT(slotImageEdited()));
    connect(m_imageButton, SIGNAL(clicked()),
            this, SLOT(slotImageButtonClicked()));
    connect(m_imageAnimationCombo, SIGNAL(activated(int)),
            this, SLOT(slotImageAnimationActivated(int)));
    connect(m_xOffsetSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotOffsetSpinChanged()));
    connect(m_yOffsetSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotOffsetSpinChanged()));

    connect(m_loop, SIGNAL(clicked()), this, SLOT(slotLoopClicked()));
    connect(m_pingPong, SIGNAL(clicked()), this, SLOT(slotPingPongClicked()));
    connect(m_singleShot, SIGNAL(clicked()), this, SLOT(slotSingleShotClicked()));
    connect(m_forward, SIGNAL(clicked()), this, SLOT(slotForwardClicked()));
    connect(m_backward, SIGNAL(clicked()), this, SLOT(slotBackwardClicked()));
    connect(m_dimmerControlCb, SIGNAL(clicked()), this, SLOT(slotDimmerControlClicked()));

    // Test slots
    connect(m_testButton, SIGNAL(clicked(bool)),
            this, SLOT(slotTestClicked()));

    m_preview->setScene(m_scene);
    if (createPreviewItems() == true)
        m_previewTimer->start(MasterTimer::tick());
}

void RGBMatrixEditor::updateSpeedDials()
{
    if (m_speedDialButton->isChecked() == false)
        return;

    if (m_speedDials != NULL)
        return;

    m_speedDials = new SpeedDialWidget(this);
    m_speedDials->setAttribute(Qt::WA_DeleteOnClose);
    m_speedDials->setWindowTitle(m_matrix->name());
    m_speedDials->show();
    m_speedDials->setFadeInSpeed(m_matrix->fadeInSpeed());
    m_speedDials->setFadeOutSpeed(m_matrix->fadeOutSpeed());
    if ((int)m_matrix->duration() < 0)
        m_speedDials->setDuration(m_matrix->duration());
    else
        m_speedDials->setDuration(m_matrix->duration() - m_matrix->fadeInSpeed());
    connect(m_speedDials, SIGNAL(fadeInChanged(int)), this, SLOT(slotFadeInChanged(int)));
    connect(m_speedDials, SIGNAL(fadeOutChanged(int)), this, SLOT(slotFadeOutChanged(int)));
    connect(m_speedDials, SIGNAL(holdChanged(int)), this, SLOT(slotHoldChanged(int)));
    connect(m_speedDials, SIGNAL(holdTapped()), this, SLOT(slotDurationTapped()));
    connect(m_speedDials, SIGNAL(destroyed(QObject*)), this, SLOT(slotDialDestroyed(QObject*)));
}

void RGBMatrixEditor::fillPatternCombo()
{
    m_patternCombo->addItems(RGBAlgorithm::algorithms(m_doc));
    if (m_matrix->algorithm() != NULL)
    {
        int index = m_patternCombo->findText(m_matrix->algorithm()->name());
        if (index >= 0)
            m_patternCombo->setCurrentIndex(index);
    }
}

void RGBMatrixEditor::fillFixtureGroupCombo()
{
    m_fixtureGroupCombo->clear();
    m_fixtureGroupCombo->addItem(tr("None"));

    QListIterator <FixtureGroup*> it(m_doc->fixtureGroups());
    while (it.hasNext() == true)
    {
        FixtureGroup* grp(it.next());
        Q_ASSERT(grp != NULL);
        m_fixtureGroupCombo->addItem(grp->name(), grp->id());
        if (m_matrix->fixtureGroup() == grp->id())
            m_fixtureGroupCombo->setCurrentIndex(m_fixtureGroupCombo->count() - 1);
    }
}

void RGBMatrixEditor::fillAnimationCombo()
{
    m_animationCombo->addItems(RGBText::animationStyles());
}

void RGBMatrixEditor::fillImageAnimationCombo()
{
    m_imageAnimationCombo->addItems(RGBImage::animationStyles());
}

void RGBMatrixEditor::updateExtraOptions()
{
    resetProperties(m_propertiesLayout->layout());
    m_propertiesGroup->hide();

    if (m_matrix->algorithm() == NULL ||
        m_matrix->algorithm()->type() == RGBAlgorithm::Script ||
        m_matrix->algorithm()->type() == RGBAlgorithm::Audio)
    {
        m_textGroup->hide();
        m_imageGroup->hide();
        m_offsetGroup->hide();

        if (m_matrix->algorithm() != NULL && m_matrix->algorithm()->type() == RGBAlgorithm::Script)
        {
            RGBScript *script = static_cast<RGBScript*> (m_matrix->algorithm());
            displayProperties(script);
        }
    }
    else if (m_matrix->algorithm()->type() == RGBAlgorithm::Plain)
    {
        m_textGroup->hide();
        m_imageGroup->hide();
        m_offsetGroup->hide();
    }
    else if (m_matrix->algorithm()->type() == RGBAlgorithm::Image)
    {
        m_textGroup->hide();
        m_imageGroup->show();
        m_offsetGroup->show();

        RGBImage *image = static_cast<RGBImage*> (m_matrix->algorithm());
        Q_ASSERT(image != NULL);
        m_imageEdit->setText(image->filename());

        int index = m_imageAnimationCombo->findText(RGBImage::animationStyleToString(image->animationStyle()));
        if (index != -1)
            m_imageAnimationCombo->setCurrentIndex(index);

        m_xOffsetSpin->setValue(image->xOffset());
        m_yOffsetSpin->setValue(image->yOffset());

    }
    else if (m_matrix->algorithm()->type() == RGBAlgorithm::Text)
    {
        m_textGroup->show();
        m_offsetGroup->show();
        m_imageGroup->hide();

        RGBText *text = static_cast<RGBText*> (m_matrix->algorithm());
        Q_ASSERT(text != NULL);
        m_textEdit->setText(text->text());

        int index = m_animationCombo->findText(RGBText::animationStyleToString(text->animationStyle()));
        if (index != -1)
            m_animationCombo->setCurrentIndex(index);

        m_xOffsetSpin->setValue(text->xOffset());
        m_yOffsetSpin->setValue(text->yOffset());
    }

    updateColorOptions();
}

void RGBMatrixEditor::updateColorOptions()
{
    if (m_matrix->algorithm() != NULL)
    {
        int accColors = m_matrix->algorithm()->acceptColors();

        m_mtxColor1Button->setVisible(accColors == 0 ? false : true);
        m_mtxColor2Button->setVisible(accColors > 1 ? true : false);
        m_resetMtxColor2Button->setVisible(accColors > 1 ? true : false);
        m_mtxColor3Button->setVisible(accColors > 2 ? true : false);
        m_resetMtxColor3Button->setVisible(accColors > 2 ? true : false);
        m_mtxColor4Button->setVisible(accColors > 3 ? true : false);
        m_resetMtxColor4Button->setVisible(accColors > 3 ? true : false);
        m_mtxColor5Button->setVisible(accColors > 4 ? true : false);
        m_resetMtxColor5Button->setVisible(accColors > 4 ? true : false);

        m_blendModeLabel->setVisible(accColors == 0 ? false : true);
        m_blendModeCombo->setVisible(accColors == 0 ? false : true);
    }
}

void RGBMatrixEditor::updateColors()
{
    if (m_matrix->algorithm() != NULL)
    {
        int accColors = m_matrix->algorithm()->acceptColors();
        if (accColors > 0)
        {
            if (m_matrix->blendMode() == Universe::MaskBlend)
            {
                m_matrix->setColor(0, Qt::white);
                // Overwrite more colors only if applied.
                if (accColors <= 2)
                    m_matrix->setColor(1, QColor());
                if (accColors <= 3)
                    m_matrix->setColor(2, QColor());
                if (accColors <= 4)
                    m_matrix->setColor(3, QColor());
                if (accColors <= 5)
                    m_matrix->setColor(4, QColor());

                m_previewHandler->calculateColorDelta(m_matrix->getColor(0), m_matrix->getColor(1),
                        m_matrix->algorithm());

                QPixmap pm(50, 26);
                pm.fill(Qt::white);
                m_mtxColor1Button->setIcon(QIcon(pm));

                pm.fill(Qt::transparent);
                m_mtxColor2Button->setIcon(QIcon(pm));
                m_mtxColor3Button->setIcon(QIcon(pm));
                m_mtxColor4Button->setIcon(QIcon(pm));
                m_mtxColor5Button->setIcon(QIcon(pm));
            }
            else if (m_controlModeCombo->currentIndex() != RGBMatrix::ControlModeRgb)
            {
                // Convert color 1 to grayscale for single color modes
                uchar gray = qGray(m_matrix->getColor(0).rgb());
                m_matrix->setColor(0, QColor(gray, gray, gray));
                QPixmap pm(50, 26);
                pm.fill(QColor(gray, gray, gray));
                m_mtxColor1Button->setIcon(QIcon(pm));

                // Convert color 2 and following to grayscale for single color modes
                if (accColors < 2)
                    m_matrix->setColor(1, QColor());
                if (m_matrix->getColor(1) == QColor())
                {
                    pm.fill(Qt::transparent);
                }
                else
                {
                    gray = qGray(m_matrix->getColor(1).rgb());
                    m_matrix->setColor(1, QColor(gray, gray, gray));
                    pm.fill(QColor(gray, gray, gray));
                }
                m_mtxColor2Button->setIcon(QIcon(pm));

                if (accColors < 3)
                    m_matrix->setColor(2, QColor());
                if (m_matrix->getColor(2) == QColor())
                {
                    pm.fill(Qt::transparent);
                }
                else
                {
                    gray = qGray(m_matrix->getColor(2).rgb());
                    m_matrix->setColor(2, QColor(gray, gray, gray));
                    pm.fill(QColor(gray, gray, gray));
                }
                m_mtxColor3Button->setIcon(QIcon(pm));

                if (accColors < 4)
                    m_matrix->setColor(3, QColor());
                if (m_matrix->getColor(3) == QColor())
                {
                    pm.fill(Qt::transparent);
                }
                else
                {
                    gray = qGray(m_matrix->getColor(3).rgb());
                    m_matrix->setColor(3, QColor(gray, gray, gray));
                    pm.fill(QColor(gray, gray, gray));
                }
                m_mtxColor4Button->setIcon(QIcon(pm));

                if (accColors < 5)
                    m_matrix->setColor(4, QColor());
                if (m_matrix->getColor(4) == QColor())
                {
                    pm.fill(Qt::transparent);
                }
                else
                {
                    gray = qGray(m_matrix->getColor(4).rgb());
                    m_matrix->setColor(4, QColor(gray, gray, gray));
                    pm.fill(QColor(gray, gray, gray));
                }
                m_mtxColor5Button->setIcon(QIcon(pm));

                m_previewHandler->calculateColorDelta(m_matrix->getColor(0), m_matrix->getColor(1),
                        m_matrix->algorithm());
            }
            else 
            {
                QPixmap pm(50, 26);
                pm.fill(m_matrix->getColor(0));
                m_mtxColor1Button->setIcon(QIcon(pm));

                // Preserve the colors (do not set them to QColor().
                // RGBMatrixStep::calculateColorDelta will ensure correct color application
                if (m_matrix->getColor(1) == QColor())
                    pm.fill(Qt::transparent);
                else
                    pm.fill(m_matrix->getColor(1));
                m_mtxColor2Button->setIcon(QIcon(pm));

                if (m_matrix->getColor(2) == QColor())
                    pm.fill(Qt::transparent);
                else
                    pm.fill(m_matrix->getColor(2));
                m_mtxColor3Button->setIcon(QIcon(pm));

                if (m_matrix->getColor(3) == QColor())
                    pm.fill(Qt::transparent);
                else
                    pm.fill(m_matrix->getColor(3));
                m_mtxColor4Button->setIcon(QIcon(pm));

                if (m_matrix->getColor(4) == QColor())
                    pm.fill(Qt::transparent);
                else
                    pm.fill(m_matrix->getColor(4));
                m_mtxColor5Button->setIcon(QIcon(pm));

                m_previewHandler->calculateColorDelta(m_matrix->getColor(0), m_matrix->getColor(1),
                        m_matrix->algorithm());
            }
        }
    }
}

/**
 * Helper function. Deletes all child widgets of the given layout @a item.
 */
void RGBMatrixEditor::resetProperties(QLayoutItem *item)
{
    if (item->layout()) 
    {
        // Process all child items recursively.
        for (int i = item->layout()->count() - 1; i >= 0; i--)
            resetProperties(item->layout()->itemAt(i));
    }
    delete item->widget();
}

void RGBMatrixEditor::displayProperties(RGBScript *script)
{
    if (script == NULL)
        return;

    int gridRowIdx = 0;

    QList<RGBScriptProperty> properties = script->properties();
    if (properties.count() > 0)
        m_propertiesGroup->show();

    foreach (RGBScriptProperty prop, properties)
    {
        switch(prop.m_type)
        {
            case RGBScriptProperty::List:
            {
                QLabel *propLabel = new QLabel(prop.m_displayName);
                m_propertiesLayout->addWidget(propLabel, gridRowIdx, 0);
                QComboBox *propCombo = new QComboBox(this);
                propCombo->addItems(prop.m_listValues);
                propCombo->setProperty("pName", prop.m_name);
                connect(propCombo, SIGNAL(currentIndexChanged(int)),
                        this, SLOT(slotPropertyComboChanged(int)));
                m_propertiesLayout->addWidget(propCombo, gridRowIdx, 1);
                if (m_matrix != NULL)
                {
                    QString pValue = m_matrix->property(prop.m_name);
                    if (!pValue.isEmpty())
                    {
                        propCombo->setCurrentText(pValue);
                    }
                    else
                    {
                        pValue = script->property(prop.m_name);
                        if (!pValue.isEmpty())
                            propCombo->setCurrentText(pValue);
                    }
                }
                gridRowIdx++;
            }
            break;
            case RGBScriptProperty::Range:
            {
                QLabel *propLabel = new QLabel(prop.m_displayName);
                m_propertiesLayout->addWidget(propLabel, gridRowIdx, 0);
                QSpinBox *propSpin = new QSpinBox(this);
                propSpin->setRange(prop.m_rangeMinValue, prop.m_rangeMaxValue);
                propSpin->setProperty("pName", prop.m_name);
                connect(propSpin, SIGNAL(valueChanged(int)),
                        this, SLOT(slotPropertySpinChanged(int)));
                m_propertiesLayout->addWidget(propSpin, gridRowIdx, 1);
                if (m_matrix != NULL)
                {
                    QString pValue = m_matrix->property(prop.m_name);
                    if (!pValue.isEmpty())
                        propSpin->setValue(pValue.toInt());
                    else
                    {
                        pValue = script->property(prop.m_name);
                        if (!pValue.isEmpty())
                            propSpin->setValue(pValue.toInt());
                    }
                }
                gridRowIdx++;
            }
            break;
            case RGBScriptProperty::Float:
            {
                QLabel *propLabel = new QLabel(prop.m_displayName);
                m_propertiesLayout->addWidget(propLabel, gridRowIdx, 0);
                QDoubleSpinBox *propSpin = new QDoubleSpinBox(this);
                propSpin->setDecimals(3);
                propSpin->setRange(-1000000, 1000000);
                propSpin->setProperty("pName", prop.m_name);
                connect(propSpin, SIGNAL(valueChanged(double)),
                        this, SLOT(slotPropertyDoubleSpinChanged(double)));
                m_propertiesLayout->addWidget(propSpin, gridRowIdx, 1);
                if (m_matrix != NULL)
                {
                    QString pValue = m_matrix->property(prop.m_name);
                    if (!pValue.isEmpty())
                        propSpin->setValue(pValue.toDouble());
                    else
                    {
                        pValue = script->property(prop.m_name);
                        if (!pValue.isEmpty())
                            propSpin->setValue(pValue.toDouble());
                    }
                }
                gridRowIdx++;
            }
            break;
            case RGBScriptProperty::String:
            {
                QLabel *propLabel = new QLabel(prop.m_displayName);
                m_propertiesLayout->addWidget(propLabel, gridRowIdx, 0);
                QLineEdit *propEdit = new QLineEdit(this);
                propEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
                propEdit->setProperty("pName", prop.m_name);
                connect(propEdit, SIGNAL(textEdited(QString)),
                        this, SLOT(slotPropertyEditChanged(QString)));
                m_propertiesLayout->addWidget(propEdit, gridRowIdx, 1);
                if (m_matrix != NULL)
                {
                    QString pValue = m_matrix->property(prop.m_name);
                    if (!pValue.isEmpty())
                        propEdit->setText(pValue);
                    else
                    {
                        pValue = script->property(prop.m_name);
                        if (!pValue.isEmpty())
                            propEdit->setText(pValue);
                    }
                }
                gridRowIdx++;
            }
            break;
            default:
                qWarning() << "Type" << prop.m_type << "not handled yet";
            break;
        }
    }
}

bool RGBMatrixEditor::createPreviewItems()
{
    m_previewHash.clear();
    m_scene->clear();

    FixtureGroup* grp = m_doc->fixtureGroup(m_matrix->fixtureGroup());
    if (grp == NULL)
    {
        QGraphicsTextItem* text = new QGraphicsTextItem(tr("No fixture group to control"));
        text->setDefaultTextColor(Qt::white);
        m_scene->addItem(text);
        return false;
    }

    m_previewHandler->initializeDirection(m_matrix->direction(), m_matrix->getColor(0),
                                      m_matrix->getColor(1), m_matrix->stepsCount(),
                                      m_matrix->algorithm());

    m_matrix->previewMap(m_previewHandler->currentStepIndex(), m_previewHandler);

    if (m_previewHandler->m_map.isEmpty())
        return false;

    for (int x = 0; x < grp->size().width(); x++)
    {
        for (int y = 0; y < grp->size().height(); y++)
        {
            QLCPoint pt(x, y);

            if (grp->headsMap().contains(pt) == true)
            {
                RGBItem *item;
                if (m_shapeButton->isChecked() == false)
                {
                    QGraphicsEllipseItem* circleItem = new QGraphicsEllipseItem();
                    circleItem->setRect(
                            x * RECT_SIZE + RECT_PADDING + ITEM_PADDING,
                            y * RECT_SIZE + RECT_PADDING + ITEM_PADDING,
                            ITEM_SIZE - (2 * ITEM_PADDING),
                            ITEM_SIZE - (2 * ITEM_PADDING));
                    item = new RGBItem(circleItem);
                }
                else
                {
                    QGraphicsRectItem *rectItem = new QGraphicsRectItem();
                    rectItem->setRect(
                            x * RECT_SIZE + RECT_PADDING + ITEM_PADDING,
                            y * RECT_SIZE + RECT_PADDING + ITEM_PADDING,
                            ITEM_SIZE - (2 * ITEM_PADDING),
                            ITEM_SIZE - (2 * ITEM_PADDING));
                    item = new RGBItem(rectItem);
                }

                item->setColor(m_previewHandler->m_map[y][x]);
                item->draw(0, 0);
                m_scene->addItem(item->graphicsItem());
                m_previewHash[pt] = item;
            }
        }
    }
    return true;
}

void RGBMatrixEditor::slotPreviewTimeout()
{
    if (m_matrix->duration() <= 0)
        return;

    m_previewIterator += MasterTimer::tick();
    uint elapsed = 0;
    while (m_previewIterator >= MAX(m_matrix->duration(), MasterTimer::tick()))
    {
        m_previewHandler->checkNextStep(m_matrix->runOrder(), m_matrix->getColor(0),
                                        m_matrix->getColor(1), m_matrix->stepsCount());

        m_matrix->previewMap(m_previewHandler->currentStepIndex(), m_previewHandler);

        m_previewIterator -= MAX(m_matrix->duration(), MasterTimer::tick());
        elapsed += MAX(m_matrix->duration(), MasterTimer::tick());
    }
    for (int y = 0; y < m_previewHandler->m_map.size(); y++)
    {
        for (int x = 0; x < m_previewHandler->m_map[y].size(); x++)
        {
            QLCPoint pt(x, y);
            if (m_previewHash.contains(pt) == true)
            {
                RGBItem* shape = m_previewHash[pt];
                if (shape->color() != QColor(m_previewHandler->m_map[y][x]).rgb())
                    shape->setColor(m_previewHandler->m_map[y][x]);

                if (shape->color() == QColor(Qt::black).rgb())
                    shape->draw(elapsed, m_matrix->fadeOutSpeed());
                else
                    shape->draw(elapsed, m_matrix->fadeInSpeed());
            }
        }
    }
}

void RGBMatrixEditor::slotNameEdited(const QString& text)
{
    m_matrix->setName(text);
    if (m_speedDials != NULL)
        m_speedDials->setWindowTitle(text);
}

void RGBMatrixEditor::slotSpeedDialToggle(bool state)
{
    if (state == true)
        updateSpeedDials();
    else
    {
        if (m_speedDials != NULL)
            m_speedDials->deleteLater();
        m_speedDials = NULL;
    }
}

void RGBMatrixEditor::slotDialDestroyed(QObject *)
{
    m_speedDialButton->setChecked(false);
}

void RGBMatrixEditor::slotPatternActivated(int patternIndex)
{
    QString algoName = m_patternCombo->itemText(patternIndex);
    RGBAlgorithm *algo = RGBAlgorithm::algorithm(m_doc, algoName);
    m_matrix->setAlgorithm(algo);
    if (algo != NULL) {
        updateColors();
#if (5 != RGBAlgorithmColorDisplayCount)
#error "Further colors need to be displayed."
#endif
        QVector<QColor> colors = {
                m_matrix->getColor(0),
                m_matrix->getColor(1),
                m_matrix->getColor(2),
                m_matrix->getColor(3),
                m_matrix->getColor(4)
        };
        algo->setColors(colors);
        m_previewHandler->calculateColorDelta(m_matrix->getColor(0), m_matrix->getColor(1),
                m_matrix->algorithm());
    }
    updateExtraOptions();

    slotRestartTest();
}

void RGBMatrixEditor::slotFixtureGroupActivated(int index)
{
    QVariant var = m_fixtureGroupCombo->itemData(index);
    if (var.isValid() == true)
    {
        m_matrix->setFixtureGroup(var.toUInt());
        slotRestartTest();
    }
    else
    {
        m_matrix->setFixtureGroup(FixtureGroup::invalidId());
        m_previewTimer->stop();
        m_scene->clear();
    }
}

void RGBMatrixEditor::slotBlendModeChanged(int index)
{
    m_matrix->setBlendMode(Universe::BlendMode(index));

    if (index == Universe::MaskBlend)
    {
        m_mtxColor1Button->setEnabled(false);
    }
    else
    {
        m_mtxColor1Button->setEnabled(true);
    }
    updateExtraOptions();
    updateColors();
    slotRestartTest();
}

void RGBMatrixEditor::slotControlModeChanged(int index)
{
    RGBMatrix::ControlMode mode = RGBMatrix::ControlMode(index);
    m_matrix->setControlMode(mode);
    updateColors();
    slotRestartTest();
}

void RGBMatrixEditor::slotMtxColor1ButtonClicked()
{
    QColor col = QColorDialog::getColor(m_matrix->getColor(0));
    if (col.isValid() == true)
    {
        m_matrix->setColor(0, col);
        updateColors();
        slotRestartTest();
    }
}

void RGBMatrixEditor::slotMtxColor2ButtonClicked()
{
    QColor col = QColorDialog::getColor(m_matrix->getColor(1));
    if (col.isValid() == true)
    {
        m_matrix->setColor(1, col);
        updateColors();
        slotRestartTest();
    }
}

void RGBMatrixEditor::slotResetMtxColor2ButtonClicked()
{
    m_matrix->setColor(1, QColor());
    updateColors();
    slotRestartTest();
}

void RGBMatrixEditor::slotMtxColor3ButtonClicked()
{
    QColor col = QColorDialog::getColor(m_matrix->getColor(2));
    if (col.isValid() == true)
    {
        m_matrix->setColor(2, col);
        updateColors();
        slotRestartTest();
    }
}

void RGBMatrixEditor::slotResetMtxColor3ButtonClicked()
{
    m_matrix->setColor(2, QColor());
    updateColors();
    slotRestartTest();
}

void RGBMatrixEditor::slotMtxColor4ButtonClicked()
{
    QColor col = QColorDialog::getColor(m_matrix->getColor(3));
    if (col.isValid() == true)
    {
        m_matrix->setColor(3, col);
        updateColors();
        slotRestartTest();
    }
}

void RGBMatrixEditor::slotResetMtxColor4ButtonClicked()
{
    m_matrix->setColor(3, QColor());
    updateColors();
    slotRestartTest();
}

void RGBMatrixEditor::slotMtxColor5ButtonClicked()
{
    QColor col = QColorDialog::getColor(m_matrix->getColor(4));
    if (col.isValid() == true)
    {
        m_matrix->setColor(4, col);
        updateColors();
        slotRestartTest();
    }
}

void RGBMatrixEditor::slotResetMtxColor5ButtonClicked()
{
    m_matrix->setColor(4, QColor());
    updateColors();
    slotRestartTest();
}

void RGBMatrixEditor::slotTextEdited(const QString& text)
{
    if (m_matrix->algorithm() != NULL && m_matrix->algorithm()->type() == RGBAlgorithm::Text)
    {
        RGBText *algo = static_cast<RGBText*> (m_matrix->algorithm());
        Q_ASSERT(algo != NULL);
        {
            QMutexLocker algorithmLocker(&m_matrix->algorithmMutex());
            algo->setText(text);
        }
        slotRestartTest();
    }
}

void RGBMatrixEditor::slotFontButtonClicked()
{
    if (m_matrix->algorithm() != NULL && m_matrix->algorithm()->type() == RGBAlgorithm::Text)
    {
        RGBText *algo = static_cast<RGBText*> (m_matrix->algorithm());
        Q_ASSERT(algo != NULL);

        bool ok = false;
        QFont font = QFontDialog::getFont(&ok, algo->font(), this);
        if (ok == true)
        {
            {
                QMutexLocker algorithmLocker(&m_matrix->algorithmMutex());
                algo->setFont(font);
            }
            slotRestartTest();
        }
    }
}

void RGBMatrixEditor::slotAnimationActivated(int index)
{
    if (m_matrix->algorithm() != NULL && m_matrix->algorithm()->type() == RGBAlgorithm::Text)
    {
        RGBText *algo = static_cast<RGBText*> (m_matrix->algorithm());
        Q_ASSERT(algo != NULL);
        {
            QMutexLocker algorithmLocker(&m_matrix->algorithmMutex());
            QString text = m_animationCombo->itemText(index);
            algo->setAnimationStyle(RGBText::stringToAnimationStyle(text));
        }
        slotRestartTest();
    }
}

void RGBMatrixEditor::slotImageEdited()
{
    if (m_matrix->algorithm() != NULL && m_matrix->algorithm()->type() == RGBAlgorithm::Image)
    {
        RGBImage *algo = static_cast<RGBImage*> (m_matrix->algorithm());
        Q_ASSERT(algo != NULL);
        {
            QMutexLocker algorithmLocker(&m_matrix->algorithmMutex());
            algo->setFilename(m_imageEdit->text());
        }
        slotRestartTest();
    }
}

void RGBMatrixEditor::slotImageButtonClicked()
{
    if (m_matrix->algorithm() != NULL && m_matrix->algorithm()->type() == RGBAlgorithm::Image)
    {
        RGBImage *algo = static_cast<RGBImage*> (m_matrix->algorithm());
        Q_ASSERT(algo != NULL);

        QString path = algo->filename();
        path = QFileDialog::getOpenFileName(this,
                                            tr("Select image"),
                                            path,
                                            QString("%1 (*.png *.bmp *.jpg *.jpeg *.gif)").arg(tr("Images")));
        if (path.isEmpty() == false)
        {
            {
                QMutexLocker algorithmLocker(&m_matrix->algorithmMutex());
                algo->setFilename(path);
            }
            m_imageEdit->setText(path);
            slotRestartTest();
        }
    }
}

void RGBMatrixEditor::slotImageAnimationActivated(int index)
{
    if (m_matrix->algorithm() != NULL && m_matrix->algorithm()->type() == RGBAlgorithm::Image)
    {
        RGBImage *algo = static_cast<RGBImage*> (m_matrix->algorithm());
        Q_ASSERT(algo != NULL);
        {
            QMutexLocker algorithmLocker(&m_matrix->algorithmMutex());
            QString text = m_imageAnimationCombo->itemText(index);
            algo->setAnimationStyle(RGBImage::stringToAnimationStyle(text));
        }
        slotRestartTest();
    }
}

void RGBMatrixEditor::slotOffsetSpinChanged()
{
    if (m_matrix->algorithm() != NULL && m_matrix->algorithm()->type() == RGBAlgorithm::Text)
    {
        RGBText *algo = static_cast<RGBText*> (m_matrix->algorithm());
        Q_ASSERT(algo != NULL);
        {
            QMutexLocker algorithmLocker(&m_matrix->algorithmMutex());
            algo->setXOffset(m_xOffsetSpin->value());
            algo->setYOffset(m_yOffsetSpin->value());
        }
        slotRestartTest();
    }

    if (m_matrix->algorithm() != NULL && m_matrix->algorithm()->type() == RGBAlgorithm::Image)
    {
        RGBImage *algo = static_cast<RGBImage*> (m_matrix->algorithm());
        Q_ASSERT(algo != NULL);
        {
            QMutexLocker algorithmLocker(&m_matrix->algorithmMutex());
            algo->setXOffset(m_xOffsetSpin->value());
            algo->setYOffset(m_yOffsetSpin->value());
        }
        slotRestartTest();
    }
}

void RGBMatrixEditor::slotLoopClicked()
{
    m_matrix->setRunOrder(Function::Loop);
    m_previewHandler->calculateColorDelta(m_matrix->getColor(0), m_matrix->getColor(1),
            m_matrix->algorithm());
    slotRestartTest();
}

void RGBMatrixEditor::slotPingPongClicked()
{
    m_matrix->setRunOrder(Function::PingPong);
    m_previewHandler->calculateColorDelta(m_matrix->getColor(0), m_matrix->getColor(1),
            m_matrix->algorithm());
    slotRestartTest();
}

void RGBMatrixEditor::slotSingleShotClicked()
{
    m_matrix->setRunOrder(Function::SingleShot);
    m_previewHandler->calculateColorDelta(m_matrix->getColor(0), m_matrix->getColor(1),
            m_matrix->algorithm());
    slotRestartTest();
}

void RGBMatrixEditor::slotForwardClicked()
{
    m_matrix->setDirection(Function::Forward);
    m_previewHandler->calculateColorDelta(m_matrix->getColor(0), m_matrix->getColor(1),
            m_matrix->algorithm());
    slotRestartTest();
}

void RGBMatrixEditor::slotBackwardClicked()
{
    m_matrix->setDirection(Function::Backward);
    m_previewHandler->calculateColorDelta(m_matrix->getColor(0), m_matrix->getColor(1),
            m_matrix->algorithm());
    slotRestartTest();
}

void RGBMatrixEditor::slotDimmerControlClicked()
{
    m_matrix->setDimmerControl(m_dimmerControlCb->isChecked());
    if (m_dimmerControlCb->isChecked() == false)
        m_dimmerControlCb->setEnabled(false);
}

void RGBMatrixEditor::slotFadeInChanged(int ms)
{
    m_matrix->setFadeInSpeed(ms);
    uint duration = Function::speedAdd(ms, m_speedDials->duration());
    m_matrix->setDuration(duration);
}

void RGBMatrixEditor::slotFadeOutChanged(int ms)
{
    m_matrix->setFadeOutSpeed(ms);
}

void RGBMatrixEditor::slotHoldChanged(int ms)
{
    uint duration = Function::speedAdd(m_matrix->fadeInSpeed(), ms);
    m_matrix->setDuration(duration);
}

void RGBMatrixEditor::slotDurationTapped()
{
    m_matrix->tap();
}

void RGBMatrixEditor::slotTestClicked()
{
    if (m_testButton->isChecked() == true)
        m_matrix->start(m_doc->masterTimer(), functionParent());
    else
        m_matrix->stopAndWait();
}

void RGBMatrixEditor::slotRestartTest()
{
    m_previewTimer->stop();

    if (m_testButton->isChecked() == true)
    {
        // Toggle off, toggle on. Duh.
        m_testButton->click();
        m_testButton->click();
    }

    if (createPreviewItems() == true)
        m_previewTimer->start(MasterTimer::tick());

}

void RGBMatrixEditor::slotModeChanged(Doc::Mode mode)
{
    if (mode == Doc::Operate)
    {
        if (m_testButton->isChecked() == true)
            m_matrix->stopAndWait();
        m_testButton->setChecked(false);
        m_testButton->setEnabled(false);
    }
    else
    {
        m_testButton->setEnabled(true);
    }
}

void RGBMatrixEditor::slotFixtureGroupAdded()
{
    fillFixtureGroupCombo();
}

void RGBMatrixEditor::slotFixtureGroupRemoved()
{
    fillFixtureGroupCombo();
    slotFixtureGroupActivated(m_fixtureGroupCombo->currentIndex());
}

void RGBMatrixEditor::slotFixtureGroupChanged(quint32 id)
{
    if (id == m_matrix->fixtureGroup())
    {
        // Update the whole chain -> maybe the fixture layout has changed
        fillFixtureGroupCombo();
        slotFixtureGroupActivated(m_fixtureGroupCombo->currentIndex());
    }
    else
    {
        // Just change the name of the group, nothing else is interesting at this point
        int index = m_fixtureGroupCombo->findData(id);
        if (index != -1)
        {
            FixtureGroup* grp = m_doc->fixtureGroup(id);
            m_fixtureGroupCombo->setItemText(index, grp->name());
        }
    }
}

void RGBMatrixEditor::slotSaveToSequenceClicked()
{
    if (m_matrix == NULL || m_matrix->fixtureGroup() == FixtureGroup::invalidId())
        return;

    FixtureGroup* grp = m_doc->fixtureGroup(m_matrix->fixtureGroup());
    if (grp != NULL && m_matrix->algorithm() != NULL)
    {
        bool testRunning = false;

        if (m_testButton->isChecked() == true)
        {
            m_testButton->click();
            testRunning = true;
        }
        else
            m_previewTimer->stop();

        Scene *grpScene = new Scene(m_doc);
        grpScene->setName(grp->name());
        grpScene->setVisible(false);

        QList<GroupHead> headList = grp->headList();
        foreach (GroupHead head, headList)
        {
            Fixture *fxi = m_doc->fixture(head.fxi);
            if (fxi == NULL)
                continue;

            if (m_controlModeCombo->currentIndex() == RGBMatrix::ControlModeRgb)
            {

                    QVector <quint32> rgb = fxi->rgbChannels(head.head);

                    // in case of CMY, dump those channels
                    if (rgb.count() == 0)
                        rgb = fxi->cmyChannels(head.head);

                    if (rgb.count() == 3)
                    {
                        grpScene->setValue(head.fxi, rgb.at(0), 0);
                        grpScene->setValue(head.fxi, rgb.at(1), 0);
                        grpScene->setValue(head.fxi, rgb.at(2), 0);
                    }
            }
            else if (m_controlModeCombo->currentIndex() == RGBMatrix::ControlModeDimmer)
            {
                quint32 channel = fxi->masterIntensityChannel();

                if (channel == QLCChannel::invalid())
                    channel = fxi->channelNumber(QLCChannel::Intensity, QLCChannel::MSB, head.head);

                if (channel != QLCChannel::invalid())
                    grpScene->setValue(head.fxi, channel, 0);
            }
            else
            {
                quint32 channel = QLCChannel::invalid();
                if (m_controlModeCombo->currentIndex() == RGBMatrix::ControlModeWhite)
                    channel = fxi->channelNumber(QLCChannel::White, QLCChannel::MSB, head.head);
                else if (m_controlModeCombo->currentIndex() == RGBMatrix::ControlModeAmber)
                    channel = fxi->channelNumber(QLCChannel::Amber, QLCChannel::MSB, head.head);
                else if (m_controlModeCombo->currentIndex() == RGBMatrix::ControlModeUV)
                    channel = fxi->channelNumber(QLCChannel::UV, QLCChannel::MSB, head.head);
                else if (m_controlModeCombo->currentIndex() == RGBMatrix::ControlModeShutter)
                {
                    QLCFixtureHead fHead = fxi->head(head.head);
                    QVector <quint32> shutters = fHead.shutterChannels();
                    if (shutters.count())
                        channel = shutters.first();
                }

                if (channel != QLCChannel::invalid())
                    grpScene->setValue(head.fxi, channel, 0);
            }
        }
        m_doc->addFunction(grpScene);

        int totalSteps = m_matrix->stepsCount();
        int increment = 1;
        int currentStep = 0;
        m_previewHandler->setStepColor(m_matrix->getColor(0));

        if (m_matrix->direction() == Function::Backward)
        {
            currentStep = totalSteps - 1;
            increment = -1;
            if (m_matrix->getColor(1).isValid())
                m_previewHandler->setStepColor(m_matrix->getColor(1));
        }
        m_previewHandler->calculateColorDelta(m_matrix->getColor(0), m_matrix->getColor(1),
                m_matrix->algorithm());

        if (m_matrix->runOrder() == RGBMatrix::PingPong)
            totalSteps = (totalSteps * 2) - 1;

        Sequence *sequence = new Sequence(m_doc);
        sequence->setName(QString("%1 %2").arg(m_matrix->name()).arg(tr("Sequence")));
        sequence->setBoundSceneID(grpScene->id());
        sequence->setDurationMode(Chaser::PerStep);
        sequence->setDuration(m_matrix->duration());

        if (m_matrix->fadeInSpeed() != 0)
        {
            sequence->setFadeInMode(Chaser::PerStep);
            sequence->setFadeInSpeed(m_matrix->fadeInSpeed());
        }
        if (m_matrix->fadeOutSpeed() != 0)
        {
            sequence->setFadeOutMode(Chaser::PerStep);
            sequence->setFadeOutSpeed(m_matrix->fadeOutSpeed());
        }

        for (int i = 0; i < totalSteps; i++)
        {
            m_matrix->previewMap(currentStep, m_previewHandler);
            ChaserStep step;
            step.fid = grpScene->id();
            step.hold = m_matrix->duration() - m_matrix->fadeInSpeed();
            step.duration = m_matrix->duration();
            step.fadeIn = m_matrix->fadeInSpeed();
            step.fadeOut = m_matrix->fadeOutSpeed();

            for (int y = 0; y < m_previewHandler->m_map.size(); y++)
            {
                for (int x = 0; x < m_previewHandler->m_map[y].size(); x++)
                {
                    uint col = m_previewHandler->m_map[y][x];
                    GroupHead head = grp->head(QLCPoint(x, y));

                    Fixture *fxi = m_doc->fixture(head.fxi);
                    if (fxi == NULL)
                        continue;

                    if (m_controlModeCombo->currentIndex() == RGBMatrix::ControlModeRgb)
                    {
                        QVector <quint32> rgb = fxi->rgbChannels(head.head);
                        QVector <quint32> cmy = fxi->cmyChannels(head.head);

                        if (rgb.count() == 3)
                        {
                            step.values.append(SceneValue(head.fxi, rgb.at(0), qRed(col)));
                            step.values.append(SceneValue(head.fxi, rgb.at(1), qGreen(col)));
                            step.values.append(SceneValue(head.fxi, rgb.at(2), qBlue(col)));
                        }

                        if (cmy.count() == 3)
                        {
                            QColor cmyCol(col);

                            step.values.append(SceneValue(head.fxi, cmy.at(0), cmyCol.cyan()));
                            step.values.append(SceneValue(head.fxi, cmy.at(1), cmyCol.magenta()));
                            step.values.append(SceneValue(head.fxi, cmy.at(2), cmyCol.yellow()));
                        }
                    }
                    else if (m_controlModeCombo->currentIndex() == RGBMatrix::ControlModeDimmer)
                    {
                        quint32 channel = fxi->masterIntensityChannel();

                        if (channel == QLCChannel::invalid())
                            channel = fxi->channelNumber(QLCChannel::Intensity, QLCChannel::MSB, head.head);

                        if (channel != QLCChannel::invalid())
                            step.values.append(SceneValue(head.fxi, channel, RGBMatrix::rgbToGrey(col)));
                    }
                    else
                    {
                        quint32 channel = QLCChannel::invalid();
                        if (m_controlModeCombo->currentIndex() == RGBMatrix::ControlModeWhite)
                            channel = fxi->channelNumber(QLCChannel::White, QLCChannel::MSB, head.head);
                        else if (m_controlModeCombo->currentIndex() == RGBMatrix::ControlModeAmber)
                            channel = fxi->channelNumber(QLCChannel::Amber, QLCChannel::MSB, head.head);
                        else if (m_controlModeCombo->currentIndex() == RGBMatrix::ControlModeUV)
                            channel = fxi->channelNumber(QLCChannel::UV, QLCChannel::MSB, head.head);
                        else if (m_controlModeCombo->currentIndex() == RGBMatrix::ControlModeShutter)
                        {
                            QLCFixtureHead fHead = fxi->head(head.head);
                            QVector <quint32> shutters = fHead.shutterChannels();
                            if (shutters.count())
                                channel = shutters.first();
                        }

                        if (channel != QLCChannel::invalid())
                            step.values.append(SceneValue(head.fxi, channel, RGBMatrix::rgbToGrey(col)));
                    }
                }
            }
            // !! Important !! matrix's heads can be displaced randomly but in a sequence
            // we absolutely need ordered values. So do it now !
            std::sort(step.values.begin(), step.values.end());

            sequence->addStep(step);
            currentStep += increment;
            if (currentStep == totalSteps && m_matrix->runOrder() == RGBMatrix::PingPong)
            {
                currentStep = totalSteps - 2;
                increment = -1;
            }
            m_previewHandler->updateStepColor(currentStep, m_matrix->getColor(0), m_matrix->stepsCount());
        }

        m_doc->addFunction(sequence);

        if (testRunning == true)
            m_testButton->click();
        else if (createPreviewItems() == true)
            m_previewTimer->start(MasterTimer::tick());
    }
}

void RGBMatrixEditor::slotShapeToggle(bool)
{
    createPreviewItems();
}

void RGBMatrixEditor::slotPropertyComboChanged(int index)
{
    if (m_matrix->algorithm() == NULL ||
        m_matrix->algorithm()->type() == RGBAlgorithm::Script)
    {
        QComboBox *combo = qobject_cast<QComboBox *>(sender());
        QString pName = combo->property("pName").toString();
        QString pValue = combo->itemText(index);
        qDebug() << "Property combo changed to" << pValue;
        m_matrix->setProperty(pName, pValue);

        updateColorOptions();
        updateColors();
    }
}

void RGBMatrixEditor::slotPropertySpinChanged(int value)
{
    qDebug() << "Property spin changed to" << value;
    if (m_matrix->algorithm() == NULL ||
        m_matrix->algorithm()->type() == RGBAlgorithm::Script)
    {
        QSpinBox *spin = qobject_cast<QSpinBox *>(sender());
        QString pName = spin->property("pName").toString();
        m_matrix->setProperty(pName, QString::number(value));
    }
}

void RGBMatrixEditor::slotPropertyDoubleSpinChanged(double value)
{
    qDebug() << "Property float changed to" << value;
    if (m_matrix->algorithm() == NULL ||
        m_matrix->algorithm()->type() == RGBAlgorithm::Script)
    {
        QDoubleSpinBox *spin = qobject_cast<QDoubleSpinBox *>(sender());
        QString pName = spin->property("pName").toString();
        m_matrix->setProperty(pName, QString::number(value));
    }
}

void RGBMatrixEditor::slotPropertyEditChanged(QString text)
{
    qDebug() << "Property string changed to" << text;
    if (m_matrix->algorithm() == NULL ||
        m_matrix->algorithm()->type() == RGBAlgorithm::Script)
    {
        QLineEdit *edit = qobject_cast<QLineEdit *>(sender());
        QString pName = edit->property("pName").toString();
        m_matrix->setProperty(pName, text);
    }
}

FunctionParent RGBMatrixEditor::functionParent() const
{
    return FunctionParent::master();
}
