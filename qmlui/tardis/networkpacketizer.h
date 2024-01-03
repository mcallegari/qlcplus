/*
  Q Light Controller Plus
  networkpacketizer.h

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

#ifndef NETWORKPACKETIZER_H
#define NETWORKPACKETIZER_H

#include <QByteArray>
#include <QVariant>

#define HEADER_LENGTH   7

class SimpleCrypt;

class NetworkPacketizer
{

public:
    NetworkPacketizer();

    enum
    {
        BoolType,
        IntType,
        FloatType,
        StringType,
        ByteArrayType,
        Vector3DType,
        RectFType,
        ColorType,
        FontType,
        SceneValueType,
        UIntPairType,
        StringIntPairType,
        StringDoublePairType,
        StringStringPairType
    };

    void initializePacket(QByteArray &packet, int opCode);
    void addSection(QByteArray &packet, QVariant value);

    QByteArray encryptPacket(QByteArray &packet, SimpleCrypt *crypter);
    int decodePacket(QByteArray &packet, int &opCode, QVariantList &sections, SimpleCrypt *decrypter);

private:

};

#endif /* NETWORKPACKETIZER_H */
