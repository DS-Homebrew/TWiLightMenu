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

docker run -t -i -v "$pwd\:/data" dsimenuplusplus make @args
Copy-Item "romsel_dsimenutheme.nds" "../7zfile/_nds/dsimenuplusplus/dsimenu.srldr"
