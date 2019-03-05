#!/bin/bash
sudo su - pi -c "/usr/bin/screen -S bldd -d -m $*/run-blinky-lite.sh $*"

