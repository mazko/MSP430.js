#!/bin/sh

set -e

# develop:    ./build.sh
# production: ./build.sh -O3
# profiling:  ./build.sh -O3 --profiling

docker images | grep -q wsim-emsdk || {
   echo "FROM 42ua/emsdk"
   echo "RUN apt-get install -y automake"
} | docker build --no-cache -t wsim-emsdk -


for alias in 'emcc' 'emconfigure' 'emmake' 'emar' './bootstrap'; do
  # docker run -e 'EMCC_DEBUG=1' ...
  alias $alias="docker run -it --rm -m 2g -w='/home/src/wsim-src' -v `pwd`:/home/src wsim-emsdk $alias"
done
unset alias


if [ ! -d "wsim-src" ]; then
  git clone https://github.com/afrab/WSim.git wsim-src
  cd wsim-src
  git checkout 65f8e4b6d65ba730071c965d7dd0b2bc742b05ec

  # cd wsim-src && git add machine/embind.h && cd -
  # cd wsim-src && git diff HEAD > ../patches/am.patch && cd -
  patch -p1 < ../patches/am.patch

  ./bootstrap

  emconfigure ./configure --disable-platform-all \
    --enable-platform-wsn430 --disable-gui --disable-ptty \
    --disable-xdebug --disable-debug

  cd -

fi


# emmake make clean
emmake make CFLAGS="$1 -Werror -Wall -Wno-error=shift-negative-value"

# https://github.com/kripken/emscripten/blob/master/src/settings.js
# --profiling
# https://kripken.github.io/emscripten-site/docs/optimizing/Optimizing-Code.html#profiling
# https://nodejs.org/uk/docs/guides/simple-profiling/
# node --prof test.node.js && node --prof-process isolate-*.log

EMCFLAGS="$@ -DMSP430f1611 -Werror -Wall"

# clang has a -Weverything flag that, unlike "-Wall", will enable all compiler warnings
emcc $EMCFLAGS ../MSP430.cpp ../nop-tracer.cpp \
  -Wmissing-prototypes -o emccmspsim.lib.o

emcc $EMCFLAGS ../embind.cpp emccmspsim.lib.o \
  -s ERROR_ON_UNDEFINED_SYMBOLS=1 \
  --memory-init-file 0 --bind -o wsim-embind.js \
  arch/msp430/libmsp430f1611.a \
  libelf/libelf.a \
  liblogger/liblogger.a

{
  echo '
    var Module = {};
    Module.preRun = function() {
      FS.mkdir("/home/msp430sim");
      FS.chdir("/home/msp430sim");
      // console.log(FS.cwd());
      // console.log(FS.readdir("."));
    };'
  cat ./wsim-src/wsim-embind.js
} > wsim-embind.js

{
  echo 'function msp430_sim_module() {'
  cat ./wsim-embind.js
  echo 'return Module;'
  echo '}'
} > ../ui/ng2/src/assets/wsim.browser.js
