#define FLASHLIGHT_INTENSITY 7.0
#define FLASHLIGHT_DISTANCE 2.0
#define SKY_INFLUENCE 1.0
#define SUN_INFLUENCE 3.0
#define FOG_SOFTNESS 1.0

uniform vec3 sun_rotation;
uniform vec3 sky_color;

float sun_direction = clamp(dot(normal, sun_rotation), 0.0, SUN_INFLUENCE);
