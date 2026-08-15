docker := x"podman run -it --rm -v $PWD:/minercat3ds:z  -w /minercat3ds devkitpro/devkitarm@sha256:116afba8df8453961de2936ffab20dd441edf4d682856c1ec8b0e53d7ed0bbf5"
   
build:
    {{ docker }} make

shell:
    {{ docker }} bash

copy_includes:
    rm -rf include
    mkdir include
    {{ docker }} sh -c 'cp -r /opt/devkitpro/libctru/include/* /opt/devkitpro/devkitARM/arm-none-eabi/include/* include/'

clean:
    {{ docker }} make clean

emulate:
    just b
    azahar minercat3ds.3dsx

install:
    just b
    # make the directory manually first
    curl -T minercat3ds.3dsx ftp://192.168.1.111:5000/3ds/minercat3ds/minercat3ds.3dsx

alias b := build
alias c := clean
alias e := emulate
