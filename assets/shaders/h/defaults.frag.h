#define FLASHLIGHT_INTENSITY 20.0
#define FLASHLIGHT_SHARPNESS 3.0
#define SKY_BRIGHTNESS 2.5
#define SKY_INFLUENCE 0.25
#define SUN_INFLUENCE 1.0
#define BRIGHTNESS 1.0
#define CONTRAST 0.5

uniform vec3 sun_rotation;
uniform vec3 sky_color;

float sky_brightness = SKY_BRIGHTNESS *
    dot(sky_color.rgb, vec3(0.2126, 0.0722, 0.6152));
float sky_color_influence = clamp(sky_brightness * SKY_INFLUENCE, 0.0, 1.0);
float sun_direction = (dot(normal, sun_rotation) + 1.0) / 2.0;
