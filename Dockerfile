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

FROM ubuntu:24.04
LABEL authors="nicolasvontrott"

ENV HOME=/root
ENV TZ=Europe/Berlin
ENV DEBIAN_FRONTEND=noninteractive
SHELL ["/bin/bash", "-c"]

RUN apt-get update && apt-get -y --no-install-recommends install \
# Uncomment the following line if you want to add support for bounding boxes or polygon files
#    osmium-tool \
    ca-certificates \
    build-essential \
    ninja-build \
    cmake \
    git \
    libcurl4-openssl-dev \
    libomp-dev \
    libosmium2-dev

COPY . /app/
WORKDIR /app/build/
RUN cmake -DCMAKE_BUILD_TYPE=Release  \
          -DUSE_PARALLEL=true  \
          -GNinja .. && ninja

ENTRYPOINT ["/app/build/apps/olu"]