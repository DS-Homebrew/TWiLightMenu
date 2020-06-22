FROM devkitpro/devkitarm:20200528
# ENV TWLNOPATCHSRLHEADER=1
RUN \
  apt-get update && \
  apt-get install -y python && \
  rm -rf /var/lib/apt/lists/*
WORKDIR /data
