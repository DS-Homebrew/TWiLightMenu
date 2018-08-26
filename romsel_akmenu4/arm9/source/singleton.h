/*
    singleton.h
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#ifndef _SINGLETON_H_
#define _SINGLETON_H_
#include <cstdlib>

template <class T>
class t_singleton
{

  public:
    static T &instance()
    {
        if (NULL == _instance)
        {
            create_instance();
           
        }
        return *_instance;
    }

  private:
    static void create_instance()
    {
        if (NULL == _instance)
        {
            _instance = new T();
        }
    }

    static void release_instance()
    {
        if (NULL != _instance)
        {
            delete _instance;
            _instance = NULL;
        }
    }

  private:
    static T *_instance;
};

template <class T>
T *t_singleton<T>::_instance = NULL;
#endif //_AGL_SINGLETON_H_
