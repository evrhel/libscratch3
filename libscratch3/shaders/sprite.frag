#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform float uColorEffect;
uniform float uBrightnessEffect;
uniform float uFisheyeEffect;
uniform float uWhirlEffect;
uniform float uPixelateEffect;
uniform float uMosaicEffect;
uniform float uGhostEffect;

uniform sampler2D uTexture;

uniform vec4 uColor;

uniform bool uUseColorMask;
uniform vec3 uColorMask;

const float kEpsilon = 0.0001;

const vec3 kColorMaskTolerance = vec3(2.0 / 255.0);

vec3 rgb2hsv(vec3 c)
{
    const vec4 offsets = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 a = c.b > c.g ? vec4(c.bg, offsets.wz) : vec4(c.gb, offsets.xy);
    vec4 b = c.r > a.x ? vec4(c.r, a.yzx) : vec4(a.xyw, c.r);

    float m = min(b.y, b.w);

    float C = b.x - m;
    float V = b.x;
    float H = abs(b.z + (b.w - b.y) / (6.0 * C + kEpsilon));

    return vec3(H, C / (b.x + kEpsilon), V);
}

vec3 hue2rgb(float H)
{
    float r = abs(H * 6.0 - 3.0) - 1.0;
    float g = 2.0 - abs(H * 6.0 - 2.0);
    float b = 2.0 - abs(H * 6.0 - 4.0);
    return clamp(vec3(r, g, b), 0.0, 1.0);
}

vec3 hsv2rgb(vec3 c)
{
    vec3 rgb = hue2rgb(c.x);
    float C = c.z * c.y;
    return rgb * C + c.z - C;
}

vec2 mosaicEffect(vec2 uv)
{
    return fract(uv * uMosaicEffect);
}

vec2 pixelateEffect(vec2 uv)
{
    vec2 size = vec2(textureSize(uTexture, 0)) / uPixelateEffect;
    return (floor(uv * size) + 0.5) / size;
}

vec2 whirlEffect(vec2 uv)
{
    const float radius = 0.5;

    vec2 offset = uv - 0.5;
    float len = length(offset);
    float factor = max(1.0 - len / radius, 0.0);
    factor = uWhirlEffect * factor * factor;

    float sinWhirl = sin(factor);
    float cosWhirl = cos(factor);

    mat2 rotation = mat2(cosWhirl, -sinWhirl, sinWhirl, cosWhirl);

    return rotation * offset + 0.5;
}

vec2 fisheyeEffect(vec2 uv)
{
    vec2 vec = (uv - 0.5) * 2.0;
    float len = length(vec);
    float factor = pow(min(len, 1.0), uFisheyeEffect) * max(1.0, len);
    return factor * (vec / len) * 0.5 + 0.5;
}

vec4 colorEffect(vec4 color)
{
    // Return immediately if no effect is applied
    // We must do this as a small change is always applied even if the effect is 0.0
    if (uColorEffect == 0.0)
        return color;
    
    vec3 hsv = rgb2hsv(color.rgb);

    const float minBrightness = 0.11 / 2.0;
    const float minSaturation = 0.09;

    if (hsv.z < minBrightness)
        hsv = vec3(0.0, 1.0, minBrightness);
    else if (hsv.y < minSaturation)
        hsv = vec3(0.0, minSaturation, hsv.z);

    hsv.x = mod(hsv.x + uColorEffect, 1.0);
    if (hsv.x < 0.0)
        hsv.x += 1.0;

    return vec4(hsv2rgb(hsv), color.a);
}

vec4 brightnessEffect(vec4 color)
{
    return vec4(clamp(color.rgb * uBrightnessEffect, 0.0, 1.0), color.a);
}

vec4 ghostEffect(vec4 color)
{
    return vec4(color.rgb, color.a * (1 - uGhostEffect));
}

void main()
{
    vec2 texCoord = TexCoords;

    texCoord = mosaicEffect(texCoord);
    //texCoord = pixelateEffect(texCoord);
    texCoord = whirlEffect(texCoord);
    texCoord = fisheyeEffect(texCoord);

    vec4 color = texture(uTexture, texCoord);

    //color.rgb = clamp(color.rgb / (color.a + kEpsilon), 0.0, 1.0);
    
    color = colorEffect(color);

    color = brightnessEffect(color);

    color = ghostEffect(color);

    FragColor = color * uColor;

    // Alpha test
    if (FragColor.a < 0.01)
        discard;

    // Color mask
    if (uUseColorMask)
    {
        vec3 dist = abs(FragColor.rgb - uColorMask);
        if (any(greaterThan(dist, kColorMaskTolerance)))
            discard;
    }
}
