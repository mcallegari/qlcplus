/*
  Q Light Controller Plus - Unit test
  doc_test.h

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

#ifndef DOC_TEST_H
#define DOC_TEST_H

#include <QObject>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class Doc;
class Doc_Test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();

    void normalizeComponentPath();
    void denormalizeComponentPath();

    void defaults();
    void mode();

    void createFixtureId();
    void addFixture();
    void deleteFixture();
    void replaceFixtures();
    void fixture();
    void totalPowerConsumption();

    void addFixtureGroup();
    void removeFixtureGroup();

    void channelGroups();
    void palettes();

    void monitorProperties();

    void addFunction();
    void deleteFunction();
    void function();
    void usage();

    void load();
    void loadWrongRoot();
    void save();

private:
    void createFixtureNode(QXmlStreamWriter &doc, quint32 id, quint32 address, quint32 channels);
    void createFixtureGroupNode(QXmlStreamWriter &doc, quint32 id);
    void createCollectionNode(QXmlStreamWriter &doc, quint32 id);
    void createBusNode(QXmlStreamWriter &doc, quint32 id, quint32 value);

private:
    Doc* m_doc;
    int m_currentAddr;
};

#endif
