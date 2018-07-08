#!/usr/bin/env bash
#
#  Q Light Controller Plus
#  check-licenses.sh
#
#  Copyright (c) 2015 Dave Olsthoorn
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


##variable declaration
#licensecheck binary location
LICENSECHECK_BIN="$(which licensecheck 2> /dev/null)"
#regex for ignore
FILE_REGEX=""
#array with file extentions to check
FILE_EXT[0]=""

##config
#file extentions
FILE_EXT[0]="cpp"
FILE_EXT[1]="c"
FILE_EXT[2]="h"
FILE_EXT[3]="js"
FILE_EXT[4]="qml"
FILE_EXT[5]="ui"
#FILE_EXT[6]="xml"

##Functions
#Join file extentions to group
function generate_regex {
 TMP_IFS="$IFS"
 local IFS="|"
 local VALUE="${FILE_EXT[*]}"
 FILE_REGEX="\.(${VALUE})$"
 IFS="$TMP_IFS"
}

generate_regex

##running licensecheck
#check if licensecheck is installed
if [ "$LICENSECHECK_BIN" == ""  ]; then
 echo "licensecheck not found..."
 exit 1
fi

#catch output in a variable
OUTPUT=$($LICENSECHECK_BIN -r ./ -c $FILE_REGEX)

#put the lines in a array
TMP_IFS=$IFS
IFS=$'\n'
OUTPUT_ARRAY=($OUTPUT)
IFS=$TMP_IFS

#print lines
for line in "${OUTPUT_ARRAY[@]}"; do
 #but only with UNKNOWN license
 echo "$line" | grep UNKNOWN
done
