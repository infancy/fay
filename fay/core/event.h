#pragma once

#include <functional>

namespace fay
{

// https://github.com/boostorg/signals2
// https://github.com/TheWisp/signals
// https://github.com/TheWisp/ImpossiblyFastEventCPP17
// https://github.com/larspensjo/SimpleSignal
// https://github.com/gongminmin/KlayGE/blob/develop/KlayGE/Core/Include/KlayGE/Signal.hpp

// https://doc.qt.io/qt-5/signalsandslots.html
// https://docs.unrealengine.com/en-US/Programming/UnrealArchitecture/Delegates/index.html
// https://docs.microsoft.com/en-us/dotnet/api/system.delegate



using event_id = std::size_t;

enum class event_type
{
    none,

#pragma region window_event

    on_window_open,

#pragma endregion window_event



#pragma region input_evnet


#pragma endregion input_evnet



#pragma region present_event

    on_present,

#pragma endregion present_event
};

template<event_type e, typename R, typename... Args>
class event
{
public:
    using cb_type = std::function<R(Args...)>;

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
    event_type event_;
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


// event_regist

inline event_id event_listen(event_type, std::function<void()> cb)
{
    // global event_handle
    static event_handle Artorias_;

}

inline void event_notify(event_type, std::function<void()> cb)
{
    // global event_handle
    static event_handle Artorias_;

}

inline void event_remove(event_id event)
{
    // global event_handle
    static event_handle Artorias_;

}

} // namespace fay