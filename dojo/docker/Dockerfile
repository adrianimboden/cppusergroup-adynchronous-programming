FROM ubuntu:bionic

COPY red_amber_green.rb /usr/local/bin

RUN \
  apt-get update && \
  apt-get -y install mercurial git curl xz-utils libc6-dev libgcc-6-dev python3 python3-jinja2 python3-distutils-extra build-essential && \
  curl https://scm.svc.firenet.ch/thingdust/external/binaries/rawfile/fd835dd6b6423b789d4b73b14fd1d62b7926b4de/clang60.tar.xz -o /root/clang.tar.xz && \
  tar -xvf /root/clang.tar.xz -C / && \
  apt-get -y purge curl xz-utils && \
  rm -rf /var/lib/apt/lists/* && \
  rm /root/clang.tar.xz && \
  mkdir -p /prebuilt

ENV CC clang
ENV CXX clang++
COPY build.py /build.py
RUN /build.py
