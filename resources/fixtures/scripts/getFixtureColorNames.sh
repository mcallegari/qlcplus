#!/bin/bash
cd `dirname $0`/..
rgrep "ColorMacro" . | sed -E 's/^.* Res1="([^"]+)"[^>]*>([^<]+)<.*$/\2=\1/;' | sort -u
