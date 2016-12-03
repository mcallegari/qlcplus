/*
  Q Light Controller Plus
  rgbmatrixeditor.cpp

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

#include <QTimer>
#include <QDebug>

#include "rgbmatrixeditor.h"

#include "rgbmatrix.h"
#include "rgbimage.h"
#include "rgbtext.h"
#include "doc.h"

RGBMatrixEditor::RGBMatrixEditor(QQuickView *view, Doc *doc, QObject *parent)
    : FunctionEditor(view, doc, parent)
    , m_matrix(NULL)
    , m_group(NULL)
    , m_previewTimer(new QTimer(this))
    , m_previewStepHandler(new RGBMatrixStep())
{
    m_view->rootContext()->setContextProperty("rgbMatrixEditor", this);

    m_gotBeat = false;
    connect(m_previewTimer, SIGNAL(timeout()), this, SLOT(slotPreviewTimeout()));
    connect(m_doc->masterTimer(), SIGNAL(beat()), this, SLOT(slotBeatReceived()));
}

RGBMatrixEditor::~RGBMatrixEditor()
{
    m_previewTimer->stop();
    m_view->rootContext()->setContextProperty("rgbMatrixEditor", NULL);
    delete m_previewStepHandler;
}

void RGBMatrixEditor::setFunctionID(quint32 id)
{
    if (id == Function::invalidId())
    {
        m_matrix = NULL;
        m_group = NULL;
        return;
    }

    m_matrix = qobject_cast<RGBMatrix *>(m_doc->function(id));
    if (m_matrix == NULL)
        return;

    m_group = m_doc->fixtureGroup(m_matrix->fixtureGroup());

    if (m_group == NULL)
    {
        if (m_doc->fixtureGroups().count())
        {
            m_group = m_doc->fixtureGroups().first();
            m_matrix->setFixtureGroup(m_group->id());
        }
    }

    initPreviewData();
    emit previewSizeChanged();

    FunctionEditor::setFunctionID(id);
}

int RGBMatrixEditor::fixtureGroup() const
{
    if (m_matrix == NULL || m_group == NULL)
        return -1;

    return m_group->id();
}

void RGBMatrixEditor::setFixtureGroup(int fixtureGroup)
{
    if (m_matrix == NULL || m_matrix->fixtureGroup() == (quint32)fixtureGroup)
        return;

    m_matrix->setFixtureGroup((quint32)fixtureGroup);
    m_group = m_doc->fixtureGroup(fixtureGroup);
    emit fixtureGroupChanged(fixtureGroup);
    initPreviewData();
    emit previewSizeChanged();
}

/************************************************************************
 * Algorithm
 ************************************************************************/

QStringList RGBMatrixEditor::algorithms() const
{
    return RGBAlgorithm::algorithms(m_doc);
}

int RGBMatrixEditor::algorithmIndex() const
{
    if (m_matrix == NULL || m_matrix->algorithm() == NULL)
        return -1;

    QStringList algoList = algorithms();
    return algoList.indexOf(m_matrix->algorithm()->name());
}

void RGBMatrixEditor::setAlgorithmIndex(int algoIndex)
{
    qDebug() << "Set algorithm:" << algoIndex;
    QStringList algoList = algorithms();
    if(algoIndex < 0 || algoIndex >= algorithms().count())
        return;

    RGBAlgorithm* algo = RGBAlgorithm::algorithm(m_doc, algoList.at(algoIndex));
    if (algo != NULL)
    {
        /** if we're setting the same algorithm, then there's nothing to do */
        if (m_matrix->algorithm() != NULL && m_matrix->algorithm()->name() == algo->name())
            return;
        algo->setColors(m_matrix->startColor(), m_matrix->endColor());
    }
    m_matrix->setAlgorithm(algo);

    initPreviewData();

    emit algorithmIndexChanged();
    emit algoColorsChanged();
}

int RGBMatrixEditor::algoColors()
{
    if (m_matrix == NULL || m_matrix->algorithm() == NULL)
        return 0;

    return m_matrix->algorithm()->acceptColors();
}

QColor RGBMatrixEditor::startColor() const
{
    if (m_matrix == NULL)
        return Qt::red;

    return m_matrix->startColor();
}

void RGBMatrixEditor::setStartColor(QColor algoStartColor)
{
    if (m_matrix == NULL || m_matrix->startColor() == algoStartColor)
        return;

    m_matrix->setStartColor(algoStartColor);
    m_previewStepHandler->calculateColorDelta(m_matrix->startColor(), m_matrix->endColor());

    emit startColorChanged(algoStartColor);
}

QColor RGBMatrixEditor::endColor() const
{
    if (m_matrix == NULL)
        return QColor();

    return m_matrix->endColor();
}

void RGBMatrixEditor::setEndColor(QColor algoEndColor)
{
    if (m_matrix == NULL || m_matrix->endColor() == algoEndColor)
        return;

    m_matrix->setEndColor(algoEndColor);
    m_previewStepHandler->calculateColorDelta(m_matrix->startColor(), m_matrix->endColor());

    emit endColorChanged(algoEndColor);
    if (algoEndColor.isValid())
        emit hasEndColorChanged(true);
}

bool RGBMatrixEditor::hasEndColor() const
{
    if (m_matrix == NULL || m_matrix->endColor().isValid() == false)
        return false;

    return true;
}

void RGBMatrixEditor::setHasEndColor(bool hasEndCol)
{
    if (m_matrix && hasEndCol == false)
    {
        m_matrix->setEndColor(QColor());
        m_previewStepHandler->calculateColorDelta(m_matrix->startColor(), m_matrix->endColor());
    }
    emit hasEndColorChanged(hasEndCol);
}

QString RGBMatrixEditor::algoText() const
{
    if (m_matrix != NULL && m_matrix->algorithm() != NULL &&
        m_matrix->algorithm()->type() == RGBAlgorithm::Text)
    {
        RGBText* algo = static_cast<RGBText*> (m_matrix->algorithm());
        return algo->text();
    }

    return QString(" Q LIGHT CONTROLLER + ");
}

void RGBMatrixEditor::setAlgoText(QString text)
{
    if (m_matrix != NULL && m_matrix->algorithm() != NULL &&
        m_matrix->algorithm()->type() == RGBAlgorithm::Text)
    {
        RGBText* algo = static_cast<RGBText*> (m_matrix->algorithm());
        if (algo->text() == text)
            return;

        QMutexLocker algorithmLocker(&m_matrix->algorithmMutex());
        algo->setText(text);
        emit algoTextChanged(text);
    }
}

QFont RGBMatrixEditor::algoTextFont() const
{
    if (m_matrix != NULL && m_matrix->algorithm() != NULL &&
        m_matrix->algorithm()->type() == RGBAlgorithm::Text)
    {
        RGBText* algo = static_cast<RGBText*> (m_matrix->algorithm());
        return algo->font();
    }
    return QFont();
}

void RGBMatrixEditor::setAlgoTextFont(QFont algoTextFont)
{
    if (m_matrix != NULL && m_matrix->algorithm() != NULL &&
        m_matrix->algorithm()->type() == RGBAlgorithm::Text)
    {
        RGBText* algo = static_cast<RGBText*> (m_matrix->algorithm());
        if (algo->font() == algoTextFont)
            return;
        QMutexLocker algorithmLocker(&m_matrix->algorithmMutex());
        algo->setFont(algoTextFont);
        emit algoTextFontChanged(algoTextFont);
    }
}

QString RGBMatrixEditor::algoImagePath() const
{
    if (m_matrix != NULL && m_matrix->algorithm() != NULL &&
        m_matrix->algorithm()->type() == RGBAlgorithm::Image)
    {
        RGBImage* algo = static_cast<RGBImage*> (m_matrix->algorithm());
        return algo->filename();
    }

    return QString();
}

void RGBMatrixEditor::setAlgoImagePath(QString path)
{
    if (m_matrix != NULL && m_matrix->algorithm() != NULL &&
        m_matrix->algorithm()->type() == RGBAlgorithm::Image)
    {
        RGBImage* algo = static_cast<RGBImage*> (m_matrix->algorithm());

        if (path.startsWith("file:"))
            path = QUrl(path).toLocalFile();

        if (algo->filename() == path)
            return;

        QMutexLocker algorithmLocker(&m_matrix->algorithmMutex());
        algo->setFilename(path);
        emit algoImagePathChanged(path);
    }
}

QSize RGBMatrixEditor::algoOffset() const
{
    if (m_matrix != NULL && m_matrix->algorithm() != NULL)
    {
        if (m_matrix->algorithm()->type() == RGBAlgorithm::Image)
        {
            RGBImage* algo = static_cast<RGBImage*> (m_matrix->algorithm());
            return QSize(algo->xOffset(), algo->yOffset());
        }
        else if (m_matrix->algorithm()->type() == RGBAlgorithm::Text)
        {
            RGBText* algo = static_cast<RGBText*> (m_matrix->algorithm());
            return QSize(algo->xOffset(), algo->yOffset());
        }
    }
    return QSize(0, 0);
}

void RGBMatrixEditor::setAlgoOffset(QSize algoOffset)
{
    if (m_matrix != NULL && m_matrix->algorithm() != NULL)
    {
        if (m_matrix->algorithm()->type() == RGBAlgorithm::Image)
        {
            RGBImage* algo = static_cast<RGBImage*> (m_matrix->algorithm());
            QMutexLocker algorithmLocker(&m_matrix->algorithmMutex());
            algo->setXOffset(algoOffset.width());
            algo->setYOffset(algoOffset.height());
            emit algoOffsetChanged(algoOffset);
        }
        else if (m_matrix->algorithm()->type() == RGBAlgorithm::Text)
        {
            RGBText* algo = static_cast<RGBText*> (m_matrix->algorithm());
            QMutexLocker algorithmLocker(&m_matrix->algorithmMutex());
            algo->setXOffset(algoOffset.width());
            algo->setYOffset(algoOffset.height());
            emit algoOffsetChanged(algoOffset);
        }
    }
}

int RGBMatrixEditor::animationStyle() const
{
    if (m_matrix != NULL && m_matrix->algorithm() != NULL)
    {
        if (m_matrix->algorithm()->type() == RGBAlgorithm::Image)
        {
            RGBImage* algo = static_cast<RGBImage*> (m_matrix->algorithm());
            return (int)algo->animationStyle();
        }
        else if (m_matrix->algorithm()->type() == RGBAlgorithm::Text)
        {
            RGBText* algo = static_cast<RGBText*> (m_matrix->algorithm());
            return (int)algo->animationStyle();
        }
    }
    return 0;
}

void RGBMatrixEditor::setAnimationStyle(int style)
{
    if (m_matrix != NULL && m_matrix->algorithm() != NULL)
    {
        if (m_matrix->algorithm()->type() == RGBAlgorithm::Image)
        {
            RGBImage* algo = static_cast<RGBImage*> (m_matrix->algorithm());
            QMutexLocker algorithmLocker(&m_matrix->algorithmMutex());
            if ((int)algo->animationStyle() == style)
                return;

            algo->setAnimationStyle(RGBImage::AnimationStyle(style));
            emit animationStyleChanged(style);
        }
        else if (m_matrix->algorithm()->type() == RGBAlgorithm::Text)
        {
            RGBText* algo = static_cast<RGBText*> (m_matrix->algorithm());
            QMutexLocker algorithmLocker(&m_matrix->algorithmMutex());
            if ((int)algo->animationStyle() == style)
                return;

            algo->setAnimationStyle(RGBText::AnimationStyle(style));
            emit animationStyleChanged(style);
        }
    }
}

void RGBMatrixEditor::createScriptObjects(QQuickItem *parent)
{
    if (m_matrix == NULL || m_matrix->algorithm() == NULL ||
            m_matrix->algorithm()->type() != RGBAlgorithm::Script)
        return;

    RGBScript* script = static_cast<RGBScript*> (m_matrix->algorithm());
    QList<RGBScriptProperty> properties = script->properties();

    foreach(RGBScriptProperty prop, properties)
    {
        // always create a label first
        QMetaObject::invokeMethod(parent, "addLabel",
                Q_ARG(QVariant, prop.m_displayName));

        switch(prop.m_type)
        {
            case RGBScriptProperty::List:
            {
                QVariantList valList;
                int idx = 0;
                int currIdx = 0;
                QString pValue = m_matrix->property(prop.m_name);
                if (pValue.isEmpty())
                    pValue = script->property(prop.m_name);

                foreach(QString val, prop.m_listValues)
                {
                    if (val == pValue)
                        currIdx = idx;

                    QVariantMap valMap;
                    valMap.insert("mIcon", "");
                    valMap.insert("mLabel", val);
                    valMap.insert("mValue", idx++);
                    valList.append(valMap);
                }

                QMetaObject::invokeMethod(parent, "addComboBox",
                        Q_ARG(QVariant, prop.m_name),
                        Q_ARG(QVariant, QVariant::fromValue(valList)),
                        Q_ARG(QVariant, currIdx));
            }
            break;
            case RGBScriptProperty::Range:
            {
                QString pValue = m_matrix->property(prop.m_name);
                if (pValue.isEmpty())
                    pValue = script->property(prop.m_name);

                QMetaObject::invokeMethod(parent, "addSpinBox",
                        Q_ARG(QVariant, prop.m_name),
                        Q_ARG(QVariant, prop.m_rangeMinValue),
                        Q_ARG(QVariant, prop.m_rangeMaxValue),
                        Q_ARG(QVariant, pValue.toInt()));
            }
            break;
            default:
                qWarning() << "Type" << prop.m_type << "not handled yet";
            break;
        }
    }
}

void RGBMatrixEditor::setScriptStringProperty(QString paramName, QString value)
{
    if (m_matrix == NULL || m_matrix->algorithm() == NULL ||
        m_matrix->algorithm()->type() != RGBAlgorithm::Script)
            return;

    qDebug() << "[setScriptStringProperty] param:" << paramName << ", value:" << value;

    m_matrix->setProperty(paramName, value);
}

void RGBMatrixEditor::setScriptIntProperty(QString paramName, int value)
{
    if (m_matrix == NULL || m_matrix->algorithm() == NULL ||
        m_matrix->algorithm()->type() != RGBAlgorithm::Script)
            return;

    qDebug() << "[setScriptIntProperty] param:" << paramName << ", value:" << value;

    m_matrix->setProperty(paramName, QString::number(value));
}

/************************************************************************
 * Speed
 ************************************************************************/

int RGBMatrixEditor::fadeInSpeed() const
{
    if (m_matrix == NULL)
        return Function::defaultSpeed();

    return m_matrix->fadeInSpeed();
}

void RGBMatrixEditor::setFadeInSpeed(int fadeInSpeed)
{
    if (m_matrix == NULL)
        return;

    if (m_matrix->fadeInSpeed() == (uint)fadeInSpeed)
        return;

    m_matrix->setFadeInSpeed(fadeInSpeed);
    emit fadeInSpeedChanged(fadeInSpeed);
}

int RGBMatrixEditor::holdSpeed() const
{
    if (m_matrix == NULL)
        return Function::defaultSpeed();

    return m_matrix->duration();
}

void RGBMatrixEditor::setHoldSpeed(int holdSpeed)
{
    if (m_matrix == NULL)
        return;

    if (m_matrix->duration() - m_matrix->fadeInSpeed() == (uint)holdSpeed)
        return;

    uint duration = Function::speedAdd(m_matrix->fadeInSpeed(), holdSpeed);
    m_matrix->setDuration(duration);

    emit holdSpeedChanged(holdSpeed);
}

int RGBMatrixEditor::fadeOutSpeed() const
{
    if (m_matrix == NULL)
        return Function::defaultSpeed();

    return m_matrix->fadeOutSpeed();
}

void RGBMatrixEditor::setFadeOutSpeed(int fadeOutSpeed)
{
    if (m_matrix == NULL)
        return;

    if (m_matrix->fadeOutSpeed() == (uint)fadeOutSpeed)
        return;

    m_matrix->setFadeOutSpeed(fadeOutSpeed);
    emit fadeOutSpeedChanged(fadeOutSpeed);
}

/************************************************************************
 * Run order and direction
 ************************************************************************/

int RGBMatrixEditor::runOrder() const
{
    if (m_matrix == NULL)
        return Function::Loop;

    return m_matrix->runOrder();
}

void RGBMatrixEditor::setRunOrder(int runOrder)
{
    if (m_matrix == NULL || m_matrix->runOrder() == Function::RunOrder(runOrder))
        return;

    m_matrix->setRunOrder(Function::RunOrder(runOrder));
    emit runOrderChanged(runOrder);
}

int RGBMatrixEditor::direction() const
{
    if (m_matrix == NULL)
        return Function::Forward;

    return m_matrix->direction();
}

void RGBMatrixEditor::setDirection(int direction)
{
    if (m_matrix == NULL || m_matrix->direction() == Function::Direction(direction))
        return;

    m_matrix->setDirection(Function::Direction(direction));
    emit directionChanged(direction);
}

/************************************************************************
 * Preview
 ************************************************************************/

QSize RGBMatrixEditor::previewSize() const
{
    if (m_matrix == NULL || m_group == NULL)
        return QSize(0, 0);

    return m_group->size();
}

QVariantList RGBMatrixEditor::previewData() const
{
    return m_previewData;
}

void RGBMatrixEditor::slotPreviewTimeout()
{
    if (m_matrix == NULL || m_group == NULL || m_matrix->duration() <= 0)
        return;

    RGBMap map;

    if (m_matrix->tempoType() == Function::Time)
    {
        m_previewElapsed += MasterTimer::tick();
    }
    else if (m_matrix->tempoType() == Function::Beats && m_gotBeat)
    {
        m_gotBeat = false;
        m_previewElapsed += 1000;
    }

    if (m_previewElapsed >= m_matrix->duration())
    {
        m_previewStepHandler->checkNextStep(m_matrix->runOrder(), m_matrix->startColor(),
                                            m_matrix->endColor(), m_matrix->stepsCount());

        map = m_matrix->previewMap(m_previewStepHandler->currentStepIndex(), m_previewStepHandler);

        //qDebug() << "Step changing. Index:" << m_previewStepHandler->currentStepIndex() << ", map size:" << map.size();

        m_previewElapsed = 0;
/*
        for (int y = 0; y < map.size(); y++)
        {
            for (int x = 0; x < map[y].size(); x++)
            {
                QLCPoint pt(x, y);
                if (m_group->head(pt).isValid())
                {
                    if (shape->color() != QColor(map[y][x]).rgb())
                        shape->setColor(map[y][x]);

                    if (shape->color() == QColor(Qt::black).rgb())
                        shape->draw(m_matrix->fadeOutSpeed());
                    else
                        shape->draw(m_matrix->fadeInSpeed());
                }
            }
        }
*/
        if (m_previewData.isEmpty() || map.isEmpty())
            return;

        QHashIterator<QLCPoint, GroupHead> it(m_group->headHash());
        while(it.hasNext())
        {
            it.next();

            QLCPoint pt(it.key());
            //GroupHead head(it.value());
            int ptIdx = pt.x() + (pt.y() * m_group->size().width());
            if (ptIdx < m_previewData.size())
                m_previewData[ptIdx] = QVariant(QColor(map[pt.y()][pt.x()]));
        }

        //qDebug() << "Preview data changed !";
        emit previewDataChanged(m_previewData);
    }
}

void RGBMatrixEditor::slotBeatReceived()
{
    m_gotBeat = true;
}

void RGBMatrixEditor::initPreviewData()
{
    m_previewTimer->stop();
    m_previewData.clear();
    QSize pSize = previewSize();

    for (int i = 0; i < pSize.width() * pSize.height(); i++)
        m_previewData.append(QVariant(QColor(0, 0, 0, 0)));

    if (m_matrix == NULL)
        return;

    m_previewStepHandler->initializeDirection(m_matrix->direction(), m_matrix->startColor(),
                                              m_matrix->endColor(), m_matrix->stepsCount());
    m_previewStepHandler->calculateColorDelta(m_matrix->startColor(), m_matrix->endColor());

    m_previewTimer->start(MasterTimer::tick());
}
