// #define SAMPLE_RENDER_MAIN

#include "fay/app/app.h"
#include "fay/core/fay.h"

#include "sample_render_app.h"

// shadow, defer_render

// extern class shadow shadow_app;

SAMPLE_RENDER_APP_DECL(clear)
SAMPLE_RENDER_APP_DECL(triangle)
SAMPLE_RENDER_APP_DECL(offscreen)
SAMPLE_RENDER_APP_DECL(shadow_map)
SAMPLE_RENDER_APP_DECL(defer_rendering)

SAMPLE_RENDER_APP_DECL(PBR)

int main(int argc, char** argv)
{
    google::InitGoogleLogging(argv[0]);
    //--stderrthreshold=0 --logtostderr=true
    //FLAGS_logtostderr = true;
    //FLAGS_stderrthreshold = 0;
    //FLAGS_v = 2;

    // vector<{XX_desc, XX}> ...;
    fay::app_desc desc;
    {
        //
    }
    auto  app_ = CREATE_SAMPLE_RENDER_APP(PBR, desc);
    return app_->run();
}