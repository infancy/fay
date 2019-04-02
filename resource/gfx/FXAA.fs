//#define SHADER_TOY

#ifdef SHADER_TOY
#else
    #version 330 core
    in vec2 vTex;
    out vec4 FragColor;

    layout (std140) uniform color
    {
        vec4 a;
        vec4 b;
    };

    uniform sampler2D frame;
#endif

//@
float rgb2luma(vec3 rgb) 
{
    return rgb.g; // 使用官方版本
    // return dot(rgb, vec3(0.2126, 0.7152, 0.0722)); // 最流行的亮度计算
    // return dot(rgb, vec3(0.299, 0.587, 0.114)); // 曾经最流行的方法
    // return sqrt(0.299 * rgb.r * rgb.r + 0.587 * rgb.g * rgb.g + 0.114 * rgb.b * rgb.b); // 更精确的计算
    // return sqrt(dot(rgb, vec3(0.299, 0.587, 0.114))); // 添加了 gamma 校正的计算
}

float rgb2luma_(vec2 uv, int offfset_x, int offset_y) 
{
    return rgb2luma(textureOffset(frame, uv, ivec2(offfset_x, offset_y)).rgb);
}

mat3 around_luma(vec2 uv)
{
    // TODO

    /*
         ------------->
        |
        |    00 01 02
        |
        |    10 11 12
        |
        |    20 21 22
        v
    */
    mat3 luma;

    luma[0][0] = rgb2luma_(uv, -1, -1);
    luma[0][1] = rgb2luma_(uv,  0, -1);
    luma[0][2] = rgb2luma_(uv,  1, -1);

    luma[1][0] = rgb2luma_(uv, -1, 0);
    luma[1][1] = rgb2luma_(uv,  0, 0);
    luma[1][2] = rgb2luma_(uv,  1, 0);

    luma[2][0] = rgb2luma_(uv, -1,  1);
    luma[2][1] = rgb2luma_(uv,  0,  1);
    luma[2][2] = rgb2luma_(uv,  1,  1);

    return luma;
}

vec3 max_min_diff_luma(mat3 luma)
{
    //float lumaMin = min(lumaCenter,min(min(lumaDown,lumaUp),min(lumaLeft,lumaRight)));
    //float lumaMax = max(lumaCenter,max(max(lumaDown,lumaUp),max(lumaLeft,lumaRight)));

    float lumaMin = min(luma[1][1], min( min(luma[2][1], luma[0][1]), min(luma[1][0], luma[1][2]) ));
    float lumaMax = max(luma[1][1], max( max(luma[2][1], luma[0][1]), max(luma[1][0], luma[1][2]) ));

    float diff = lumaMax - lumaMin;

    return vec3(lumaMax, lumaMin, diff);
}

bool main_direction_is_horizontal(mat3 luma)
{
    // 水平垂直方向
    float lumaDownUp    = luma[0][1] + luma[2][1];
    float lumaLeftRight = luma[1][0] + luma[1][2];

    // 4个角
    float lumaLeftCorners  = luma[2][0] + luma[0][0];
    float lumaDownCorners  = luma[2][0] + luma[2][2];

    float lumaRightCorners = luma[0][2] + luma[2][2];
    float lumaUpCorners    = luma[0][2] + luma[0][0];

    // 水平垂直方向计算差值
    float edgeHorizontal = 
        abs(-2.0 * luma[1][0] + lumaLeftCorners)  + 
        abs(-2.0 * luma[1][2] + lumaRightCorners) + 
        abs(-2.0 * luma[1][1] + lumaDownUp )      * 2.0;
        
    float edgeVertical  = 
        abs(-2.0 * luma[0][1] + lumaUpCorners)   + 
        abs(-2.0 * luma[2][1] + lumaDownCorners) +
        abs(-2.0 * luma[1][1] + lumaLeftRight)   * 2.0;

    return edgeHorizontal > edgeVertical;
}

// 阈值与最大亮度系数
const float EDGE_THRESHOLD_MAX = 0.166;
// 最低判断阈值
const float EDGE_THRESHOLD_MIN = 0.0833;

//#define ONE_SHOW_EDGE
#define TWO_SHOW_MAIN_DIR

#ifdef SHADER_TOY
void mainImage(out vec4 FragColor, in vec2 vTex)
#else
void main()
#endif
{    
    vec2 currrentUV = vTex;

    mat3 luma = around_luma(currrentUV);

    // 边缘检测
    vec3 mmd = max_min_diff_luma(luma);

    if(mmd.z < max(EDGE_THRESHOLD_MIN, EDGE_THRESHOLD_MAX * mmd.x))
    {
        FragColor = texture(frame, vTex);
        return;
    }
#ifdef ONE_SHOW_EDGE
    else
    {
        FragColor = vec4(1, 0, 0, 1);
        return;
    }
#endif

    // 判定方向（横纵）
    bool isHorizontal = main_direction_is_horizontal(luma);

#ifdef TWO_SHOW_MAIN_DIR
    if (isHorizontal)
    {
        FragColor = vec4(1, 0, 0, 1);
    }
    else
    {
        FragColor = vec4(1, 1, 1, 1);
    }
    return;
#endif


	//FragColor = texture(frame, vTex);
}