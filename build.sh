#!/bin/bash
export DEVKITPRO=/c/devkitpro
export DEVKITARM=/c/devkitpro/devkitARM
export PATH=/c/devkitpro/devkitARM/bin:/c/devkitpro/tools/bin:/c/devkitpro/msys2/usr/bin:$PATH
cd "/e/pokemon recomp"
make "$@"
