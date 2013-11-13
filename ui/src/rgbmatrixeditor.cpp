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
#include <QFontDialog>
#include <QGradient>
#include <QSettings>
#include <QTimer>
#include <QDebug>

#include "fixtureselection.h"
#include "speeddialwidget.h"
#include "rgbmatrixeditor.h"
#include "rgbmatrix.h"
#include "rgbitem.h"
#include "rgbtext.h"
#include "apputil.h"
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
            delete m_speedDials;
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

    QPixmap pm(100, 26);
    pm.fill(m_matrix->startColor());
    m_startColorButton->setIcon(QIcon(pm));

    if (m_matrix->endColor().isValid())
        pm.fill(m_matrix->endColor());
    else
        pm.fill(Qt::transparent);
    m_endColorButton->setIcon(QIcon(pm));

    m_preferIntensityChannels->setChecked(m_matrix->preferIntensityChannels());

    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameEdited(const QString&)));
    connect(m_speedDialButton, SIGNAL(toggled(bool)),
            this, SLOT(slotSpeedDialToggle(bool)));
    connect(m_patternCombo, SIGNAL(activated(const QString&)),
            this, SLOT(slotPatternActivated(const QString&)));
    connect(m_fixtureGroupCombo, SIGNAL(activated(int)),
            this, SLOT(slotFixtureGroupActivated(int)));
    connect(m_startColorButton, SIGNAL(clicked()),
            this, SLOT(slotStartColorButtonClicked()));
    connect(m_endColorButton, SIGNAL(clicked()),
            this, SLOT(slotEndColorButtonClicked()));
    connect(m_textEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotTextEdited(const QString&)));
    connect(m_fontButton, SIGNAL(clicked()),
            this, SLOT(slotFontButtonClicked()));
    connect(m_animationCombo, SIGNAL(activated(const QString&)),
            this, SLOT(slotAnimationActivated(const QString&)));
    connect(m_xOffsetSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotOffsetSpinChanged()));
    connect(m_yOffsetSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotOffsetSpinChanged()));
    connect(m_preferIntensityChannels, SIGNAL(stateChanged(int)),
            this, SLOT(slotPreferIntensityChannelsChanged(int)));

    connect(m_loop, SIGNAL(clicked()), this, SLOT(slotLoopClicked()));
    connect(m_pingPong, SIGNAL(clicked()), this, SLOT(slotPingPongClicked()));
    connect(m_singleShot, SIGNAL(clicked()), this, SLOT(slotSingleShotClicked()));
    connect(m_forward, SIGNAL(clicked()), this, SLOT(slotForwardClicked()));
    connect(m_backward, SIGNAL(clicked()), this, SLOT(slotBackwardClicked()));

    // Test slots
    connect(m_testButton, SIGNAL(clicked(bool)),
            this, SLOT(slotTestClicked()));

    createPreviewItems();
    m_preview->setScene(m_scene);
    m_previewTimer->start(MasterTimer::tick());

    updateExtraOptions();
    updateSpeedDials();
}

void RGBMatrixEditor::updateSpeedDials()
{
    if (m_speedDialButton->isChecked() == false)
        return;

    if (m_speedDials != NULL)
        return;

    m_speedDials = new SpeedDialWidget(this);
    m_speedDials->setWindowTitle(m_matrix->name());
    m_speedDials->show();
    m_speedDials->setFadeInSpeed(m_matrix->fadeInSpeed());
    m_speedDials->setFadeOutSpeed(m_matrix->fadeOutSpeed());
    if ((int)m_matrix->duration() < 0)
        m_speedDials->setDuration(m_matrix->duration());
    else
        m_speedDials->setDuration(m_matrix->duration() - m_matrix->fadeInSpeed() - m_matrix->fadeOutSpeed());
    connect(m_speedDials, SIGNAL(fadeInChanged(int)), this, SLOT(slotFadeInChanged(int)));
    connect(m_speedDials, SIGNAL(fadeOutChanged(int)), this, SLOT(slotFadeOutChanged(int)));
    connect(m_speedDials, SIGNAL(holdChanged(int)), this, SLOT(slotHoldChanged(int)));
    connect(m_speedDials, SIGNAL(durationTapped()), this, SLOT(slotDurationTapped()));
}

void RGBMatrixEditor::fillPatternCombo()
{
    m_patternCombo->addItems(RGBAlgorithm::algorithms());
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

void RGBMatrixEditor::updateExtraOptions()
{
    if (m_matrix->algorithm() == NULL || m_matrix->algorithm()->type() != RGBAlgorithm::Text)
    {
        m_textGroup->hide();
        m_offsetGroup->hide();
    }
    else
    {
        m_textGroup->show();
        m_offsetGroup->show();

        RGBText* text = static_cast<RGBText*> (m_matrix->algorithm());
        Q_ASSERT(text != NULL);
        m_textEdit->setText(text->text());

        int index = m_animationCombo->findText(RGBText::animationStyleToString(text->animationStyle()));
        if (index != -1)
            m_animationCombo->setCurrentIndex(index);

        m_xOffsetSpin->setValue(text->xOffset());
        m_yOffsetSpin->setValue(text->yOffset());
    }
}

void RGBMatrixEditor::createPreviewItems()
{
    m_previewHash.clear();
    m_scene->clear();

    QPalette pal(this->palette());

    FixtureGroup* grp = m_doc->fixtureGroup(m_matrix->fixtureGroup());
    if (grp == NULL)
    {
        QGraphicsTextItem* text = new QGraphicsTextItem(tr("No fixture group to control"));
        m_scene->addItem(text);
        return;
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
    m_previewMaps = m_matrix->previewMaps();

    if ((m_previewDirection == Function::Forward) || m_previewMaps.isEmpty())
    {
        m_previewStep = 0;
    }
    else
    {
        m_previewStep = m_previewMaps.size() - 1;
    }

    RGBMap map;
    if (m_previewStep < m_previewMaps.size())
        map = m_previewMaps[m_previewStep];

    for (int x = 0; x < grp->size().width(); x++)
    {
        for (int y = 0; y < grp->size().height(); y++)
        {
            QLCPoint pt(x, y);

            if (grp->headHash().contains(pt) == true)
            {
                RGBItem* item = new RGBItem;
                item->setRect(x * RECT_SIZE + RECT_PADDING + ITEM_PADDING,
                              y * RECT_SIZE + RECT_PADDING + ITEM_PADDING,
                              ITEM_SIZE - (2 * ITEM_PADDING),
                              ITEM_SIZE - (2 * ITEM_PADDING));
                item->setColor(map[y][x]);
                item->draw(0);
                m_scene->addItem(item);
                m_previewHash[pt] = item;
            }
        }
    }
}

void RGBMatrixEditor::slotPreviewTimeout()
{
    RGBItem* shape = NULL;

    if (m_matrix->duration() <= 0)
        return;

    m_previewIterator += MasterTimer::tick();
    if (m_previewIterator >= m_matrix->duration())
    {
        qDebug() << "previewTimeout. Step:" << m_previewStep;
        if (m_matrix->runOrder() == RGBMatrix::PingPong)
        {
            if (m_previewDirection == Function::Forward && (m_previewStep + 1) == m_previewMaps.size())
                m_previewDirection = Function::Backward;
            else if (m_previewDirection == Function::Backward && (m_previewStep - 1) < 0)
                m_previewDirection = Function::Forward;
        }

        if (m_previewDirection == Function::Forward)
        {
            m_previewStep++;
            if (m_previewStep >= m_previewMaps.size())
            {
                m_previewStep = 0;
                m_matrix->setStepColor(m_matrix->startColor());
            }
            else
                m_matrix->updateStepColor(m_previewDirection);
        }
        else
        {
            m_previewStep--;
            if (m_previewStep < 0)
            {
                m_previewStep = m_previewMaps.size() - 1;
                if (m_matrix->endColor().isValid())
                    m_matrix->setStepColor(m_matrix->endColor());
                else
                    m_matrix->setStepColor(m_matrix->startColor());
            }
            else
                m_matrix->updateStepColor(m_previewDirection);
        }
        m_previewMaps = m_matrix->previewMaps();
        m_previewIterator = 0;
    }

    RGBMap map;
    if (m_previewStep >= 0 && m_previewStep < m_previewMaps.size())
        map = m_previewMaps[m_previewStep];

    for (int y = 0; y < map.size(); y++)
    {
        for (int x = 0; x < map[y].size(); x++)
        {
            QLCPoint pt(x, y);
            if (m_previewHash.contains(pt) == true)
            {
                shape = static_cast<RGBItem*>(m_previewHash[pt]);
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
            delete m_speedDials;
        m_speedDials = NULL;
    }
}

void RGBMatrixEditor::slotPatternActivated(const QString& text)
{
    RGBAlgorithm* algo = RGBAlgorithm::algorithm(text);
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

void RGBMatrixEditor::slotTextEdited(const QString& text)
{
    if (m_matrix->algorithm() != NULL && m_matrix->algorithm()->type() == RGBAlgorithm::Text)
    {
        RGBText* algo = static_cast<RGBText*> (m_matrix->algorithm());
        Q_ASSERT(algo != NULL);
        algo->setText(text);
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
            algo->setFont(font);
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
        algo->setAnimationStyle(RGBText::stringToAnimationStyle(text));
        slotRestartTest();
    }
}

void RGBMatrixEditor::slotOffsetSpinChanged()
{
    if (m_matrix->algorithm() != NULL && m_matrix->algorithm()->type() == RGBAlgorithm::Text)
    {
        RGBText* algo = static_cast<RGBText*> (m_matrix->algorithm());
        Q_ASSERT(algo != NULL);
        algo->setXOffset(m_xOffsetSpin->value());
        algo->setYOffset(m_yOffsetSpin->value());
        slotRestartTest();
    }
}

void RGBMatrixEditor::slotPreferIntensityChannelsChanged(int state)
{
    m_matrix->setPreferIntensityChannels(state == Qt::Checked);
    slotRestartTest();
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
        duration = m_matrix->fadeInSpeed() + ms + m_matrix->fadeOutSpeed();
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
        createPreviewItems();
        m_previewTimer->start(MasterTimer::tick());
    }
}

void RGBMatrixEditor::slotRestartTest()
{
    m_previewTimer->stop();
    m_previewMaps = m_matrix->previewMaps();

    if (m_testButton->isChecked() == true)
    {
        // Toggle off, toggle on. Duh.
        m_testButton->click();
        m_testButton->click();
    }
    else
        createPreviewItems();
    m_previewTimer->start(MasterTimer::tick());
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
