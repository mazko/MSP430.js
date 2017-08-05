#!/bin/sh

set -e

mkdir /tmp/pom-$$
cd /tmp/pom-$$
echo '<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 http://maven.apache.org/xsd/maven-4.0.0.xsd">
  <modelVersion>4.0.0</modelVersion>

  <groupId>test</groupId>
  <artifactId>wordgame</artifactId>
  <version>0.0.1-SNAPSHOT</version>

  <dependencies>

    <dependency>
        <groupId>javax.websocket</groupId>
        <artifactId>javax.websocket-api</artifactId>
        <version>1.1</version>
    </dependency>

    <dependency>
        <groupId>org.glassfish.tyrus</groupId>
        <artifactId>tyrus-server</artifactId>
        <version>1.13.1</version>
    </dependency>

    <dependency>
        <groupId>org.glassfish.tyrus</groupId>
        <artifactId>tyrus-container-grizzly-server</artifactId>
        <version>1.13.1</version>
    </dependency>

  </dependencies>

  <build>
    <plugins>
      <plugin>
        <groupId>org.apache.maven.plugins</groupId>
        <artifactId>maven-compiler-plugin</artifactId>
        <version>3.1</version>
        <configuration>
          <compilerVersion>1.7</compilerVersion>
          <source>1.7</source>
          <target>1.7</target>
        </configuration>
      </plugin>

      <plugin>
          <groupId>org.apache.maven.plugins</groupId>
          <artifactId>maven-dependency-plugin</artifactId>
          <executions>
              <execution>
                  <id>copy-dependencies</id>
                  <phase>prepare-package</phase>
                  <goals>
                      <goal>copy-dependencies</goal>
                  </goals>
                  <configuration>
                      <outputDirectory>${project.build.directory}/lib</outputDirectory>
                      <overWriteReleases>false</overWriteReleases>
                      <overWriteSnapshots>false</overWriteSnapshots>
                      <overWriteIfNewer>true</overWriteIfNewer>
                  </configuration>
              </execution>
          </executions>
      </plugin>
    </plugins>
  </build>
</project>
' > pom.xml
mvn clean install
cd -

docker images | grep -q mspsim-java7 || {
   echo "FROM ubuntu:14.04"
   echo "RUN apt-get update && apt-get install -y openjdk-7-jdk make"
} | docker build --no-cache -t mspsim-java7 -

git clone https://github.com/mspsim/mspsim.git mspsim-src
cd mspsim-src
git checkout 378edbb9b9ac4eab6d6c67a6d1dd32677eb3a3c3

cp /tmp/pom-$$/target/lib/* lib/

# cd mspsim-src && git add . && cd -
# cd mspsim-src && git diff HEAD se/sics/mspsim Makefile > ../proiot.patch && cd -
patch -p1 < ../proiot.patch
git add .

wget http://central.maven.org/maven2/net/java/dev/jna/jna/4.4.0/jna-4.4.0.jar --directory-prefix=./lib/

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