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

  <properties>
      <undertow.version>1.4.18.Final</undertow.version>
      <jna.version>4.4.0</jna.version>
  </properties>

  <dependencies>

    <dependency>
        <groupId>net.java.dev.jna</groupId>
        <artifactId>jna</artifactId>
        <version>${jna.version}</version>
    </dependency>

    <dependency>
        <groupId>net.java.dev.jna</groupId>
        <artifactId>jna-platform</artifactId>
        <version>${jna.version}</version>
    </dependency>

    <dependency>
        <groupId>io.undertow</groupId>
        <artifactId>undertow-core</artifactId>
        <version>${undertow.version}</version>
    </dependency>

    <dependency>
        <groupId>io.undertow</groupId>
        <artifactId>undertow-servlet</artifactId>
        <version>${undertow.version}</version>
    </dependency>

    <dependency>
        <groupId>io.undertow</groupId>
        <artifactId>undertow-websockets-jsr</artifactId>
        <version>${undertow.version}</version>
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

docker run -it --rm -w='/home/src/'            \
  -e JAVA_TOOL_OPTIONS='-Dfile.encoding=UTF8'  \
  -v `pwd`:/home/src mspsim-java7 make jar

rm -rf ../mspsim-build/
mkdir ../mspsim-build/
mv mspsim.jar ../mspsim-build/mspsim.js.jar
cp -r lib/ ../mspsim-build/lib
cd -
cp readme mspsim-build/
cp -r ../ui/ng2/dist/ mspsim-build/webapp/
rm mspsim.js.zip
zip -r mspsim.js.zip mspsim-build/