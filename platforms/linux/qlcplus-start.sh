#!/usr/bin/env bash
#
#  Q Light Controller Plus
#  qlcplus-start.sh
#
#  Copyright (c) 2024 Massimo Callegari
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0.txt
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

QLCPLUS_OPTS="-platform eglfs --nowm --web --web-auth --operate --overscan"

if [ -ne $HOME/.qlcplus/eglfs.json ]; then
	echo '{ "device": "/dev/dri/card1" }' > $HOME/.qlcplus/eglfs.json
fi

if [ -e /root/.qlcplus/autostart.qxw ]; then
        QLCPLUS_OPTS="$QLCPLUS_OPTS --open /root/.qlcplus/autostart.qxw"
fi

# if NTP hasn't done its job already, set the date to modern age...
CURRDATE=`date +%Y`
if [ "$CURRDATE" -lt "2024" ]; then
date +%Y%m%d -s "20240313"
fi

export QT_QPA_EGLFS_PHYSICAL_WIDTH=320
export QT_QPA_EGLFS_PHYSICAL_HEIGHT=200
export QT_QPA_EGLFS_ALWAYS_SET_MODE=1
export QT_QPA_EGLFS_KMS_CONFIG=$HOME/.qlcplus/eglfs.json

/usr/bin/qlcplus $QLCPLUS_OPTS
