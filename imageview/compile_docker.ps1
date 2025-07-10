docker -v
if (!$?) {
    Write-Error "Docker not installed.";
    Write-Output "Install Docker at https://www.docker.com/";
}

#check for dsimenuplusplus image
docker image inspect TWiLightMenu >$null 2>&1 

if (!$?) {
    # build the image if it doesn't exist.
    docker build -t TWiLightMenu --label TWiLightMenu ../
}

docker run --rm -t -i -v "$pwd\:/data" TWiLightMenu make @args

if ($args.Count -eq 0 -and $?) {
    Copy-Item "imageview.nds" "../7zfile/Multimedia/_nds/TWiLightMenu/imageview.srldr"
}