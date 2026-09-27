/*
  Q Light Controller Plus
  webaccessupload_test.cpp

  Copyright (c) Q Light Controller Plus

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

#include <QTest>

#include "webaccessupload.h"
#include "webaccessupload_test.h"

void WebAccessUpload_Test::acceptsFixtureFileNames()
{
    QVERIFY(isPlainUploadedFileName("fixture.qxf"));
    QVERIFY(isPlainUploadedFileName("fixture.D4"));
}

void WebAccessUpload_Test::rejectsPathComponents()
{
    QVERIFY(isPlainUploadedFileName("../fixture.qxf") == false);
    QVERIFY(isPlainUploadedFileName("..\\fixture.qxf") == false);
    QVERIFY(isPlainUploadedFileName("/tmp/fixture.qxf") == false);
    QVERIFY(isPlainUploadedFileName("dir/fixture.qxf") == false);
}

void WebAccessUpload_Test::rejectsReservedAndEmptyNames()
{
    QVERIFY(isPlainUploadedFileName("") == false);
    QVERIFY(isPlainUploadedFileName(".") == false);
    QVERIFY(isPlainUploadedFileName("..") == false);
}

void WebAccessUpload_Test::rejectsUnexpectedExtensions()
{
    QVERIFY(isPlainUploadedFileName("fixture.php") == false);
    QVERIFY(isPlainUploadedFileName("fixture.qxf.php") == false);
    QVERIFY(isPlainUploadedFileName("fixture") == false);
}

void WebAccessUpload_Test::rejectsOverlongNames()
{
    QVERIFY(isPlainUploadedFileName(QString(251, 'a') + ".qxf"));
    QVERIFY(isPlainUploadedFileName(QString(252, 'a') + ".qxf") == false);
}

QTEST_MAIN(WebAccessUpload_Test)
