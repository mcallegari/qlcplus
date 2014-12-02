/*
  Q Light Controller Plus
  vcmatrixpresetselection.cpp

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

#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QDebug>

#include "vcmatrixpresetselection.h"
#include "ui_vcmatrixpresetselection.h"
#include "rgbscript.h"
#include "rgbscriptscache.h"
#include "doc.h"

VCMatrixPresetSelection::VCMatrixPresetSelection(Doc *doc, QWidget *parent)
    : QDialog(parent)
    , m_doc(doc)
{
    Q_ASSERT(doc != NULL);

    setupUi(this);
    m_presetCombo->addItems(m_doc->rgbScriptsCache()->names());
    slotUpdatePresetProperties();
    connect(m_presetCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotUpdatePresetProperties()));
}

VCMatrixPresetSelection::~VCMatrixPresetSelection()
{
}

/**
 * Helper function. Deletes all child widgets of the given layout @a item.
 */
void VCMatrixPresetSelection::resetProperties(QLayoutItem *item)
{
    if (item->layout()) {
        // Process all child items recursively.
        for (int i = item->layout()->count() - 1; i >= 0; i--)
            resetProperties(item->layout()->itemAt(i));
    }
    delete item->widget();
}

void VCMatrixPresetSelection::displayProperties(RGBScript *script)
{
    if (script == NULL)
        return;

    int gridRowIdx = 0;

    QList<RGBScriptProperty> properties = script->properties();
    if (properties.count() > 0)
        m_propertiesGroup->show();
    else
        m_propertiesGroup->hide();

    m_properties.clear();

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
                QString pValue = script->property(prop.m_name);
                if (!pValue.isEmpty())
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
                    propCombo->setCurrentText(pValue);
#else
                    propCombo->setCurrentIndex(propCombo->findText(pValue));
#endif
                connect(propCombo, SIGNAL(currentIndexChanged(QString)),
                        this, SLOT(slotPropertyComboChanged(QString)));
                m_propertiesLayout->addWidget(propCombo, gridRowIdx, 1);
                m_properties[prop.m_name] = pValue;
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
                QString pValue = script->property(prop.m_name);
                if (!pValue.isEmpty())
                    propSpin->setValue(pValue.toInt());
                connect(propSpin, SIGNAL(valueChanged(int)),
                        this, SLOT(slotPropertySpinChanged(int)));
                m_propertiesLayout->addWidget(propSpin, gridRowIdx, 1);
                m_properties[prop.m_name] = pValue;
                gridRowIdx++;
            }
            break;
            default:
                qWarning() << "Type" << prop.m_type << "not handled yet";
            break;
        }
    }
}

void VCMatrixPresetSelection::slotUpdatePresetProperties()
{
    resetProperties(m_propertiesLayout->layout());
    RGBScript selScript = m_doc->rgbScriptsCache()->script(m_presetCombo->currentText());
    displayProperties(&selScript);
}

void VCMatrixPresetSelection::slotPropertyComboChanged(QString value)
{
    qDebug() << "Property combo changed to" << value;
    QComboBox *combo = (QComboBox *)sender();
    QString pName = combo->property("pName").toString();
    m_properties[pName] = value;
}

void VCMatrixPresetSelection::slotPropertySpinChanged(int value)
{
    qDebug() << "Property spin changed to" << value;
    QSpinBox *spin = (QSpinBox *)sender();
    QString pName = spin->property("pName").toString();
    m_properties[pName] = QString::number(value);
}

QString VCMatrixPresetSelection::selectedPreset()
{
    return m_presetCombo->currentText();
}

QHash<QString, QString> VCMatrixPresetSelection::customizedProperties()
{
    return m_properties;
}

