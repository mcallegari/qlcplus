#!/bin/bash

# Copy audio plugin directly under "audio/plugin" directory to allow to load these plugins
# First argument: building directory

cp $1/engine/audio/plugins/sndfile/*.so $1/engine/audio/plugins
cp $1/engine/audio/plugins/mad/*.so $1/engine/audio/plugins
