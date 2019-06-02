#pragma once

#include "fay/core/fay.h"

namespace fay
{

// https://github.com/boost-experimental/te
// https://github.com/facebook/folly/blob/master/folly/docs/Poly.md
// https://github.com/ldionne/dyno

enum class event_type
{
    none,

    // window_event
    on_window_open,

    // input_evnet

    // present_event
    on_present,
};

template<event_type e, typename R, typename... Ts>
class event
{
public:
    using cb_type = R(Ts...);

public:

    void attach()
    {

    }

    void notify()
    {

    }

    void remove()
    {

    }

private:
    event_type e_;
    std::vector<cb_type> queue_;
};

// event_manager, register
// event_system

class event_handle
{
public:

    event_handle& listen()
    {

    }

    event_handle& silent()
    {

    }

private:
    struct connection
    {
        event_type event;
        void* callback;
    };

private:
    std::vector<connection> events_;
};



inline void listen()
{
    static event_handle Artorias_;

}

} // namespace fay