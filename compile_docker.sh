#!/bin/bash
docker -v
if [ $? -ne 0 ]; then
    echo "Docker not installed." >&2
    echo "Install Docker at https://www.docker.com/"
fi

#check for TWiLightMenu image
docker image inspect twilightmenu >/dev/null 2>&1 

if [ $? -ne 0 ]; then
    # build the image if it doesn't exist.
    docker build -t twilightmenu --label twilightmenu ./docker
fi

docker run --rm -t -i -v "$(pwd)\:/data" twilightmenu make $@