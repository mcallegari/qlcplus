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

enum
{
    /* Virtual console editing actions */
    VCWidgetGeometry = 0xE000,
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
    VCButtonState = 0xF000,

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
