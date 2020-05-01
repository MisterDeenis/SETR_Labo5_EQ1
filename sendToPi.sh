#!/bin/bash

scp $1/build/audioReceiver pi@192.168.0.176:/home/pi/projects/laboratoire5
scp $1/build/comEmitter pi@192.168.0.176:/home/pi/projects/laboratoire5
scp $1/build/distorsion pi@192.168.0.176:/home/pi/projects/laboratoire5
scp $1/build/delay pi@192.168.0.176:/home/pi/projects/laboratoire5
scp $1/build/highpass pi@192.168.0.176:/home/pi/projects/laboratoire5
scp $1/build/lowpass pi@192.168.0.176:/home/pi/projects/laboratoire5

scp $1/build/audioEmitter pi@192.168.0.129:/home/pi/projects/laboratoire5
scp $1/build/comReceiver pi@192.168.0.129:/home/pi/projects/laboratoire5