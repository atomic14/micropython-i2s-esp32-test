[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/Z8Z734F5Y)

# MicroPython and I2S Audio on the ESP32

A quick demo of this [PR]() that adds support for I2S to MicroPython.

You can watch a video of this in action [here](https://youtu.be/UXt27kOokh0)

[![Demo Video](https://img.youtube.com/vi/UXt27kOokh0/0.jpg)](https://www.youtube.com/watch?v=UXt27kOokh0)

I2S is not yet officially supported, but there is a Pull Request on GitHub that adds it in.

We can build a custom version of Micropython pretty easily. We just need the ESP-IDF from here: https://github.com/espressif/esp-idf.

And then we can get a copy of Micropython from here: https://github.com/micropython/micropython and merge in the pull request with I2S support: https://github.com/micropython/micropython/pull/7183

The full commands to set up the IDF are:

```
git clone -b v4.2 --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh
. export.sh
```

And then to build our custom version of Micropython:

```
git clone git@github.com:micropython/micropython.git
cd micropython
git fetch origin pull/7183/head:i2s_support
git merge i2s_support
make -C mpy-cross
cd ports/esp32
make submodules
make BOARD=UM_TINYPICO
```

You can find a bunch of sample code here for the I2S pull request:
https://github.com/miketeachman/micropython-i2s-examples
