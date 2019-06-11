
    #version 330 core
    in vec2 vTex;
    out vec4 FragColor;

    layout (std140) uniform color
    {
        vec4 a;
        vec4 b;
    };

    uniform sampler2D frame;

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

const int ITERATION = 12;
const float steps[ITERATION] = float[]( 1.0, 1.0, 1.0, 1.0, 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0);

vec3 edge_end_point(vec2 currentUv, vec2 offset, float lumaLocalAverage, float gradientScaled)
{
    // 边缘端点
    vec2 endpoint = currentUv;
    float lumaEnd = 0.0;

    for(int i = 0; i < ITERATION; ++i)
    {
        endpoint = endpoint + offset * steps[i];

        lumaEnd = rgb2luma(texture(frame, endpoint).rgb) - lumaLocalAverage;

        // 大于该方向上的阈值，认为到达了边缘的端点处
        if(abs(lumaEnd) >= gradientScaled)
            break;
    }

    return vec3(endpoint, lumaEnd);
}

// 阈值与最大亮度系数
const float EDGE_THRESHOLD_MAX = 0.166;
// 最低判断阈值
const float EDGE_THRESHOLD_MIN = 0.0833;

//#define ONE_SHOW_EDGE
//#define TWO_SHOW_MAIN_DIR

void main()
{    
    mat3 luma = around_luma(vTex);

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
    if (isHorizontal) // 垂直？？？
        FragColor = vec4(1, 0, 0, 1);
    else
        FragColor = vec4(1, 1, 1, 1);

    return;
#endif

    // 寻找两端
    // ??????
    float lumaN = isHorizontal ? luma[0][1] : luma[1][0];
    float lumaP = isHorizontal ? luma[2][1] : luma[1][2];

    float gradientN = lumaN - luma[1][1];
    float gradientP = lumaP - luma[1][1];

    bool is_negative_steepest = abs(gradientN) >= abs(gradientP);

    float lumaMain = is_negative_steepest ? lumaN : lumaP;

    // 平均亮度
    float lumaLocalAverage = 0.5 * (lumaMain + luma[1][1]);

    // 阈值
    float gradientScaled = 0.25 * max( abs(gradientN), abs(gradientP) );

    // 纹素尺寸
    vec2 texelSize = 1.0 / textureSize(frame, 0);
    float stepLength = isHorizontal ? texelSize.y : texelSize.x;

    // 沿边缘方向移动半个纹素
    vec2 currentUv = vTex;
    if(isHorizontal)
        currentUv.y += stepLength * 0.5; // 比如说当前是竖直线段正方向，就往上移半格到边界上
    else
        currentUv.x += stepLength * 0.5;

    // 一个纹素偏移量
    // ??????
    vec2 offset = isHorizontal ? vec2(texelSize.x, 0.0) : vec2(0.0, texelSize.y);



    // 线段端点
    vec3 endN = edge_end_point(currentUv, -offset, lumaLocalAverage, gradientScaled);
    vec2 edgeN = endN.xy;

    vec3 endP = edge_end_point(currentUv,  offset, lumaLocalAverage, gradientScaled);
    vec2 edgeP = endP.xy;

    // 两侧线段长度
    float distanceN = isHorizontal ? (vTex.x - edgeN.x) : (vTex.y - edgeN.y);
    float distanceP = isHorizontal ? (edgeP.x - vTex.x) : (edgeP.y - vTex.y);

    // 最终偏移量
    float pixelOffset = - (min(distanceN, distanceP) / (distanceN + distanceP)) + 0.5;

    // 中心和平均亮度比较
    bool isLumaCenterSmaller = luma[0][0] < lumaLocalAverage;

    // 近端点 和 中心到平均亮度的变化不一致
    bool correctVariation = (( (distanceN < distanceP) ? endN.z : endP.z) < 0.0) != isLumaCenterSmaller;

    // 如果不一致则不应用偏移量
    float finalOffset = correctVariation ? pixelOffset : 0.0;



    vec2 finalUv = vTex;
    if(isHorizontal)
        finalUv.y += finalOffset * stepLength;
    else
        finalUv.x += finalOffset * stepLength;

    FragColor = vec4(texture(frame, finalUv).rgb, 1.0);
}