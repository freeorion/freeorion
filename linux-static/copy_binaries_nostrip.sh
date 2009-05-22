#!/bin/sh
# FreeorionD and FreeorionCA need to be named without -static
for F in freeorion freeoriond freeorionca; do
    cp -va ${F}-static /usr/local/games/freeorion/$F
#    strip --strip-all /usr/local/games/freeorion/$F
done
