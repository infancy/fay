#pragma once

#define SAMPLE_RENDER_APP_DECL( class_name ) \
class class_name; \
std::unique_ptr<fay::app> create_##class_name##_app(const fay::app_desc& desc);

#define SAMPLE_RENDER_APP_IMPL( class_name ) \
std::unique_ptr<fay::app> create_##class_name##_app(const fay::app_desc& desc) \
{ \
    return std::make_unique<class_name>(desc); \
}

/*
#define SAMPLE_RENDER_APP( class_name )
#ifdef SAMPLE_RENDER_MAIN
    SAMPLE_RENDER_APP_DECL(class_name)
#else
    SAMPLE_RENDER_APP_IMPL(class_name)
#endif
*/

#define CREATE_SAMPLE_RENDER_APP( class_name, desc ) create_##class_name##_app(desc)
