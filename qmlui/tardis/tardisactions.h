/*
  Q Light Controller Plus
  tardisactions.h

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

#ifndef TARDISACTIONS_H
#define TARDISACTIONS_H

#include <QVariant>

typedef struct
{
    int m_action;
    qint64 m_timestamp;
    quint32  m_objID;
    QVariant m_oldValue;
    QVariant m_newValue;
} TardisAction;

Q_DECLARE_METATYPE(TardisAction)

typedef QPair<quint32, uint> UIntPair;
Q_DECLARE_METATYPE(UIntPair)

typedef QPair<QString, int> StringIntPair;
Q_DECLARE_METATYPE(StringIntPair)

typedef QPair<QString, QString> StringStringPair;
Q_DECLARE_METATYPE(StringStringPair)

enum
{
    /* Fixture editing actions */
    FixtureCreate = 0x0000,
    FixtureDelete,
    FixtureMove,
    FixtureSetPosition,
    FixtureSetDumpValue,

    /* Fixture group editing actions */
    FixtureGroupCreate,
    FixtureGroupDelete,

    /* Function editing actions */
    FunctionCreate = 0x0100,
    FunctionDelete,
    FunctionSetName,
    FunctionSetPath,
    FunctionSetRunOrder,
    FunctionSetDirection,
    FunctionSetTempoType,
    FunctionSetFadeIn,
    FunctionSetFadeOut,
    FunctionSetDuration,

    SceneSetChannelValue,
    SceneUnsetChannelValue,

    ChaserAddStep,
    ChaserRemoveStep,
    ChaserSetStepFadeIn,
    ChaserSetStepHold,
    ChaserSetStepFadeOut,
    ChaserSetStepDuration,

    EFXAddFixture,
    EFXRemoveFixture,
    EFXSetAlgorithmIndex,
    EFXSetRelative,
    EFXSetWidth,
    EFXSetHeight,
    EFXSetXOffset,
    EFXSetYOffset,
    EFXSetRotation,
    EFXSetStartOffset,
    EFXSetXFrequency,
    EFXSetYFrequency,
    EFXSetXPhase,
    EFXSetYPhase,

    CollectionAddFunction,
    CollectionRemoveFunction,

    RGBMatrixSetFixtureGroup,
    RGBMatrixSetAlgorithmIndex,
    RGBMatrixSetStartColor,
    RGBMatrixSetEndColor,
    RGBMatrixSetScriptIntValue,
    RGBMatrixSetScriptStringValue,
    RGBMatrixSetText,
    RGBMatrixSetTextFont,
    RGBMatrixSetImage,
    RGBMatrixSetOffset,
    RGBMatrixSetAnimationStyle,

    AudioSetSource,

    VideoSetSource,
    VideoSetScreenIndex,
    VideoSetFullscreen,
    VideoSetGeometry,
    VideoSetRotation,

    /* Virtual console editing actions */
    VCWidgetCreate = 0xE000,
    VCWidgetDelete,
    VCWidgetGeometry,
    VCWidgetAllowResize,
    VCWidgetDisabled,
    VCWidgetVisible,
    VCWidgetCaption,
    VCWidgetBackgroundColor,
    VCWidgetBackgroundImage,
    VCWidgetForegroundColor,
    VCWidgetFont,
    VCWidgetPage,

    /* Virtual Console live actions */
    VCButtonSetState = 0xF000,

    /* Network protocol actions */
    NetAnnounce = 0xFF00,
    NetAnnounceReply,
    NetAuthentication,
    NetAuthenticationReply,
    NetPoll,
    NetPollReply,
    NetProjectTransfer
};


#endif /* TARDISACTIONS_H */
