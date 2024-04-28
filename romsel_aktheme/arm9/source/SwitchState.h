/* 
 * File:   SwitchState.h
 * Author: matthew
 *
 * Created on November 16, 2015, 7:42 PM
 */
#pragma once

class SwitchState
{
	
	int state;

	SwitchState(int state, int size) : state(state), SIZE(size) { }
public:
	const int SIZE;

	SwitchState(int size) : state(0), SIZE(size) { }

	SwitchState(const SwitchState& orig) : state(orig.state), SIZE(orig.SIZE) { }

	SwitchState& operator++() {
		if (++state >= SIZE)
			state = 0;
		return *this;
	}

	SwitchState operator++(int) {
		SwitchState tmp(*this);
		if (++state >= SIZE)
			state = 0;
		return tmp;
	}

	operator int() const
	{
		return state;
	}

	~SwitchState() { }
};