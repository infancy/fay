#include "fay/app/app_flexible.h"
#include "fay/app/scene.h"

class gfx : public fay::app
{
public:
    gfx(const fay::app_desc& desc) : fay::app(desc)
    {
        desc_.window.title = "gfx";

        //scene_ = std::make_unique<fay::scene>(render.get());
    }

    void setup() override
    {
        scene_ = std::make_unique<fay::scene>(render.get());
        gfx_ = scene_->graphics_scene_proxy();

        std::string name;
        //std::cin >> name;

        //scene_->add_model("object/Rei/Rei.obj");// + name);
        scene_->add_model(fay::nierautomata_2b);

        fay::shader_desc sd = fay::scan_shader_program("gfx/renderable.vs", "gfx/renderable.fs", false);
        sd.name = "shd"; //todo
        auto shd_id = render->create(sd);

        fay::pipeline_desc pd;
        {
            pd.name = "triangles";
            pd.primitive_type = fay::primitive_type::triangles;
            pd.face_winding = fay::face_winding::ccw;
            pd.cull_mode = fay::cull_mode::none;
            pd.depth_compare_op = fay::compare_op::less;
        }
        auto pipe_id = render->create(pd);

        paras.a = { 1.f, 0.f, 0.f, 1.f };
        paras.b = { 0.f, 1.f, 0.f, 1.f };

        pass1
            .begin_default_frame()
            .clear_frame()
            .apply_shader(shd_id)
            .bind_uniform_block("color", fay::memory{ (uint8_t*)&paras, sizeof(fay::render_paras) })
            .apply_pipeline(pipe_id);
    }

    void update() override
    {
        misc.update_io();
        glm::mat4 view = misc.camera_.view_matrix();
        glm::mat4 projection = glm::perspective(glm::radians(misc.camera_.Zoom),
            (float)misc.Width / (float)misc.Height, 0.1f, 10000.0f);

        // draw
        glm::mat4 model(1.f);
        MVP = projection * view * model;

        auto cmds = pass1;
        cmds.bind_uniform("MVP", MVP);

        /*
        // submit, 
    void flush_to(command_list& cmd)
    {
        for (auto& mesh : render_list)
            mesh->render(cmd);
    }
        */
        for (auto& comps : *gfx_.renderables)
            std::get<1>(comps)->renderable->render(cmds);

        cmds.end_frame();

        render->submit(cmds);
        render->execute();
    }

    fay::render_data misc;

    fay::scene_ptr scene_;
    fay::graphics_scene gfx_;

    fay::buffer_id buf_id;

    fay::render_paras paras;
    glm::mat4 MVP{ 1.f };
    fay::command_list pass1, pass2;
};

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
    gfx app_{ desc };
    return app_.run();
}