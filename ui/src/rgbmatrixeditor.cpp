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
#include <QSpinBox>
#include <QLabel>
#include <QTimer>
#include <QDebug>
#include <QMutex>

#include "fixtureselection.h"
#include "speeddialwidget.h"
#include "rgbmatrixeditor.h"
#include "rgbmatrix.h"
#include "rgbitem.h"
#include "rgbtext.h"
#include "rgbimage.h"
#include "apputil.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"

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
    , m_speedDials(NULL)
    , m_scene(new QGraphicsScene(this))
    , m_previewTimer(new QTimer(this))
    , m_previewIterator(0)
    , m_previewStep(0)
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

    fillPatternCombo();
    fillFixtureGroupCombo();
    fillAnimationCombo();
    fillImageAnimationCombo();

    QPixmap pm(100, 26);
    pm.fill(m_matrix->startColor());
    m_startColorButton->setIcon(QIcon(pm));

    if (m_matrix->endColor().isValid())
        pm.fill(m_matrix->endColor());
    else
        pm.fill(Qt::transparent);
    m_endColorButton->setIcon(QIcon(pm));

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
    connect(m_patternCombo, SIGNAL(activated(const QString&)),
            this, SLOT(slotPatternActivated(const QString&)));
    connect(m_fixtureGroupCombo, SIGNAL(activated(int)),
            this, SLOT(slotFixtureGroupActivated(int)));
    connect(m_startColorButton, SIGNAL(clicked()),
            this, SLOT(slotStartColorButtonClicked()));
    connect(m_endColorButton, SIGNAL(clicked()),
            this, SLOT(slotEndColorButtonClicked()));
    connect(m_resetEndColorButton, SIGNAL(clicked()),
            this, SLOT(slotResetEndColorButtonClicked()));
    connect(m_textEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotTextEdited(const QString&)));
    connect(m_fontButton, SIGNAL(clicked()),
            this, SLOT(slotFontButtonClicked()));
    connect(m_animationCombo, SIGNAL(activated(const QString&)),
            this, SLOT(slotAnimationActivated(const QString&)));
    connect(m_imageEdit, SIGNAL(editingFinished()),
            this, SLOT(slotImageEdited()));
    connect(m_imageButton, SIGNAL(clicked()),
            this, SLOT(slotImageButtonClicked()));
    connect(m_imageAnimationCombo, SIGNAL(activated(const QString&)),
            this, SLOT(slotImageAnimationActivated(const QString&)));
    connect(m_xOffsetSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotOffsetSpinChanged()));
    connect(m_yOffsetSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotOffsetSpinChanged()));

    connect(m_loop, SIGNAL(clicked()), this, SLOT(slotLoopClicked()));
    connect(m_pingPong, SIGNAL(clicked()), this, SLOT(slotPingPongClicked()));
    connect(m_singleShot, SIGNAL(clicked()), this, SLOT(slotSingleShotClicked()));
    connect(m_forward, SIGNAL(clicked()), this, SLOT(slotForwardClicked()));
    connect(m_backward, SIGNAL(clicked()), this, SLOT(slotBackwardClicked()));

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

        RGBImage* image = static_cast<RGBImage*> (m_matrix->algorithm());
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

        RGBText* text = static_cast<RGBText*> (m_matrix->algorithm());
        Q_ASSERT(text != NULL);
        m_textEdit->setText(text->text());

        int index = m_animationCombo->findText(RGBText::animationStyleToString(text->animationStyle()));
        if (index != -1)
            m_animationCombo->setCurrentIndex(index);

        m_xOffsetSpin->setValue(text->xOffset());
        m_yOffsetSpin->setValue(text->yOffset());
    }

    if (m_matrix->algorithm() != NULL)
    {
        int accColors = m_matrix->algorithm()->acceptColors();
        if (accColors == 0)
        {
            m_startColorButton->hide();
            m_endColorButton->hide();
            m_resetEndColorButton->hide();
        }
        else if (accColors == 1)
        {
            m_startColorButton->show();
            m_endColorButton->hide();
            m_resetEndColorButton->hide();
        }
        else
        {
            m_startColorButton->show();
            m_endColorButton->show();
            m_resetEndColorButton->show();
        }
    }
}

/**
 * Helper function. Deletes all child widgets of the given layout @a item.
 */
void RGBMatrixEditor::resetProperties(QLayoutItem *item)
{
    if (item->layout()) {
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

    foreach(RGBScriptProperty prop, properties)
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
                connect(propCombo, SIGNAL(currentIndexChanged(QString)),
                        this, SLOT(slotPropertyComboChanged(QString)));
                m_propertiesLayout->addWidget(propCombo, gridRowIdx, 1);
                if (m_matrix != NULL)
                {
                    QString pValue = m_matrix->property(prop.m_name);
                    if (!pValue.isEmpty())
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
                        propCombo->setCurrentText(pValue);
#else
                        propCombo->setCurrentIndex(propCombo->findText(pValue));
#endif
                    else
                    {
                        pValue = script->property(prop.m_name);
                        if (!pValue.isEmpty())
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
                            propCombo->setCurrentText(pValue);
#else
                            propCombo->setCurrentIndex(propCombo->findText(pValue));
#endif
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

    // No preview in operate mode, too coslty
    if (m_doc->mode() == Doc::Operate)
        return false;

    FixtureGroup* grp = m_doc->fixtureGroup(m_matrix->fixtureGroup());
    if (grp == NULL)
    {
        QGraphicsTextItem* text = new QGraphicsTextItem(tr("No fixture group to control"));
        text->setDefaultTextColor(Qt::white);
        m_scene->addItem(text);
        return false;
    }

    m_previewDirection = m_matrix->direction();

    if (m_previewDirection == Function::Forward)
    {
        m_matrix->setStepColor(m_matrix->startColor());
    }
    else
    {
        if (m_matrix->endColor().isValid())
            m_matrix->setStepColor(m_matrix->endColor());
        else
            m_matrix->setStepColor(m_matrix->startColor());
    }

    m_matrix->calculateColorDelta();

    if (m_previewDirection == Function::Forward)
    {
        m_previewStep = 0;
    }
    else
    {
        m_previewStep = m_matrix->stepsCount() - 1;
    }

    RGBMap map = m_matrix->previewMap(m_previewStep);

    if (map.isEmpty())
        return false;

    for (int x = 0; x < grp->size().width(); x++)
    {
        for (int y = 0; y < grp->size().height(); y++)
        {
            QLCPoint pt(x, y);

            if (grp->headHash().contains(pt) == true)
            {
                if (m_shapeButton->isChecked() == false)
                {
                    RGBCircleItem* item = new RGBCircleItem;
                    item->setRect(x * RECT_SIZE + RECT_PADDING + ITEM_PADDING,
                                  y * RECT_SIZE + RECT_PADDING + ITEM_PADDING,
                                  ITEM_SIZE - (2 * ITEM_PADDING),
                                  ITEM_SIZE - (2 * ITEM_PADDING));
                    item->setColor(map[y][x]);
                    item->draw(0);
                    m_scene->addItem(item);
                    m_previewHash[pt] = item;
                }
                else
                {
                    RGBRectItem* item = new RGBRectItem;
                    item->setRect(x * RECT_SIZE + RECT_PADDING + ITEM_PADDING,
                                  y * RECT_SIZE + RECT_PADDING + ITEM_PADDING,
                                  ITEM_SIZE - 1,
                                  ITEM_SIZE - 1);
                    item->setColor(map[y][x]);
                    item->draw(0);
                    m_scene->addItem(item);
                    m_previewHash[pt] = item;
                }
            }
        }
    }
    return true;
}

void RGBMatrixEditor::slotPreviewTimeout()
{
    RGBCircleItem* shape = NULL;

    if (m_matrix->duration() <= 0)
        return;

    RGBMap map;

    m_previewIterator += MasterTimer::tick();
    if (m_previewIterator >= m_matrix->duration())
    {
        int stepsCount = m_matrix->stepsCount();
        //qDebug() << "previewTimeout. Step:" << m_previewStep;
        if (m_matrix->runOrder() == RGBMatrix::PingPong)
        {
            if (m_previewDirection == Function::Forward && (m_previewStep + 1) == stepsCount)
                m_previewDirection = Function::Backward;
            else if (m_previewDirection == Function::Backward && (m_previewStep - 1) < 0)
                m_previewDirection = Function::Forward;
        }

        if (m_previewDirection == Function::Forward)
        {
            m_previewStep++;
            if (m_previewStep >= stepsCount)
            {
                m_previewStep = 0;
                m_matrix->setStepColor(m_matrix->startColor());
            }
            else
                m_matrix->updateStepColor(m_previewStep);
        }
        else
        {
            m_previewStep--;
            if (m_previewStep < 0)
            {
                m_previewStep = stepsCount - 1;
                if (m_matrix->endColor().isValid())
                    m_matrix->setStepColor(m_matrix->endColor());
                else
                    m_matrix->setStepColor(m_matrix->startColor());
            }
            else
                m_matrix->updateStepColor(m_previewStep);
        }
        map = m_matrix->previewMap(m_previewStep);
        m_previewIterator = 0;
    }

    for (int y = 0; y < map.size(); y++)
    {
        for (int x = 0; x < map[y].size(); x++)
        {
            QLCPoint pt(x, y);
            if (m_previewHash.contains(pt) == true)
            {
                shape = static_cast<RGBCircleItem*>(m_previewHash[pt]);
                if (shape->color() != QColor(map[y][x]).rgb())
                    shape->setColor(map[y][x]);

                if (shape->color() == QColor(Qt::black).rgb())
                    shape->draw(m_matrix->fadeOutSpeed());
                else
                    shape->draw(m_matrix->fadeInSpeed());
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

void RGBMatrixEditor::slotPatternActivated(const QString& text)
{
    RGBAlgorithm* algo = RGBAlgorithm::algorithm(m_doc, text);
    if (algo != NULL)
        algo->setColors(m_matrix->startColor(), m_matrix->endColor());
    m_matrix->setAlgorithm(algo);
    m_matrix->calculateColorDelta();
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

void RGBMatrixEditor::slotStartColorButtonClicked()
{
    QColor col = QColorDialog::getColor(m_matrix->startColor());
    if (col.isValid() == true)
    {
        m_matrix->setStartColor(col);
        m_matrix->calculateColorDelta();
        QPixmap pm(100, 26);
        pm.fill(col);
        m_startColorButton->setIcon(QIcon(pm));
        slotRestartTest();
    }
}

void RGBMatrixEditor::slotEndColorButtonClicked()
{
    QColor col = QColorDialog::getColor(m_matrix->endColor());
    if (col.isValid() == true)
    {
        m_matrix->setEndColor(col);
        m_matrix->calculateColorDelta();
        QPixmap pm(100, 26);
        pm.fill(col);
        m_endColorButton->setIcon(QIcon(pm));
        slotRestartTest();
    }
}

void RGBMatrixEditor::slotResetEndColorButtonClicked()
{
    m_matrix->setEndColor(QColor());
    m_matrix->calculateColorDelta();
    QPixmap pm(100, 26);
    pm.fill(Qt::transparent);
    m_endColorButton->setIcon(QIcon(pm));
    slotRestartTest();
}

void RGBMatrixEditor::slotTextEdited(const QString& text)
{
    if (m_matrix->algorithm() != NULL && m_matrix->algorithm()->type() == RGBAlgorithm::Text)
    {
        RGBText* algo = static_cast<RGBText*> (m_matrix->algorithm());
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
        RGBText* algo = static_cast<RGBText*> (m_matrix->algorithm());
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

void RGBMatrixEditor::slotAnimationActivated(const QString& text)
{
    if (m_matrix->algorithm() != NULL && m_matrix->algorithm()->type() == RGBAlgorithm::Text)
    {
        RGBText* algo = static_cast<RGBText*> (m_matrix->algorithm());
        Q_ASSERT(algo != NULL);
        {
            QMutexLocker algorithmLocker(&m_matrix->algorithmMutex());
            algo->setAnimationStyle(RGBText::stringToAnimationStyle(text));
        }
        slotRestartTest();
    }
}

void RGBMatrixEditor::slotImageEdited()
{
    if (m_matrix->algorithm() != NULL && m_matrix->algorithm()->type() == RGBAlgorithm::Image)
    {
        RGBImage* algo = static_cast<RGBImage*> (m_matrix->algorithm());
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
        RGBImage* algo = static_cast<RGBImage*> (m_matrix->algorithm());
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

void RGBMatrixEditor::slotImageAnimationActivated(const QString& text)
{
    if (m_matrix->algorithm() != NULL && m_matrix->algorithm()->type() == RGBAlgorithm::Image)
    {
        RGBImage* algo = static_cast<RGBImage*> (m_matrix->algorithm());
        Q_ASSERT(algo != NULL);
        {
            QMutexLocker algorithmLocker(&m_matrix->algorithmMutex());
            algo->setAnimationStyle(RGBImage::stringToAnimationStyle(text));
        }
        slotRestartTest();
    }
}

void RGBMatrixEditor::slotOffsetSpinChanged()
{
    if (m_matrix->algorithm() != NULL && m_matrix->algorithm()->type() == RGBAlgorithm::Text)
    {
        RGBText* algo = static_cast<RGBText*> (m_matrix->algorithm());
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
        RGBImage* algo = static_cast<RGBImage*> (m_matrix->algorithm());
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
    m_matrix->calculateColorDelta();
    slotRestartTest();
}

void RGBMatrixEditor::slotPingPongClicked()
{
    m_matrix->setRunOrder(Function::PingPong);
    m_matrix->calculateColorDelta();
    slotRestartTest();
}

void RGBMatrixEditor::slotSingleShotClicked()
{
    m_matrix->setRunOrder(Function::SingleShot);
    m_matrix->calculateColorDelta();
    slotRestartTest();
}

void RGBMatrixEditor::slotForwardClicked()
{
    m_matrix->setDirection(Function::Forward);
    m_matrix->calculateColorDelta();
    slotRestartTest();
}

void RGBMatrixEditor::slotBackwardClicked()
{
    m_matrix->setDirection(Function::Backward);
    m_matrix->calculateColorDelta();
    slotRestartTest();
}

void RGBMatrixEditor::slotFadeInChanged(int ms)
{
    m_matrix->setFadeInSpeed(ms);
}

void RGBMatrixEditor::slotFadeOutChanged(int ms)
{
    m_matrix->setFadeOutSpeed(ms);
}

void RGBMatrixEditor::slotHoldChanged(int ms)
{
    uint duration = 0;
    if (ms < 0)
        duration = ms;
    else
        duration = m_matrix->fadeInSpeed() + ms;
    m_matrix->setDuration(duration);
}

void RGBMatrixEditor::slotDurationTapped()
{
    m_matrix->tap();
}

void RGBMatrixEditor::slotTestClicked()
{
    if (m_testButton->isChecked() == true)
    {
        m_previewTimer->stop();
        m_matrix->start(m_doc->masterTimer());
    }
    else
    {
        m_matrix->stopAndWait();
        m_previewIterator = 0;
        if (createPreviewItems() == true)
            m_previewTimer->start(MasterTimer::tick());
    }
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
    else
    {
        if (createPreviewItems() == true)
            m_previewTimer->start(MasterTimer::tick());
    }
}

void RGBMatrixEditor::slotModeChanged(Doc::Mode mode)
{
    if (mode == Doc::Operate)
    {
        if (m_testButton->isChecked() == true)
            m_matrix->stopAndWait();
        m_testButton->setChecked(false);
        m_previewTimer->stop();
        m_testButton->setEnabled(false);
    }
    else
    {
        m_previewTimer->start(MasterTimer::tick());
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
        QList<GroupHead> headList = grp->headList();
        foreach (GroupHead head, headList)
        {
            Fixture *fxi = m_doc->fixture(head.fxi);
            if (fxi == NULL)
                continue;
            QVector <quint32> rgbCh = fxi->rgbChannels(head.head);
            if (rgbCh.count() == 3)
            {
                grpScene->setValue(head.fxi, rgbCh.at(0), 0);
                grpScene->setValue(head.fxi, rgbCh.at(1), 0);
                grpScene->setValue(head.fxi, rgbCh.at(2), 0);
            }

            quint32 master = fxi->masterIntensityChannel(head.head);
            if (master != QLCChannel::invalid())
                grpScene->setValue(head.fxi, master, 0);
        }
        m_doc->addFunction(grpScene);

        int totalSteps = m_matrix->stepsCount();
        int increment = 1;
        int currentStep = 0;
        m_matrix->setStepColor(m_matrix->startColor());

        if (m_matrix->direction() == Function::Backward)
        {
            currentStep = totalSteps - 1;
            increment = -1;
            if (m_matrix->endColor().isValid())
                m_matrix->setStepColor(m_matrix->endColor());
        }
        m_matrix->calculateColorDelta();

        if (m_matrix->runOrder() == RGBMatrix::PingPong)
            totalSteps = (totalSteps * 2) - 1;

        Chaser *chaser = new Chaser(m_doc);
        chaser->setName(m_matrix->name());
        chaser->enableSequenceMode(grpScene->id());
        chaser->setDurationMode(Chaser::PerStep);
        chaser->setDuration(m_matrix->duration());
        chaser->setStartTime(0);
        if (m_matrix->fadeInSpeed() != 0)
        {
            chaser->setFadeInMode(Chaser::PerStep);
            chaser->setFadeInSpeed(m_matrix->fadeInSpeed());
        }
        if (m_matrix->fadeOutSpeed() != 0)
        {
            chaser->setFadeOutMode(Chaser::PerStep);
            chaser->setFadeOutSpeed(m_matrix->fadeOutSpeed());
        }

        for (int i = 0; i < totalSteps; i++)
        {
            RGBMap map = m_matrix->previewMap(currentStep);
            ChaserStep step;
            step.fid = grpScene->id();
            step.hold = m_matrix->duration() - m_matrix->fadeInSpeed();
            step.duration = m_matrix->duration();
            step.fadeIn = m_matrix->fadeInSpeed();
            step.fadeOut = m_matrix->fadeOutSpeed();

            for (int y = 0; y < map.size(); y++)
            {
                for (int x = 0; x < map[y].size(); x++)
                {
                    QColor rgb = QColor(map[y][x]);
                    GroupHead head = grp->head(QLCPoint(x, y));

                    Fixture *fxi = m_doc->fixture(head.fxi);
                    if (fxi == NULL)
                        continue;
                    QVector <quint32> rgbCh = fxi->rgbChannels(head.head);
                    if (rgbCh.count() == 3)
                    {
                        step.values.append(SceneValue(head.fxi, rgbCh.at(0), rgb.red()));
                        step.values.append(SceneValue(head.fxi, rgbCh.at(1), rgb.green()));
                        step.values.append(SceneValue(head.fxi, rgbCh.at(2), rgb.blue()));
                    }

                    quint32 master = fxi->masterIntensityChannel(head.head);
                    if (master != QLCChannel::invalid())
                        step.values.append(SceneValue(head.fxi, master, 255));
                }
            }
            // !! Important !! matrix's heads can be displaced randomly but in a sequence
            // we absolutely need ordered values. So do it now !
            qSort(step.values.begin(), step.values.end());

            chaser->addStep(step);
            currentStep += increment;
            if (currentStep == totalSteps && m_matrix->runOrder() == RGBMatrix::PingPong)
            {
                currentStep = totalSteps - 2;
                increment = -1;
            }
            m_matrix->updateStepColor(currentStep);
        }

        m_doc->addFunction(chaser);

        if (testRunning == true)
            m_testButton->click();
        else if (createPreviewItems() == true)
            m_previewTimer->start(MasterTimer::tick());
    }
}

void RGBMatrixEditor::slotShapeToggle(bool )
{
    createPreviewItems();
}

void RGBMatrixEditor::slotPropertyComboChanged(QString value)
{
    qDebug() << "Property combo changed to" << value;
    if (m_matrix->algorithm() == NULL ||
        m_matrix->algorithm()->type() == RGBAlgorithm::Script)
    {
        RGBScript *script = static_cast<RGBScript*> (m_matrix->algorithm());
        QComboBox *combo = (QComboBox *)sender();
        QString pName = combo->property("pName").toString();
        script->setProperty(pName, value);
        m_matrix->setProperty(pName, value);
    }
}

void RGBMatrixEditor::slotPropertySpinChanged(int value)
{
    qDebug() << "Property spin changed to" << value;
    if (m_matrix->algorithm() == NULL ||
        m_matrix->algorithm()->type() == RGBAlgorithm::Script)
    {
        RGBScript *script = static_cast<RGBScript*> (m_matrix->algorithm());
        QSpinBox *spin = (QSpinBox *)sender();
        QString pName = spin->property("pName").toString();
        script->setProperty(pName, QString::number(value));
        m_matrix->setProperty(pName, QString::number(value));
    }
}
