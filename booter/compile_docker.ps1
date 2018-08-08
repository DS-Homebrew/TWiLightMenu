docker -v
if (!$?) {
    Write-Error "Docker not installed.";
    Write-Output "Install Docker at https://www.docker.com/";
}

#check for dsimenuplusplus image
docker image inspect dsimenuplusplus >$null 2>&1 

if (!$?) {
    # build the image if it doesn't exist.
    docker build -t dsimenuplusplus --label dsimenuplusplus ../docker
}

docker run --rm -t -i -v "$pwd\:/data" dsimenuplusplus make @args

if($args.Count -eq 0 -and $?) {
    Copy-Item "booter.nds" "../7zfile/BOOT.NDS"
	Copy-Item "booter.nds" "../7zfile/CFW - SDNAND root/title/00030015/53524c41/content/00000000.app"
}