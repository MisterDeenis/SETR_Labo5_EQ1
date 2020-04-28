#!/bin/bash

scp $1/build/audioReceiver pi@192.168.1.30:/home/pi/projects/laboratoire5
scp $1/build/comEmitter pi@192.168.1.30:/home/pi/projects/laboratoire5

scp $1/build/audioEmitter pi@192.168.1.19:/home/pi/projects/laboratoire5
scp $1/build/comReceiver pi@192.168.1.19:/home/pi/projects/laboratoire5