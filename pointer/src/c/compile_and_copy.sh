#!/bin/bash

gcc -DMI_SERVBUILD -fPIC -I$INFORMIXDIR/incl/public -I$INFORMIXDIR/incl/esql -I$INFORMIXDIR/incl -g -shared -Bsymbolic -c pointer.c
ld -dy -G -Bsymbolic -o pointer.so pointer.o
cp pointer.so $INFORMIXDIR/extend
