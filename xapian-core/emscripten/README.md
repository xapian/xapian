Building for Emscripten
=======================

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
