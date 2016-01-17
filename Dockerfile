FROM ubuntu:14.04

RUN apt-get update \
    && apt-get install -y build-essential pkg-config qt5-default \
        libboost-dev libssl-dev libprotobuf-dev protobuf-compiler libcap-dev

ADD . /src

RUN cd /src \
    && qmake -recursive main.pro CONFIG+=no-client CONFIG+=no-ice CONFIG+=no-dbus CONFIG+=no-bonjour \
    && make
