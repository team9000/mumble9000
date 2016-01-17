FROM ubuntu:14.04

RUN apt-get update \
    && apt-get install -y build-essential pkg-config qt5-default \
        libboost-dev libssl-dev libprotobuf-dev protobuf-compiler libcap-dev

ADD . /src

RUN useradd -m murmur \
    && mv /src /home/murmur/src \
    && chown -R murmur:murmur /home/murmur

USER murmur

RUN cd /home/murmur/src \
    && qmake -recursive main.pro CONFIG+=no-client CONFIG+=no-ice CONFIG+=no-dbus CONFIG+=no-bonjour \
    && make \
    && mv release/murmurd /tmp/murmurd \
    && rm -Rf /home/murmur/src \
    && mv /tmp/murmurd /home/murmur/murmurd

CMD [ "/home/murmur/murmurd" ]
