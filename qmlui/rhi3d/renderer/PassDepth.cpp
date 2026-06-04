/*
  Q Light Controller Plus
  PassDepth.cpp

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

#include "renderer/PassDepth.h"

void PassDepth::prepare(FrameContext &ctx)
{
    Q_UNUSED(ctx);
    // TODO: build depth-only pipeline state and targets.
}

void PassDepth::execute(FrameContext &ctx)
{
    Q_UNUSED(ctx);
    // TODO: render depth pre-pass.
}
