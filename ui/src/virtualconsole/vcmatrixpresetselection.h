/*
  Q Light Controller Plus
  vcmatrixpresetselection.h

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

#ifndef VCMATRIXPRESETSELECTION_H
#define VCMATRIXPRESETSELECTION_H

#include <QDialog>

#include "ui_vcmatrixpresetselection.h"

class RGBScript;
class Doc;

class VCMatrixPresetSelection : public QDialog, public Ui_VCMatrixPresetSelection
{
    Q_OBJECT

public:
    VCMatrixPresetSelection(Doc *doc, QWidget *parent = 0);
    ~VCMatrixPresetSelection();

    QString selectedPreset();

    QHash<QString, QString> customizedProperties();

protected slots:
    void slotUpdatePresetProperties();

    void slotPropertyComboChanged(int index);
    void slotPropertySpinChanged(int value);
    void slotPropertyDoubleSpinChanged(double value);
    void slotPropertyEditChanged(QString text);

private:
    void resetProperties(QLayoutItem *item);
    void displayProperties(RGBScript *script);

private:
    Doc *m_doc;

    /** A map holding the customized script properties */
    QHash<QString, QString> m_properties;
};

#endif // VCMATRIXPRESETSELECTION_H
