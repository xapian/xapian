/**
 Copyright (c) 2003, Technology Concepts & Design, Inc.
 Copyright (c) 2006, Olly Betts
 All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted 
provided that the following conditions are met:

    - Redistributions of source code must retain the above copyright notice, this list of conditions 
    and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright notice, this list of conditions 
    and the following disclaimer in the documentation and/or other materials provided with the distribution.

    - Neither the name of the Technology Concepts & Design, Inc. nor the names of its contributors may be used to 
    endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED 
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY 
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
THE POSSIBILITY OF SUCH DAMAGE.
**/

#ifndef __XAPIANOBJECTHOLDER_H__
#define __XAPIANOBJECTHOLDER_H__

#include <stdlib.h>
#include <pthread.h>
#include <string>
#include <typeinfo>

#if defined __GNUC__ && __GNUC__ >= 3
#include <ext/hash_map>
using __gnu_cxx::hash_map;
using __gnu_cxx::hash;
#elif defined _MSC_VER && _MSC_VER >= 1310
// 1310 is MSVC 2003, aka MSVC7.1.
#include <hash_map>
using stdext::hash_map;
using stdext::hash;
#else
#include <hash_map.h>
#endif

using namespace std;

// equality function for longs, used by our map of C++ objects
struct eq {
  bool operator()(long s1, long s2) const {
    return s1 == s2;
  }
};

template <class T> class XapianObjectHolder {
    private:
        pthread_mutex_t _lock;
        hash_map<long, T, hash<long>, eq> _holder;
        long _idcntr;
        
    public:
        XapianObjectHolder() {
            _idcntr = 0;
            pthread_mutex_init(&_lock, NULL);
        }
        ~XapianObjectHolder() {
            pthread_mutex_lock(&_lock);
            _holder.clear();
            pthread_mutex_unlock(&_lock);
            
            pthread_mutex_destroy(&_lock);
        }
        
        T get(long theid) {
            pthread_mutex_lock(&_lock);

            if (!_holder.count(theid)) {
                char *message = (char *) malloc(256);
                sprintf(message, "No such %s with id of %ld", typeid(T).name(), theid);
                
                pthread_mutex_unlock(&_lock);
                throw message;
            }
            
            T obj = _holder[theid];
            pthread_mutex_unlock(&_lock);
            return obj;
        }

        T remove(long theid) {
            T obj = 0;
            pthread_mutex_lock(&_lock);
            
            if (_holder.count(theid)) {
                obj = _holder[theid];
                _holder.erase(theid);
            }
            
            pthread_mutex_unlock(&_lock);
            return obj;
        }

        long put(T obj) {
            pthread_mutex_lock(&_lock);
            long theid = _idcntr++;
            _holder[theid] = obj;
            pthread_mutex_unlock(&_lock);
            return theid;
        }
};

#endif
