#!/bin/sh

set -e

docker images | grep -q mspsim-java7 || {
   echo "FROM ubuntu:14.04"
   echo "RUN apt-get update && apt-get install -y openjdk-7-jdk make"
} | docker build --no-cache -t mspsim-java7 -

git clone https://github.com/mspsim/mspsim.git mspsim-src
cd mspsim-src
git checkout 378edbb9b9ac4eab6d6c67a6d1dd32677eb3a3c3

# wget http://central.maven.org/maven2/net/java/dev/jna/jna/4.4.0/jna-4.4.0.jar --directory-prefix=./lib/

# cd mspsim-src && git add . && cd -
# cd mspsim-src && git diff HEAD se/sics/mspsim Makefile > ../proiot.patch && cd -
patch -p1 < ../proiot.patch

docker run -it --rm -w='/home/src/'            \
  -e JAVA_TOOL_OPTIONS='-Dfile.encoding=UTF8'  \
  -v `pwd`:/home/src mspsim-java7 make jar

rm -rf ../mspsim-build/
mkdir ../mspsim-build/
mv mspsim.jar ../mspsim-build/mspsim.js.jar
cp -r lib/ ../mspsim-build/lib
cd -
cp readme mspsim-build/
rm mspsim.js.zip
zip -r mspsim.js.zip mspsim-build/