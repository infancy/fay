#pragma once

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

	// mouse position
    double x, y;
	double lastx, lasty;
    double dx, dy;

    // mouse button
    double wheel;
	bool left_down, middle_down, right_down; // mouse down

	// keyboard
    std::array<bool, 256> key; // {};

    bool operator[](char ch) const
    {
        return key[ch];
    }
};

// TODO: remove it
inline single_input input_{}; // only be modified by the window
const inline single_input& input{ input_ };

/*
inline single_input input2_{};
inline const single_input& input2{ input2_ };
*/

} // namespace fay