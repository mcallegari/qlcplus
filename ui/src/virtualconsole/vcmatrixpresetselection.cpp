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
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QDebug>
#include <QSettings>

#include "vcmatrixpresetselection.h"
#include "ui_vcmatrixpresetselection.h"
#include "rgbscriptscache.h"
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
 #include "rgbscript.h"
#else
 #include "rgbscriptv4.h"
#endif
#include "doc.h"

#define SETTINGS_GEOMETRY "vcmatrixpresetselection/geometry"

VCMatrixPresetSelection::VCMatrixPresetSelection(Doc *doc, QWidget *parent)
    : QDialog(parent)
    , m_doc(doc)
{
    Q_ASSERT(doc != NULL);

    setupUi(this);

    QSizePolicy sp = scrollArea->widget()->sizePolicy();
    sp.setHorizontalPolicy(QSizePolicy::Ignored);
    scrollArea->widget()->setSizePolicy(sp);

    QSettings settings;
    QVariant geometrySettings = settings.value(SETTINGS_GEOMETRY);
    if (geometrySettings.isValid() == true)
        restoreGeometry(geometrySettings.toByteArray());

    m_presetCombo->addItems(RGBAlgorithm::algorithms(m_doc));
    slotUpdatePresetProperties();
    connect(m_presetCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotUpdatePresetProperties()));
    connect(m_checkAll, SIGNAL(toggled(bool)),
            this, SLOT(slotCheckAllToggled(bool)));
}

VCMatrixPresetSelection::~VCMatrixPresetSelection()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
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
    m_dynamicProperties.clear();

    foreach (RGBScriptProperty prop, properties)
    {
        QCheckBox *propCheck = new QCheckBox(this);
        propCheck->setProperty("pName", prop.m_name);
        m_propertiesLayout->addWidget(propCheck, gridRowIdx, 2);
        connect(propCheck, SIGNAL(toggled(bool)),
                this, SLOT(slotPropertyCheckChanged(bool)));

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
                    propCombo->setCurrentText(pValue);

                connect(propCombo, SIGNAL(currentIndexChanged(int)),
                        this, SLOT(slotPropertyComboChanged(int)));

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
            case RGBScriptProperty::Float:
            {
                QLabel *propLabel = new QLabel(prop.m_displayName);
                m_propertiesLayout->addWidget(propLabel, gridRowIdx, 0);
                QDoubleSpinBox *propSpin = new QDoubleSpinBox(this);
                propSpin->setDecimals(3);
                propSpin->setRange(-1000000, 1000000);
                propSpin->setProperty("pName", prop.m_name);
                QString pValue = script->property(prop.m_name);
                if (!pValue.isEmpty())
                    propSpin->setValue(pValue.toDouble());

                connect(propSpin, SIGNAL(valueChanged(double)),
                        this, SLOT(slotPropertyDoubleSpinChanged(double)));

                m_propertiesLayout->addWidget(propSpin, gridRowIdx, 1);
                m_properties[prop.m_name] = pValue;
                gridRowIdx++;
            }
            break;
            case RGBScriptProperty::String:
            {
                QLabel *propLabel = new QLabel(prop.m_displayName);
                m_propertiesLayout->addWidget(propLabel, gridRowIdx, 0);
                QLineEdit *propEdit = new QLineEdit(this);
                propEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
                propEdit->setProperty("pName", prop.m_name);
                QString pValue = script->property(prop.m_name);

                if (!pValue.isEmpty())
                    propEdit->setText(pValue);

                connect(propEdit, SIGNAL(textEdited(QString)),
                        this, SLOT(slotPropertyEditChanged(QString)));

                m_propertiesLayout->addWidget(propEdit, gridRowIdx, 1);
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
    RGBScript* selScript = m_doc->rgbScriptsCache()->script(m_presetCombo->currentText());
    displayProperties(selScript);
    delete selScript;
}

void VCMatrixPresetSelection::slotPropertyComboChanged(int index)
{
    QComboBox *combo = qobject_cast<QComboBox *>(sender());
    QString pName = combo->property("pName").toString();
    QString pValue = combo->itemText(index);
    qDebug() << "Property combo changed to" << pValue;
    m_properties[pName] = pValue;
}

void VCMatrixPresetSelection::slotPropertySpinChanged(int value)
{
    qDebug() << "Property spin changed to" << value;
    QSpinBox *spin = qobject_cast<QSpinBox *>(sender());
    QString pName = spin->property("pName").toString();
    m_properties[pName] = QString::number(value);
}

void VCMatrixPresetSelection::slotPropertyDoubleSpinChanged(double value)
{
    qDebug() << "Property float changed to" << value;
    QDoubleSpinBox *spin = qobject_cast<QDoubleSpinBox *>(sender());
    QString pName = spin->property("pName").toString();
    m_properties[pName] = QString::number(value);
}

void VCMatrixPresetSelection::slotPropertyEditChanged(QString text)
{
    qDebug() << "Property string changed to" << text;
    QLineEdit *edit = qobject_cast<QLineEdit *>(sender());
    QString pName = edit->property("pName").toString();
    m_properties[pName] = text;
}

QString VCMatrixPresetSelection::selectedPreset()
{
    return m_presetCombo->currentText();
}

QMap<QString, QString> VCMatrixPresetSelection::customizedProperties()
{
    return m_properties;
}

QMap<QString, bool> VCMatrixPresetSelection::dynamicProperties() const
{
    return m_dynamicProperties;
}

void VCMatrixPresetSelection::slotPropertyCheckChanged(bool checked)
{
    QCheckBox *check = qobject_cast<QCheckBox *>(sender());
    QString pName = check->property("pName").toString();
    m_dynamicProperties[pName] = checked;
}

void VCMatrixPresetSelection::slotCheckAllToggled(bool checked)
{
    QLayout *layout = m_propertiesLayout->layout();
    for (int i = 0; i < layout->count(); ++i)
    {
        QWidget *widget = layout->itemAt(i)->widget();
        if (widget)
        {
            QCheckBox *checkbox = qobject_cast<QCheckBox*>(widget);
            if (checkbox)
                checkbox->setChecked(checked);
        }
    }
}
