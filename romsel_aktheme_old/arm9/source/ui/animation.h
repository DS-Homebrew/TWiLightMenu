/*
    animation.h
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

#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include "ui/ui.h"

class Animation
{

  public:
    Animation();

    virtual ~Animation();

  public:
    virtual void update() = 0;

    void show();

    void hide();

    bool visible() { return _visible; }

  protected:
    akui::Point _position;
    akui::Size _size;
    bool _visible;
};

class AnimationManager
{

  public:
    AnimationManager();

    ~AnimationManager();

  public:
    void update();

    void addAnimation(Animation *animation);

    void removeAnimation(Animation *animation);

  protected:
    std::list<Animation *> _animations;
};

typedef singleton<AnimationManager> animationManager_s;
inline AnimationManager &animationManager() { return animationManager_s::instance(); }

#endif //_ANIMATION_H_