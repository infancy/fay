#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_APP_APP_H
#define FAY_APP_APP_H

#include "fay/app/input.h"
#include "fay/app/window.h"
#include "fay/core/fay.h"
#include "fay/gfx/camera.h"
#include "fay/gfx/light.h"
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
	app(const app_desc& _desc) : desc{ _desc }
	{
	}

	virtual ~app()
	{
	}

    virtual void setup()
    {
    }


    virtual void update(const single_input& io)
    {
	}

    virtual void render()
    {
    }

    virtual void clear()
    {
    }

	// virtual void render()
	// virtual void close()

    void add_update_items_(
        std::vector<camera*> cameras,
        std::vector<light*> lights,
        std::vector<transform*> transforms)
    {
        cameras_ = cameras;
        lights_ = lights;
        transforms_ = transforms;
    }

	virtual int run()
	{
		init_app(); // setup_app();

        setup();
		while (!window->should_close())
		{
			// event
			window->update();

            // TODO: event system
            update_items();

			// update
            // TODO: update(window->input());
			update(input);

            // physical...

            // render
            // device->render();
            render();

			// show
			window->show();
		}
        clear();

		return 0;
	}

private:
	void init_app()
	{
		// TODO: factory
		window = std::make_unique<window_glfw>(desc.window, desc.render);	// create window and context
        device = std::make_unique<render_device>(desc.render);
	}

    void update_items(const single_input& io = input)
    {
        static size_t current_mode_{};
        static size_t current_items_[3]{};

        // TODO: io['z'], io.x
        if (io[' ']) current_mode_ = ++current_mode_ % 3;
        if (io['z']) current_mode_ = 0;
        if (io['x']) current_mode_ = 1;
        if (io['c']) current_mode_ = 2;

        //if (io['0']) current_item_ = 0;
        if (io['1']) current_items_[current_mode_] = 0;
        if (io['2']) current_items_[current_mode_] = 1;
        if (io['3']) current_items_[current_mode_] = 2;
        if (io['4']) current_items_[current_mode_] = 3;

        auto current_item_ = current_items_[current_mode_];

        if (current_mode_ == 0)
        {
            // camera
            if (cameras_.size() > current_item_ && cameras_[current_item_])
                cameras_[current_item_]->on_input_event(io);
        }
        else if (current_mode_ == 1)
        {
            // light
            if(lights_.size() > current_item_ && lights_[current_item_])
                lights_[current_item_]->on_input_event(io);
        }
        else if (current_mode_ == 2)
        {
            // model
            if (transforms_.size() > current_item_ && transforms_[current_item_])
            transforms_[current_item_]->on_input_event(io);
        }
        else if (current_mode_ == 3)
        {
            // GUI
        }
        else
        {
            LOG(ERROR) << "shouldn't be here";
        }
    }

protected:
	app_desc desc;
	std::unique_ptr<window_glfw> window; // TODO: std::unique_ptr<window>
    render_device_ptr device;

private:
    // TODO: event system
    std::vector<camera*> cameras_;
    std::vector<light*> lights_;
    std::vector<transform*> transforms_;
};

class app_manager
{

};

} // namespace fay

#endif // FAY_APP_APP_H