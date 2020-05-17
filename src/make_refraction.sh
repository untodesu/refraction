#!/bin/bash
# Not sure this will even build on linux.

pushd `dirname $0`
devtools/bin/vpc  /refraction +compilers +game +shaders /mksln refraction
popd
