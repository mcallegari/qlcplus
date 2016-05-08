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
{
    m_view->rootContext()->setContextProperty("rgbMatrixEditor", this);

    connect(m_previewTimer, SIGNAL(timeout()), this, SLOT(slotPreviewTimeout()));
}

RGBMatrixEditor::~RGBMatrixEditor()
{
    m_previewTimer->stop();
    m_view->rootContext()->setContextProperty("rgbMatrixEditor", NULL);
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
        algo->setColors(m_matrix->startColor(), m_matrix->endColor());
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
    m_matrix->calculateColorDelta();

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
    m_matrix->calculateColorDelta();

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
        m_matrix->calculateColorDelta();
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

void RGBMatrixEditor::createScriptObjects(QQuickItem *parent)
{
    if (m_matrix == NULL || m_matrix->algorithm() == NULL ||
        m_matrix->algorithm()->type() != RGBAlgorithm::Script)
            return;

    QQmlComponent *labelComp = new QQmlComponent(m_view->engine(), QUrl("qrc:/RobotoText.qml"));
    if (labelComp->isError())
        qDebug() << labelComp->errors();

    RGBScript* script = static_cast<RGBScript*> (m_matrix->algorithm());
    QList<RGBScriptProperty> properties = script->properties();

    foreach(RGBScriptProperty prop, properties)
    {
        // always create a label first
        QQuickItem *propLabel = qobject_cast<QQuickItem*>(labelComp->create());
        propLabel->setParentItem(parent);
        propLabel->setProperty("label", prop.m_displayName);

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

    RGBScript *script = static_cast<RGBScript*> (m_matrix->algorithm());
    script->setProperty(paramName, value);
    m_matrix->setProperty(paramName, value);
}

void RGBMatrixEditor::setScriptIntProperty(QString paramName, int value)
{
    if (m_matrix == NULL || m_matrix->algorithm() == NULL ||
        m_matrix->algorithm()->type() != RGBAlgorithm::Script)
            return;

    RGBScript *script = static_cast<RGBScript*> (m_matrix->algorithm());
    script->setProperty(paramName, QString::number(value));
    m_matrix->setProperty(paramName, QString::number(value));
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

void RGBMatrixEditor::initPreviewData()
{
    m_previewTimer->stop();
    m_previewData.clear();
    QSize pSize = previewSize();

    for (int i = 0; i < pSize.width() * pSize.height(); i++)
        m_previewData.append(QVariant(QColor(0, 0, 0, 0)));

    if (m_matrix == NULL)
        return;

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
    m_previewTimer->start(MasterTimer::tick());
}
