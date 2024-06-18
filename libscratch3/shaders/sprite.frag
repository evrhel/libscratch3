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

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
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
    vec3 hsv = rgb2hsv(color.rgb);

    const float minBrightness = 0.11 / 2.0;
    const float minSaturation = 0.09;

    if (hsv.z < minBrightness)
        hsv = vec3(0.0, 1.0, minBrightness);
    else if (hsv.y < minSaturation)
        hsv = vec3(hsv.x, minSaturation, hsv.z);

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
    return mix(color, vec4(0.0), uGhostEffect);
}

void main()
{
    vec2 texCoord = TexCoords;

    texCoord = mosaicEffect(texCoord);
    //texCoord = pixelateEffect(texCoord);
    texCoord = whirlEffect(texCoord);
    texCoord = fisheyeEffect(texCoord);

    vec4 color = texture(uTexture, texCoord);

    const float epsilon = 0.0001;

    //color.rgb = clamp(color.rgb / (color.a + epsilon), 0.0, 1.0);

    //color = colorEffect(color);
    color = brightnessEffect(color);

    //color.rgb *= color.a + epsilon;

    color = ghostEffect(color);

    FragColor = color * uColor;
}
