```console
mkdir build
cd build
cmake ..
make
```
flash firmware
```console
cd bin
cycfx2prog prg:led.ihx run
```