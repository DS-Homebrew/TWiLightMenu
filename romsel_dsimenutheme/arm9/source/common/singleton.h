/*
    common/singleton.h
    Copyright (c) 2018 chyyran

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#pragma once
#ifndef _SINGLETON_H_
#define _SINGLETON_H_
#include <cstdlib>
#include <utility>

template <typename T, typename... Args>
class singleton
{

  public:
    static inline T &instance(Args &&... args)
    {
        if (!_instance)
            make(std::forward<Args>(args)...);
        return *_instance;
    }

  private:
    static inline void make(Args... args)
    {
        if (!_instance)
            _instance = new T(std::forward<Args>(args)...);
    }

    static inline void reset()
    {
        if (_instance)
        {
            delete _instance;
            _instance = NULL;
        }
    }

  private:
    static T *_instance;
};

template<typename T,  typename...Args>
T * singleton<T, Args...>::_instance = NULL;

#endif //_SINGLETON_H_
