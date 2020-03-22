#pragma once

// #include <fay/core/fay.h>
#include "fay/core/event.h"

namespace fay
{

class object
{
public:


    bool is_active();

protected:
    //! all listened events are cleared when the object is destroyed
    void listen_event(event_type event, std::function<void()> react);

private:
    // event_recorder recorder_;
};

} // namespace fay
