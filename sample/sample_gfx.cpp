#include "fay/app/app_flexible.h"
#include "fay/app/scene.h"

class gfx : public fay::app
{
public:
    gfx(const fay::app_desc& _desc) : fay::app(_desc)
    {
        desc.window.title = "gfx";

        //scene_ = std::make_unique<fay::scene>(device.get());
    }

    void setup() override
    {
        // TODO: remove it
        add_update_items_
        (
            { cameras_, cameras_ + 1 },
            { lights_, lights_ + 1 },
            { transforms_, transforms_ + 1 }
        );

        scene_ = std::make_unique<fay::scene>(device.get());
        gfx_ = scene_->graphics_scene_proxy();

        std::string name;
        //std::cin >> name;

        //scene_->add_model("object/Rei/Rei.obj");// + name);
        scene_->add_model(fay::Box);

        fay::shader_desc sd = fay::scan_shader_program("renderable", "gfx/renderable.vs", "gfx/renderable.fs");
        sd.name = "shd"; //todo
        auto shd_id = device->create(sd);

        fay::pipeline_desc pd;
        {
            pd.name = "triangles";
            pd.primitive_type = fay::primitive_type::triangles;
            pd.face_winding = fay::face_winding::ccw;
            pd.cull_mode = fay::cull_mode::none;
            pd.depth_compare_op = fay::compare_op::less;
        }
        auto pipe_id = device->create(pd);

        paras.a = { 1.f, 0.f, 0.f, 1.f };
        paras.b = { 0.f, 1.f, 0.f, 1.f };

        pass1
            .begin_default_frame()
            .clear_frame()
            .apply_shader(shd_id)
            .bind_uniform_block("color", fay::memory{ (uint8_t*)&paras, sizeof(fay::render_paras) })
            .apply_pipeline(pipe_id);
    }

    void render() override
    {
        // draw
        MVP = camera.world_to_ndc() * transform.model_matrix();

        auto cmds = pass1;
        cmds.bind_uniform("MVP", MVP);

        /*
        // submit, 
    void flush_to(command_list& cmd)
    {
        for (auto& mesh : render_list)
            mesh->device(cmd);
    }
        */
        for (auto& comps : *gfx_.renderables)
            std::get<1>(comps)->renderable->render(cmds);

        cmds.end_frame();

        device->submit(cmds);
        device->execute();
    }

    // TODO: add to scene
    fay::camera cameras_[2]
    {
        fay::camera{ glm::vec3{  0, 20, 50 } },
        fay::camera{ glm::vec3{ 100, 0, 50 } },
    };
    fay::light lights_[2]
    {
        fay::light{ glm::vec3{  0, 20, 50 } },
        fay::light{ glm::vec3{ 100, 0, 50 } },
    };
    fay::transform transforms_[2]
    {
        fay::transform{ glm::vec3{  0, 20, 50 } },
        fay::transform{ glm::vec3{ 100, 0, 50 } },
    };
    const fay::camera& camera{ cameras_[0] };
    const fay::light& light{ lights_[0] };
    const fay::transform& transform{ transforms_[0] };


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