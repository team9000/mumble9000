FROM ubuntu:14.04

RUN apt-get update \
    && apt-get install build-essential pkg-config qt5-default \
        libboost-dev libssl-dev libprotobuf-dev protobuf-compiler

ADD . /src

RUN cd /src \
    && qmake -recursive main.pro CONFIG+=no-client CONFIG+=no-ice CONFIG+=no-dbus CONFIG+=no-bonjour \
    && make
