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
#include "tardis.h"
#include "doc.h"

RGBMatrixEditor::RGBMatrixEditor(QQuickView *view, Doc *doc, QObject *parent)
    : FunctionEditor(view, doc, parent)
    , m_matrix(nullptr)
    , m_group(nullptr)
    , m_previewTimer(new QTimer(this))
    , m_previewElapsed(0)
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
    m_view->rootContext()->setContextProperty("rgbMatrixEditor", nullptr);
    delete m_previewStepHandler;
}

void RGBMatrixEditor::setFunctionID(quint32 id)
{
    if (id == Function::invalidId())
    {
        m_matrix = nullptr;
        m_group = nullptr;
        return;
    }

    m_matrix = qobject_cast<RGBMatrix *>(m_doc->function(id));
    if (m_matrix == nullptr)
        return;

    m_group = m_doc->fixtureGroup(m_matrix->fixtureGroup());

    if (m_group == nullptr)
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
    if (m_matrix == nullptr || m_group == nullptr)
        return -1;

    return m_group->id();
}

void RGBMatrixEditor::setFixtureGroup(int fixtureGroup)
{
    if (m_matrix == nullptr || m_matrix->fixtureGroup() == (quint32)fixtureGroup)
        return;

    QMutexLocker locker(&m_previewMutex);
    m_previewTimer->stop();
    m_group = m_doc->fixtureGroup(fixtureGroup);
    Tardis::instance()->enqueueAction(Tardis::RGBMatrixSetFixtureGroup, m_matrix->id(), m_matrix->fixtureGroup(), fixtureGroup);
    m_matrix->setFixtureGroup((quint32)fixtureGroup);
    emit fixtureGroupChanged(fixtureGroup);
    emit previewSizeChanged();
    initPreviewData();
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
    if (m_matrix == nullptr || m_matrix->algorithm() == nullptr)
        return -1;

    QStringList algoList = algorithms();
    return algoList.indexOf(m_matrix->algorithm()->name());
}

void RGBMatrixEditor::setAlgorithmIndex(int algoIndex)
{
    qDebug() << "Set algorithm:" << algoIndex;
    QStringList algoList = algorithms();
    if (algoIndex < 0 || algoIndex >= algorithms().count())
        return;

    RGBAlgorithm *algo = RGBAlgorithm::algorithm(m_doc, algoList.at(algoIndex));
    if (algo != nullptr)
    {
        /** if we're setting the same algorithm, then there's nothing to do */
        if (m_matrix->algorithm() != nullptr && m_matrix->algorithm()->name() == algo->name())
            return;

        Q_ASSERT(5 == RGBAlgorithmColorDisplayCount);
        QVector<QColor> colors = {
                m_matrix->getColor(0),
                m_matrix->getColor(1),
                m_matrix->getColor(2),
                m_matrix->getColor(3),
                m_matrix->getColor(4)
        };
        algo->setColors(colors);
    }

    Tardis::instance()->enqueueAction(Tardis::RGBMatrixSetAlgorithmIndex, m_matrix->id(), algorithmIndex(), algoIndex);
    m_matrix->setAlgorithm(algo);

    initPreviewData();

    emit algorithmIndexChanged();
    emit algoColorsChanged();
}

int RGBMatrixEditor::algoColors()
{
    if (m_matrix == nullptr || m_matrix->algorithm() == nullptr)
        return 0;

    return m_matrix->algorithm()->acceptColors();
}

QColor RGBMatrixEditor::colorAtIndex(int index)
{
    if (m_matrix == nullptr || m_matrix->algorithm() == nullptr)
        return QColor();

    return m_matrix->getColor(index);
}

void RGBMatrixEditor::setColorAtIndex(int index, QColor color)
{
    if (m_matrix == nullptr || m_matrix->getColor(index) == color)
        return;

    Tardis::instance()->enqueueAction(Tardis::RGBMatrixSetColor1, m_matrix->id(), m_matrix->getColor(index), color);
    m_matrix->setColor(index, color);
    if (index < 2)
        m_previewStepHandler->calculateColorDelta(m_matrix->getColor(0), m_matrix->getColor(1), m_matrix->algorithm());
}

void RGBMatrixEditor::resetColorAtIndex(int index)
{
    if (m_matrix == nullptr || m_matrix->getColor(index).isValid() == false)
        return;

    m_matrix->setColor(index, QColor());
    if (index < 2)
        m_previewStepHandler->calculateColorDelta(m_matrix->getColor(0), m_matrix->getColor(1), m_matrix->algorithm());
}

bool RGBMatrixEditor::hasColorAtIndex(int index)
{
    if (m_matrix == nullptr || m_matrix->getColor(index).isValid() == false)
        return false;

    return true;
}

QString RGBMatrixEditor::algoText() const
{
    if (m_matrix != nullptr && m_matrix->algorithm() != nullptr &&
        m_matrix->algorithm()->type() == RGBAlgorithm::Text)
    {
        RGBText *algo = static_cast<RGBText*> (m_matrix->algorithm());
        return algo->text();
    }

    return QString(" Q LIGHT CONTROLLER + ");
}

void RGBMatrixEditor::setAlgoText(QString text)
{
    if (m_matrix != nullptr && m_matrix->algorithm() != nullptr &&
        m_matrix->algorithm()->type() == RGBAlgorithm::Text)
    {
        RGBText *algo = static_cast<RGBText*> (m_matrix->algorithm());
        if (algo->text() == text)
            return;

        Tardis::instance()->enqueueAction(Tardis::RGBMatrixSetText, m_matrix->id(), algo->text(), text);
        QMutexLocker algorithmLocker(&m_matrix->algorithmMutex());
        algo->setText(text);
        emit algoTextChanged(text);
    }
}

QFont RGBMatrixEditor::algoTextFont() const
{
    if (m_matrix != nullptr && m_matrix->algorithm() != nullptr &&
        m_matrix->algorithm()->type() == RGBAlgorithm::Text)
    {
        RGBText *algo = static_cast<RGBText*> (m_matrix->algorithm());
        return algo->font();
    }
    return QFont();
}

void RGBMatrixEditor::setAlgoTextFont(QFont algoTextFont)
{
    if (m_matrix != nullptr && m_matrix->algorithm() != nullptr &&
        m_matrix->algorithm()->type() == RGBAlgorithm::Text)
    {
        RGBText *algo = static_cast<RGBText*> (m_matrix->algorithm());
        if (algo->font() == algoTextFont)
            return;

        Tardis::instance()->enqueueAction(Tardis::RGBMatrixSetTextFont, m_matrix->id(), algo->font(), algoTextFont);
        QMutexLocker algorithmLocker(&m_matrix->algorithmMutex());
        algo->setFont(algoTextFont);
        emit algoTextFontChanged(algoTextFont);
    }
}

QString RGBMatrixEditor::algoImagePath() const
{
    if (m_matrix != nullptr && m_matrix->algorithm() != nullptr &&
        m_matrix->algorithm()->type() == RGBAlgorithm::Image)
    {
        RGBImage *algo = static_cast<RGBImage*> (m_matrix->algorithm());
        return algo->filename();
    }

    return QString();
}

void RGBMatrixEditor::setAlgoImagePath(QString path)
{
    if (m_matrix != nullptr && m_matrix->algorithm() != nullptr &&
        m_matrix->algorithm()->type() == RGBAlgorithm::Image)
    {
        RGBImage *algo = static_cast<RGBImage*> (m_matrix->algorithm());

        if (path.startsWith("file:"))
            path = QUrl(path).toLocalFile();

        if (algo->filename() == path)
            return;

        Tardis::instance()->enqueueAction(Tardis::RGBMatrixSetImage, m_matrix->id(), algo->filename(), path);
        QMutexLocker algorithmLocker(&m_matrix->algorithmMutex());
        algo->setFilename(path);
        emit algoImagePathChanged(path);
    }
}

QSize RGBMatrixEditor::algoOffset() const
{
    if (m_matrix != nullptr && m_matrix->algorithm() != nullptr)
    {
        if (m_matrix->algorithm()->type() == RGBAlgorithm::Image)
        {
            RGBImage *algo = static_cast<RGBImage*> (m_matrix->algorithm());
            return QSize(algo->xOffset(), algo->yOffset());
        }
        else if (m_matrix->algorithm()->type() == RGBAlgorithm::Text)
        {
            RGBText *algo = static_cast<RGBText*> (m_matrix->algorithm());
            return QSize(algo->xOffset(), algo->yOffset());
        }
    }
    return QSize(0, 0);
}

void RGBMatrixEditor::setAlgoOffset(QSize algoOffset)
{
    if (m_matrix != nullptr && m_matrix->algorithm() != nullptr)
    {
        if (m_matrix->algorithm()->type() == RGBAlgorithm::Image)
        {
            RGBImage *algo = static_cast<RGBImage*> (m_matrix->algorithm());
            if (algo->xOffset() == algoOffset.width() && algo->yOffset() == algoOffset.height())
                return;

            Tardis::instance()->enqueueAction(Tardis::RGBMatrixSetOffset, m_matrix->id(),
                                              QSize(algo->xOffset(), algo->yOffset()), algoOffset);
            QMutexLocker algorithmLocker(&m_matrix->algorithmMutex());
            algo->setXOffset(algoOffset.width());
            algo->setYOffset(algoOffset.height());
            emit algoOffsetChanged(algoOffset);
        }
        else if (m_matrix->algorithm()->type() == RGBAlgorithm::Text)
        {
            RGBText *algo = static_cast<RGBText*> (m_matrix->algorithm());
            if (algo->xOffset() == algoOffset.width() && algo->yOffset() == algoOffset.height())
                return;

            Tardis::instance()->enqueueAction(Tardis::RGBMatrixSetOffset, m_matrix->id(),
                                              QSize(algo->xOffset(), algo->yOffset()), algoOffset);
            QMutexLocker algorithmLocker(&m_matrix->algorithmMutex());
            algo->setXOffset(algoOffset.width());
            algo->setYOffset(algoOffset.height());
            emit algoOffsetChanged(algoOffset);
        }
    }
}

int RGBMatrixEditor::animationStyle() const
{
    if (m_matrix != nullptr && m_matrix->algorithm() != nullptr)
    {
        if (m_matrix->algorithm()->type() == RGBAlgorithm::Image)
        {
            RGBImage *algo = static_cast<RGBImage*> (m_matrix->algorithm());
            return (int)algo->animationStyle();
        }
        else if (m_matrix->algorithm()->type() == RGBAlgorithm::Text)
        {
            RGBText *algo = static_cast<RGBText*> (m_matrix->algorithm());
            return (int)algo->animationStyle();
        }
    }
    return 0;
}

void RGBMatrixEditor::setAnimationStyle(int style)
{
    if (m_matrix != nullptr && m_matrix->algorithm() != nullptr)
    {
        if (m_matrix->algorithm()->type() == RGBAlgorithm::Image)
        {
            RGBImage *algo = static_cast<RGBImage*> (m_matrix->algorithm());
            if ((int)algo->animationStyle() == style)
                return;

            Tardis::instance()->enqueueAction(Tardis::RGBMatrixSetAnimationStyle, m_matrix->id(), (int)algo->animationStyle(), style);
            QMutexLocker algorithmLocker(&m_matrix->algorithmMutex());
            algo->setAnimationStyle(RGBImage::AnimationStyle(style));
            emit animationStyleChanged(style);
        }
        else if (m_matrix->algorithm()->type() == RGBAlgorithm::Text)
        {
            RGBText *algo = static_cast<RGBText*> (m_matrix->algorithm());
            if ((int)algo->animationStyle() == style)
                return;

            Tardis::instance()->enqueueAction(Tardis::RGBMatrixSetAnimationStyle, m_matrix->id(), (int)algo->animationStyle(), style);
            QMutexLocker algorithmLocker(&m_matrix->algorithmMutex());
            algo->setAnimationStyle(RGBText::AnimationStyle(style));
            emit animationStyleChanged(style);
        }
    }
}

void RGBMatrixEditor::createScriptObjects(QQuickItem *parent)
{
    if (m_matrix == nullptr || m_matrix->algorithm() == nullptr ||
            m_matrix->algorithm()->type() != RGBAlgorithm::Script)
        return;

    RGBScript *script = static_cast<RGBScript*> (m_matrix->algorithm());
    QList<RGBScriptProperty> properties = script->properties();

    foreach (RGBScriptProperty prop, properties)
    {
        // always create a label first
        QMetaObject::invokeMethod(parent, "addLabel",
                Q_ARG(QVariant, prop.m_displayName));
        QString pValue = m_matrix->property(prop.m_name);

        switch(prop.m_type)
        {
            case RGBScriptProperty::List:
            {
                QVariantList valList;
                int idx = 0;
                int currIdx = 0;

                foreach (QString val, prop.m_listValues)
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
                QMetaObject::invokeMethod(parent, "addSpinBox",
                        Q_ARG(QVariant, prop.m_name),
                        Q_ARG(QVariant, prop.m_rangeMinValue),
                        Q_ARG(QVariant, prop.m_rangeMaxValue),
                        Q_ARG(QVariant, pValue.toInt()));
            }
            break;
            case RGBScriptProperty::Float:
            {
                QMetaObject::invokeMethod(parent, "addDoubleSpinBox",
                                          Q_ARG(QVariant, prop.m_name),
                                          Q_ARG(QVariant, pValue.toDouble()));
            }
            break;
            case RGBScriptProperty::String:
            {
                QMetaObject::invokeMethod(parent, "addTextEdit",
                                          Q_ARG(QVariant, prop.m_name),
                                          Q_ARG(QVariant, pValue));
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
    if (m_matrix == nullptr || m_matrix->algorithm() == nullptr ||
        m_matrix->algorithm()->type() != RGBAlgorithm::Script)
            return;

    qDebug() << "[setScriptStringProperty] param:" << paramName << ", value:" << value;

    StringStringPair oldValue(paramName, m_matrix->property(paramName));
    Tardis::instance()->enqueueAction(Tardis::RGBMatrixSetScriptStringValue, m_matrix->id(), QVariant::fromValue(oldValue),
                                      QVariant::fromValue(StringStringPair(paramName, value)));

    m_matrix->setProperty(paramName, value);
    emit algoColorsChanged();
}

void RGBMatrixEditor::setScriptIntProperty(QString paramName, int value)
{
    if (m_matrix == nullptr || m_matrix->algorithm() == nullptr ||
        m_matrix->algorithm()->type() != RGBAlgorithm::Script)
            return;

    qDebug() << "[setScriptIntProperty] param:" << paramName << ", value:" << value;

    StringIntPair oldValue(paramName, m_matrix->property(paramName).toInt());
    Tardis::instance()->enqueueAction(Tardis::RGBMatrixSetScriptIntValue, m_matrix->id(), QVariant::fromValue(oldValue),
                                      QVariant::fromValue(StringIntPair(paramName, value)));
    m_matrix->setProperty(paramName, QString::number(value));
}

void RGBMatrixEditor::setScriptFloatProperty(QString paramName, double value)
{
    if (m_matrix == nullptr || m_matrix->algorithm() == nullptr ||
        m_matrix->algorithm()->type() != RGBAlgorithm::Script)
        return;

    qDebug() << "[setScriptIntProperty] param:" << paramName << ", value:" << value;

    StringDoublePair oldValue(paramName, m_matrix->property(paramName).toDouble());
    Tardis::instance()->enqueueAction(Tardis::RGBMatrixSetScriptDoubleValue, m_matrix->id(), QVariant::fromValue(oldValue),
                                      QVariant::fromValue(StringDoublePair(paramName, value)));
    m_matrix->setProperty(paramName, QString::number(value));
}

/************************************************************************
 * Blend mode
 ************************************************************************/

int RGBMatrixEditor::blendMode() const
{
    if (m_matrix == nullptr)
        return 0;

    return m_matrix->blendMode();
}

void RGBMatrixEditor::setBlendMode(int mode)
{
    if (m_matrix == nullptr || mode == m_matrix->blendMode())
        return;

    m_matrix->setBlendMode(Universe::BlendMode(mode));

    emit blendModeChanged();
}

/************************************************************************
 * Control mode
 ************************************************************************/

int RGBMatrixEditor::controlMode() const
{
    if (m_matrix == nullptr)
        return 0;

    return m_matrix->controlMode();
}

void RGBMatrixEditor::setControlMode(int mode)
{
    if (m_matrix == nullptr || mode == m_matrix->controlMode())
        return;

    m_matrix->setControlMode(RGBMatrix::ControlMode(mode));

    emit controlModeChanged();
}

/************************************************************************
 * Preview
 ************************************************************************/

QSize RGBMatrixEditor::previewSize() const
{
    if (m_matrix == nullptr || m_group == nullptr)
        return QSize(0, 0);

    return m_group->size();
}

QVariantList RGBMatrixEditor::previewData() const
{
    return m_previewData;
}

void RGBMatrixEditor::slotPreviewTimeout()
{
    if (m_matrix == nullptr || m_group == nullptr || m_matrix->duration() <= 0)
        return;

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
        QMutexLocker locker(&m_previewMutex);

        m_previewStepHandler->checkNextStep(m_matrix->runOrder(), m_matrix->getColor(0),
                                            m_matrix->getColor(1), m_matrix->stepsCount());

        m_matrix->previewMap(m_previewStepHandler->currentStepIndex(), m_previewStepHandler);

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
        if (m_previewData.isEmpty() || m_previewStepHandler->m_map.isEmpty())
            return;

        QMapIterator<QLCPoint, GroupHead> it(m_group->headsMap());
        while (it.hasNext())
        {
            it.next();

            QLCPoint pt(it.key());
            //GroupHead head(it.value());
            int ptIdx = pt.x() + (pt.y() * m_group->size().width());
            if (ptIdx < m_previewData.size())
                m_previewData[ptIdx] = QVariant(QColor(m_previewStepHandler->m_map[pt.y()][pt.x()]));
        }

        //qDebug() << "Preview data changed!";
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
    m_previewElapsed = 0;
    m_previewData.clear();
    QSize pSize = previewSize();

    for (int i = 0; i < pSize.width() * pSize.height(); i++)
        m_previewData.append(QVariant(QColor(0, 0, 0, 0)));

    if (m_matrix == nullptr)
        return;

    m_previewStepHandler->initializeDirection(m_matrix->direction(), m_matrix->getColor(0),
                                              m_matrix->getColor(1), m_matrix->stepsCount(), m_matrix->algorithm());
    m_previewStepHandler->calculateColorDelta(m_matrix->getColor(0), m_matrix->getColor(1), m_matrix->algorithm());

    m_previewTimer->start(MasterTimer::tick());
}
