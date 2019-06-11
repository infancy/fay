#include "fay/app/app_flexible.h"
#include "fay/app/scene.h"

class gfx : public fay::app
{
public:
    gfx(const fay::app_desc& _desc) : fay::app(_desc)
    {
        desc.window.width = 1080;
        desc.window.height = 720;
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
        scene_->add_model(fay::Sponza);

        fay::shader_desc sd = fay::scan_shader_program("renderable", "gfx/renderable.vs", "gfx/renderable.fs", fay::render_backend_type::opengl);
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

        {
            fay::shader_desc sd = fay::scan_shader_program("light_shd", "gfx/lines.vs", "gfx/lines.fs", desc.render.backend);
            debug_shd = device->create(sd);

            fay::pipeline_desc pd;
            pd.name = "light_pipe";
            pd.stencil_enabled = false;
            pd.primitive_type = fay::primitive_type::lines;
            debug_pipe = device->create(pd);
        }

        paras.a = { 1.f, 0.f, 0.f, 1.f };
        paras.b = { 0.f, 1.f, 0.f, 1.f };

        pass1
            .begin_default(pipe_id, shd_id)
            .bind_uniform_block("color", fay::memory{ (uint8_t*)&paras, sizeof(fay::render_paras) });
    }

    void render_node(fay::node_sp root_node, fay::frustum box_camera, fay::command_list& cmds)
    {
        for (auto node : root_node->childrens())
        {
            auto e = node->get_entity();

            auto bounds = e->get_component<fay::bounds3_component>()->bounds;

            if (!(bounds.is_vaild() && box_camera.expect(bounds)))
            {
                auto rcomp = e->get_component<fay::renderable_component>();

                if (rcomp)
                    rcomp->renderable->render(cmds);
            }

            render_node(node, box_camera, cmds);
        }
    }

    void render() override
    {
        // draw
        MVP = camera.world_to_ndc();// *transform.model_matrix();

        auto cmds = pass1;
        cmds.bind_uniform("MVP", MVP);

        fay::frustum box_camera(camera.world_to_ndc());

    //#define FAY_USEOOP
    #ifdef FAY_USEOOP
        render_node(scene_->root(), box_camera, cmds);
    #else
        for (auto& comps : *gfx_.renderables)
        {
            auto bounds = std::get<fay::bounds3_component*>(comps)->bounds;

            if (!(bounds.is_vaild() && box_camera.expect(bounds)))
                std::get<fay::renderable_component*>(comps)->renderable->render(cmds);
        }
    #endif // FAY_USEOOP

        /*
        cmds
            .apply_pipeline(debug_pipe)
            .apply_shader(debug_shd)
            .bind_uniform("MVP", camera.world_to_ndc());

        bool flag{ true };
        for (auto& comps : *gfx_.renderables)
        {
            auto bounds = std::get<fay::bounds3_component*>(comps)->bounds;

            auto debug_bounds = create_box_mesh(bounds, device.get());

            if(flag = !flag; flag)
                debug_bounds->render(cmds);
        }
        */


        cmds.end_frame();

        device->submit(cmds);
        device->execute();
    }

    // TODO: add to scene
    fay::camera cameras_[2]
    {
        fay::camera{ glm::vec3{   0, 0, -10 } },
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

    fay::shader_id debug_shd;
    fay::pipeline_id debug_pipe;

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