docker -v
if (!$?) {
    Write-Error "Docker not installed.";
    Write-Output "Install Docker at https://www.docker.com/";
}

#check for dsimenuplusplus image
docker image inspect twilightmenu >$null 2>&1 

if (!$?) {
    # build the image if it doesn't exist.
    docker build -t twilightmenu --label twilightmenu ../docker
}

docker run --rm -t -i -v "$pwd\:/data" twilightmenu make @args

if($args.Count -eq 0 -and $?) {
    Copy-Item "settings.nds" "../7zfile/_nds/TWiLightMenu/settings.srldr"
}