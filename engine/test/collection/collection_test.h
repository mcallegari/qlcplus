/*
  Q Light Controller - Unit test
  collection_test.h

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

#ifndef COLLECTION_TEST_H
#define COLLECTION_TEST_H

#include <QObject>

class Doc;
class Collection_Test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void initial();
    void functions();
    void contains();
    void functionRemoval();

    void loadSuccess();
    void loadWrongType();
    void loadWrongRoot();
    void loadWrongMemberTag();
    void loadPostLoad();
    void save();
    void copyFrom();
    void createCopy();

    void write();

    void stopNotOwnChildren();

private:
    Doc* m_doc;
};

#endif
