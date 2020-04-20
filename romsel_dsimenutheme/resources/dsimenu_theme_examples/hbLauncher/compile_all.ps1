mkdir out
Get-ChildItem | foreach { pushd $_; make clean; make; popd; cp -Recurse -Force $_ out; }
cd out
Get-ChildItem | foreach { pushd $_; rm -Recurse background_grit; rm -Recurse grit; rm Makefile; popd; }
