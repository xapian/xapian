Building for Emscripten
=======================

Emscripten < 1.39.10 has a bug in its `O_TRUNC` emulation which affects Xapian.
We have a workaround in place for the use of `O_TRUNC` which is known to
trigger this bug, but there are other uses of `O_TRUNC` in the code so using
Emscripten >= 1.39.10 is recommended.

Emscripten support has been tested with glass backend.

Instructions below was tested on ubuntu 16.04, after doing a regular platform build.

Build xapian (from `xapian-core` folder):
```
emconfigure ./configure CPPFLAGS='-DFLINTLOCK_USE_FLOCK' CXXFLAGS='-Oz -s USE_ZLIB=1 -fno-rtti' --disable-backend-honey --disable-backend-inmemory --disable-shared --disable-backend-remote
emmake make
```

Change directory to the emscripten folder

`cd emscripten`

Test compiling a webassembly binary (from source `xapianjstest.cc`):

``
XAPIAN=.. && em++ -Oz -s USE_ZLIB=1 -std=c++11 -s WASM=1 -I$XAPIAN/include xapianjstest.cc $XAPIAN/.libs/libxapian-1.5.a -o xapianjstest.js
``

and then you can run it using nodejs:

`node xapianjstest.js`
