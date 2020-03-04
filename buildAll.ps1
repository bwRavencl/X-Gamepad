$targets = @("x86_64-w64-mingw32", "x86_64-apple-darwin", "x86_64-linux-gnu")

$targets | ForEach-Object {
    docker run -it --rm -v ${pwd}:/workdir -e CROSS_TRIPLE=$_ ${docker build -q https://github.com/bwRavencl/crossbuild.git#fix-line-endings} ./build.sh
}