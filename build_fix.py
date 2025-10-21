#!/usr/bin/python3

import os

Colors = ["Red", "Green", "Blue"]


for i in range(0, 480):
    bar = int(i/30) + 1
    pixel = (i/3 % 10) + 1
    color = Colors[(i ) % 3]
    print('<Channel Name="{} {}-{}" Preset="Intensity{}"/>'.format(color, bar, pixel, color))

for i in range(0, 480):
    bar = int(i/30) + 1
    pixel = (i/3 % 10) + 1
    color = Colors[(i ) % 3]
    print('<Channel Number="{}">{} {}-{}</Channel>'.format(i, color, bar, pixel))

# print("<Head>")
#for i in range(0, 480):
#    print('   <Channel>{}</Channel>'.format(i))
#    if i % 3 == 2:
#        print("</Head>")
#        print("<Head>")
#print("</Head>")


