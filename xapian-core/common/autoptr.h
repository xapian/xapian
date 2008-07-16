/* autoptr.h: An auto pointer implementation
 * Nicked from gcc...
 */

// Copyright (C) 2001 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.

// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.

/*
 * Copyright (c) 1997-1999
 * Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */

#ifndef OM_HGUARD_AUTOPTR_H
#define OM_HGUARD_AUTOPTR_H

template<class _Tp1> struct AutoPtrRef {
   _Tp1* _M_ptr;
   AutoPtrRef(_Tp1* __p) : _M_ptr(__p) {}
};

template <class _Tp> class AutoPtr {
private:
  _Tp* _M_ptr;

public:
  typedef _Tp element_type;

  explicit AutoPtr(_Tp* __p = 0)  : _M_ptr(__p) {}
  AutoPtr(AutoPtr& __a)  : _M_ptr(__a.release()) {}

  template <class _Tp1> AutoPtr(AutoPtr<_Tp1>& __a) 
    : _M_ptr(__a.release()) {}

  AutoPtr& operator=(AutoPtr& __a)  {
    reset(__a.release());
    return *this;
  }

  template <class _Tp1>
  AutoPtr& operator=(AutoPtr<_Tp1>& __a)  {
    reset(__a.release());
    return *this;
  }
  
  // Note: The C++ standard says there is supposed to be an empty throw
  // specification here, but omitting it is standard conforming.  Its 
  // presence can be detected only if _Tp::~_Tp() throws, but (17.4.3.6/2)
  // this is prohibited.
  ~AutoPtr() { delete _M_ptr; }
 
  _Tp& operator*() const  {
    return *_M_ptr;
  }
  _Tp* operator->() const  {
    return _M_ptr;
  }
  _Tp* get() const  {
    return _M_ptr;
  }
  _Tp* release()  {
    _Tp* __tmp = _M_ptr;
    _M_ptr = 0;
    return __tmp;
  }
  void reset(_Tp* __p = 0)  {
    if (__p != _M_ptr) {
      delete _M_ptr;
      _M_ptr = __p;
    }    
  }

  // According to the C++ standard, these conversions are required.  Most
  // present-day compilers, however, do not enforce that requirement---and, 
  // in fact, most present-day compilers do not support the language 
  // features that these conversions rely on.
public:
  AutoPtr(AutoPtrRef<_Tp> __ref) 
    : _M_ptr(__ref._M_ptr) {}

  AutoPtr& operator=(AutoPtrRef<_Tp> __ref)  {
    if (__ref._M_ptr != this->get()) {
      delete _M_ptr;
      _M_ptr = __ref._M_ptr;
    }
    return *this;
  }

  template <class _Tp1> operator AutoPtrRef<_Tp1>()  
    { return AutoPtrRef<_Tp>(this->release()); }
  template <class _Tp1> operator AutoPtr<_Tp1>() 
    { return AutoPtr<_Tp1>(this->release()); }
};

#endif /* OM_HGUARD_AUTOPTR_H */
