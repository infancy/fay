#include "fay/app/app_flexible.h"
#include "fay/app/scene.h"

// model
const std::string Nier_2b = "object/Nier_2b/2b.obj";
const std::string Box = "object/box/box.obj";
const std::string Blocks = "object/blocks/blocks.obj";
const std::string Rei = "object/Rei/Rei.obj";
const std::string CornellBox = "object/CornellBox/CornellBox.obj";
const std::string Planet = "object/planet/planet.obj";
const std::string Rock = "object/rock/rock.obj";
const std::string Fairy = "object/fairy/fairy.obj";
const std::string Nanosuit = "object/nanosuit/nanosuit.obj";
const std::string silly_dancing = "object/silly_dancing.fbx";
const std::string nierautomata_2b = "object/nierautomata_2b/scene.gltf";
const std::string ftm_sketchfab = "object/ftm/ftm_sketchfab.blend";
const std::string Nier_2b_ik_rigged = "object/Nier_2b_ik_rigged/scene.gltf";
const std::string Sponza = "object/sponza/sponza.obj";

const unsigned int Width = 1080;
const unsigned int Height = 720;

// timing
float currentFrame = 0.f;
float deltaTime = 0.f;
float lastFrame = 0.f;

// model
glm::vec3 model_scale(1.f);

// camera_
fay::camera camera_(glm::vec3(0, 0, 10));;
float lastX = Width / 2.0f;
float lastY = Height / 2.0f;
bool firstMouse = true;

//light 
glm::vec3 lightPosition = glm::vec3(0, 10, 0); //objectspace light position
float light_speed = 2.f;
glm::vec3 light_scale(0.5f, 0.5f, 0.5f);

// some flag
bool some_flag = false;
char mouse_move = 'z';
int render_state = 1;

struct render_paras
{
    glm::vec4 a{};
    glm::vec4 b{};
};

void update_io(const fay::single_input& io = fay::input)
{
    currentFrame = glfwGetTime();
    deltaTime = (currentFrame - lastFrame) * 4;
    lastFrame = currentFrame;
    
    float xpos = io.posx, ypos = io.posy;
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = xpos - lastX; lastX = xpos;
    // reversed since y-coordinates go from bottom to top but z_xais form out to in
    float yoffset = lastY - ypos; lastY = ypos;
    
    // TODO: io[z], io.x
    if (io.key[' ']) mouse_move = ++mouse_move % 3;
    if (io.key['z']) mouse_move = 'z';
    if (io.key['x']) mouse_move = 'x';
    if (io.key['c']) mouse_move = 'c';

    if (mouse_move == 'z')
    {
        if (model_scale.x <= 1.f)
            model_scale -= glm::vec3(0.1f, 0.1f, 0.1f) * glm::vec3(io.wheel);
        else
            model_scale -= glm::vec3(1.f, 1.f, 1.f) * glm::vec3(io.wheel);

        if (model_scale.x < 0.f)
            model_scale = glm::vec3(0.1f, 0.1f, 0.1f);
        else if (model_scale.x > 10.f)
            model_scale = glm::vec3(10.f, 10.f, 10.f);

        camera_.ProcessMouseMovement(xoffset, yoffset);
        if (io.key['w']) camera_.ProcessKeyboard(fay::FORWARD, deltaTime);
        if (io.key['s']) camera_.ProcessKeyboard(fay::BACKWARD, deltaTime);
        if (io.key['a']) camera_.ProcessKeyboard(fay::LEFT, deltaTime);
        if (io.key['d']) camera_.ProcessKeyboard(fay::RIGHT, deltaTime);
        //camera_.ProcessMouseScroll(io.wheel); 
    }
    else if (mouse_move == 'x')
    {
        
        light_scale -= glm::vec3(0.1f, 0.1f, 0.1f) * glm::vec3(io.wheel);
        if (light_scale.x < 0.f)
            light_scale = glm::vec3(0.1f, 0.1f, 0.1f);
        else if (light_scale.x > 1.f)
            light_scale = glm::vec3(1.f, 1.f, 1.f);

        light_speed -= io.wheel;
        if (light_speed <= 0.f)
            light_speed = 0.f;
        else if (light_speed >= 10.f)
            light_speed = 10.f;

        if (io.key['w']) lightPosition.z -= deltaTime * light_speed;
        if (io.key['s']) lightPosition.z += deltaTime * light_speed;
        if (io.key['a']) lightPosition.x -= deltaTime * light_speed;
        if (io.key['d']) lightPosition.x += deltaTime * light_speed;
        if (io.left_down) lightPosition.y += deltaTime * light_speed;
        if (io.right_down) lightPosition.y -= deltaTime * light_speed;

        // if (io.MouseDown[2]) clear_color = ImColor(255, 255, 255);
    }
    else if (mouse_move == 'c')
    {
        // GUI
    }
    else
    {
        LOG(ERROR) << "shouldn't be here";
    }
}

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
        std::cin >> name;

        scene_->add_model("object/" + name);

        fay::shader_desc sd = fay::scan_shader_program("gfx/model.vs", "gfx/model.fs", false);
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
            .bind_uniform_block("color", fay::memory{ (uint8_t*)&paras, sizeof(render_paras) })
            .apply_pipeline(pipe_id);
    }

    void update() override
    {
        update_io();
        glm::mat4 view = camera_.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera_.Zoom),
            (float)Width / (float)Height, 0.1f, 10000.0f);

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

    fay::scene_ptr scene_;
    fay::graphics_scene gfx_;

    fay::buffer_id buf_id;

    render_paras paras;
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