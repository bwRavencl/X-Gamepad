$targets = @("x86_64-w64-mingw32", "x86_64-apple-darwin", "x86_64-linux-gnu")

$targets | ForEach-Object {
    docker run -it --rm -v ${pwd}:/workdir -e CROSS_TRIPLE=$_ ${docker build -q https://github.com/multiarch/crossbuild.git#529881986478b18c9973b8979fa6287d44a77c8f} ./build.sh
}