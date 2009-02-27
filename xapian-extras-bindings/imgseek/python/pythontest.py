#!/usr/bin/env python

import xapian

# Import xapian.imgseek.
try:
    import xapian.imgseek
except ImportError:
    # Hack to work when uninstalled - normally, this won't be required.
    import imgseek
    xapian.imgseek = imgseek
    del imgseek

db = xapian.inmemory_open()
a = xapian.imgseek.ImgSig()
try:
    a.unserialise('')
except xapian.NetworkError:
    pass
else:
    assert False

a.register_Image('/home/richard/richard.jpg')
print repr(a.serialise())
a.unserialise('O~\x7f\x06\x00\x00\x00\x0en\x10\x05-x\x02\x07\x8cJ\x06\x00')
