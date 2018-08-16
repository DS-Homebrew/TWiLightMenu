FROM quarktheawesome/devkitpro-arm-ppc:r47_r29
ADD libnds.tar /devkitpro
RUN \
  apt-get update && \
  apt-get install -y python && \
  rm -rf /var/lib/apt/lists/*
WORKDIR /data
