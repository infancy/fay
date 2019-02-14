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
    double this_time, last_time, delta_time;

	// mouse
    double x, y;
	double lastx, lasty;
	double dx, dy, wheel; // mouse move

	bool left_down, middle_down, right_down; // mouse down

	// keyboard
    std::array<bool, 256> key; // {};
	bool key_q, key_w;
};

// TODO: remove it
inline single_input input_{}; // only be modified by the window
const inline single_input& input{ input_ };

/*
inline single_input input2_{};
inline const single_input& input2{ input2_ };
*/

} // namespace fay

#endif // FAY_APP_INPUT_H