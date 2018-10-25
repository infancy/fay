#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_APP_INPUT_H
#define FAY_APP_INPUT_H

#include "fay/core/fay.h"

namespace fay
{

enum class cursor_mode
{
	normal,
	hidden,
	disabled
};

struct single_input
{
	// time
	double time, last, dt;

	// mouse
	double posx, posy;
	double lastx, lasty;
	double dx, dy, wheel;

	bool left, middle, right;

	// keyboard
	bool key_q, key_w;
};

inline single_input input_{};
inline const single_input& input{ input_ };

/*
inline single_input input2_{};
inline const single_input& input2{ input2_ };
*/

} // namespace fay

#endif // FAY_APP_INPUT_H