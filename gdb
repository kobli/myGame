#!/bin/sh
export LD_LIBRARY_PATH=./extlibs/SFML-2.3.2/lib/lin_64:$LD_LIBRARY_PATH
gdb myGame
