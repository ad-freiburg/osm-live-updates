# Copyright 2024, University of Freiburg
# Authors: Nicolas von Trott <nicolasvontrott@gmail.com>.

# This file is part of osm-live-updates.
#
# osm-live-updates is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# osm-live-updates is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with osm-live-updates.  If not, see <https://www.gnu.org/licenses/>.

FROM ubuntu:20.04
LABEL authors="nicolasvontrott"
LABEL Description="Build environment"

ENV HOME /root
ENV TZ=Europe/Berlin

ARG DEBIAN_FRONTEND=noninteractive

SHELL ["/bin/bash", "-c"]

RUN apt-get update && apt-get -y --no-install-recommends install \
    build-essential \
    clang \
    cmake \
    gdb \
    wget \
    git \
    libcurl4-openssl-dev \
    clang-tidy \
    g++ \
    libboost-dev \
    libboost-serialization-dev \
    libboost-regex-dev \
    libboost-iostreams-dev \
    libexpat1-dev \
    libbz2-dev \
    zlib1g-dev \
    libomp-dev

# Install certificates for git
RUN cd ${HOME} && \
  apt-get install --reinstall ca-certificates \
  mkdir /usr/local/share/ca-certificates/cacert.org \
  wget -P /usr/local/share/ca-certificates/cacert.org http://www.cacert.org/certs/root.crt http://www.cacert.org/certs/class3.crt \
  update-ca-certificates \
  git config --global http.sslCAinfo /etc/ssl/certs/ca-certificates.crt
