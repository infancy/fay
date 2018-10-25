#include "fay/app/app.h"

class clear : public fay::app
{
public:
	clear(const fay::app_desc& desc) : fay::app(desc)
	{
		desc_.window.title = "clear";
	}

	void update() override
	{
		// render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}
};

int main()
{
	fay::app_desc desc;
	{
		//
	}
	clear clear_v{ desc };
	return clear_v.run();
}