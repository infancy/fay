#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_APP_APP_H
#define FAY_APP_APP_H

#include "fay/app/window.h"
#include "fay/core/fay.h"
#include "fay/render/device.h"

namespace fay
{

struct app_desc
{
	window_desc window{};
    render_desc render{};
};

class app	// : protected app_desc
{
public:
	app(const app_desc& desc) : desc_{ desc }
	{
	}

	virtual ~app()
	{
	}

    virtual void setup()
    {
    }

    virtual void update()
    {
	}

    virtual void clear()
    {
    }

	// virtual void render()
	// virtual void close()

	virtual int run()
	{
		init_app(); // setup_app();

        setup();
		while (!window_->should_close())
		{
			// event
			window_->update();

			// update
			update();

			// show
			window_->show();
		}
        clear();

		return 0;
	}

private:
	void init_app()
	{
		// TODO: factory
		window_ = std::make_unique<window_glfw>(desc_.window);	// create window and context

        switch (desc_.render.render_backend_type)
        {
            case render_backend_type::opengl:
            case render_backend_type::opengl_dsa:

                desc_.render.glfw_window = window_->native_handle();
                break;

            case render_backend_type::d3d11:
            default:

                LOG(ERROR) << "error";
                break;
        }
        render = std::make_unique<render_device>(desc_.render);
	}

public:
    render_device_ptr render;

protected:
	app_desc desc_;
	std::unique_ptr<window_glfw> window_; // TODO: std::unique_ptr<window>
};

class app_manager
{

};

} // namespace fay

#endif // FAY_APP_APP_H