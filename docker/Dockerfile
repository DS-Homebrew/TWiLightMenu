FROM devkitpro/devkitarm:20190212
ADD libnds.tar /opt/devkitpro
# ENV TWLNOPATCHSRLHEADER=1
RUN \
  apt-get update && \
  apt-get install -y python && \
  rm -rf /var/lib/apt/lists/* 
  # git clone https://github.com/RocketRobz/libnds /temp/libnds && \
  # cd /temp/libnds && \
  # make install
WORKDIR /data
