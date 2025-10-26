#ifndef GAME_H
#define GAME_H

#define GAME_RELEASE_BUILD  0

#define GAME_VERSION_STABLE "-stable"
#define GAME_VERSION_BETA   "-beta"
#define GAME_VERSION_ALPHA  "-alpha"
#define GAME_VERSION_DEV    "-dev"

#define GAME_AUTHOR         "Author: Lily Awertnex"
#define GAME_NAME           "Heaven-Hell Continuum"
#define GAME_VERSION        "0.2.4"GAME_VERSION_DEV

#include <engine/h/core.h>
#include <engine/h/defines.h>
#include <engine/h/diagnostics.h>

#define MODE_INTERNAL_VSYNC                     0
#define MODE_INTERNAL_DEBUG                     1
#define MODE_INTERNAL_COLLIDE                   1
#define MODE_INTERNAL_CHUNK_QUEUE_VISUALIZER    0

#define show_cursor \
    glfwSetInputMode(render.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL)

#define disable_cursor \
    glfwSetInputMode(render.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED)

#define center_cursor \
    glfwSetCursorPos(render.window, render.size.x / 2.0f, render.size.y / 2.0f)

/* ---- internal ------------------------------------------------------------ */

#define SET_MARGIN                      10
#define SET_CAMERA_DISTANCE_MAX         4.0f
#define SET_REACH_DISTANCE_MAX          5.0f
#define SET_DAY_TICKS_MAX               24000
#define SET_RENDER_DISTANCE             16
#define SET_RENDER_DISTANCE_DEFAULT     6
#define SET_RENDER_DISTANCE_MIN         2
#define SET_RENDER_DISTANCE_MAX         32
#define SET_FOV_DEFAULT                 70.0f
#define SET_FOV_MIN                     30.0f
#define SET_FOV_MAX                     150.0f
#define SET_MOUSE_SENSITIVITY_DEFAULT   100.0f
#define SET_MOUSE_SENSITIVITY_MIN       10.0f
#define SET_MOUSE_SENSITIVITY_MAX       200.0f
#define SET_TARGET_FPS_DEFAULT          60
#define SET_TARGET_FPS_MIN              1
#define SET_TARGET_FPS_MAX              256
#define SET_GUI_SCALE_DEFAULT           2.0f
#define SET_GUI_SCALE_0                 0.0f /* TODO: auto gui scale */
#define SET_GUI_SCALE_1                 1.0f
#define SET_GUI_SCALE_2                 2.0f
#define SET_GUI_SCALE_3                 3.0f
#define SET_GUI_SCALE_4                 4.0f
#define SET_LERP_SPEED_DEFAULT          20.0f
#define SET_LERP_SPEED_GLIDE            2.5f
#define SET_LERP_SPEED_RIGID            100.0f
#define SET_PLAYER_EYE_HEIGHT           1.55f
#define SET_PLAYER_JUMP_HEIGHT          8.0f
#define SET_PLAYER_SPEED_WALK           4.0f
#define SET_PLAYER_SPEED_FLY            9.0f
#define SET_PLAYER_SPEED_FLY_FAST       50.0f
#define SET_PLAYER_SPEED_SNEAK          1.5f
#define SET_PLAYER_SPEED_SPRINT         8.0f
#define SET_PLAYER_SPEED_MAX            100.0f
#define SET_HOTBAR_SLOTS_MAX            10
#define SET_TERRAIN_SEED_DEFAULT        0

#define CHUNK_DIAMETER  16
#define CHUNK_LAYER     (CHUNK_DIAMETER * CHUNK_DIAMETER)
#define CHUNK_VOLUME    (CHUNK_DIAMETER * CHUNK_DIAMETER * CHUNK_DIAMETER)

#define WORLD_SEA_LEVEL         0
#define WORLD_RADIUS            2048    /* chunk count */
#define WORLD_RADIUS_VERTICAL   64      /* chunk count */

#define WORLD_DIAMETER          ((WORLD_RADIUS * 2) + 1)
#define WORLD_DIAMETER_VERTICAL ((WORLD_RADIUS_VERTICAL * 2) + 1)
#define WORLD_MAX_CHUNKS \
    (WORLD_DIAMETER * WORLD_DIAMETER * WORLD_DIAMETER_VERTICAL)

#define CHUNK_BUF_RADIUS        SET_RENDER_DISTANCE
#define CHUNK_BUF_DIAMETER      ((CHUNK_BUF_RADIUS * 2) + 1)
#define CHUNK_BUF_LAYER         (CHUNK_BUF_DIAMETER * CHUNK_BUF_DIAMETER)
#define CHUNK_BUF_VOLUME \
    (CHUNK_BUF_DIAMETER * CHUNK_BUF_DIAMETER * CHUNK_BUF_DIAMETER)

#define CHUNK_BUF_RADIUS_MAX    SET_RENDER_DISTANCE_MAX
#define CHUNK_BUF_DIAMETER_MAX ((CHUNK_BUF_RADIUS_MAX * 2) + 1)
#define CHUNK_BUF_LAYER_MAX \
    (CHUNK_BUF_DIAMETER_MAX * CHUNK_BUF_DIAMETER_MAX)
#define CHUNK_BUF_VOLUME_MAX \
    (CHUNK_BUF_DIAMETER_MAX * CHUNK_BUF_DIAMETER_MAX * CHUNK_BUF_DIAMETER_MAX)

#define CHUNK_TAB_CENTER \
    (CHUNK_BUF_RADIUS + \
     (CHUNK_BUF_RADIUS * CHUNK_BUF_DIAMETER) + \
     (CHUNK_BUF_RADIUS * CHUNK_BUF_DIAMETER * CHUNK_BUF_DIAMETER))

#define CHUNK_QUEUE_1ST_ID      0
#define CHUNK_QUEUE_2ND_ID      1
#define CHUNK_QUEUE_3RD_ID      2
#define CHUNK_QUEUE_LAST_ID     CHUNK_QUEUE_3RD_ID
#define CHUNK_QUEUE_1ST_MAX     256
#define CHUNK_QUEUE_2ND_MAX     4096
#define CHUNK_QUEUE_3RD_MAX     16384

/* 'chunk_mesh()' temp static buffer count */
#define BLOCK_BUFFERS_MAX       3

/* number of chunks to process per frame */
#define CHUNK_PARSE_RATE_PRIORITY_LOW       64
#define CHUNK_PARSE_RATE_PRIORITY_MID       128
#define CHUNK_PARSE_RATE_PRIORITY_HIGH      2048

/* number of blocks to process per chunk per frame */
#define BLOCK_PARSE_RATE                    512

#define COLOR_TEXT_DEFAULT                  DIAGNOSTIC_COLOR_DEBUG
#define COLOR_TEXT_BRIGHT                   DIAGNOSTIC_COLOR_DEFAULT
#define COLOR_TEXT_MOSS                     0x6f9f3fff
#define COLOR_TEXT_RADIOACTIVE              0x3f9f3fff
#define COLOR_DIAGNOSTIC_NONE               0x995429ff
#define COLOR_DIAGNOSTIC_ERROR              0xec6051ff
#define COLOR_DIAGNOSTIC_INFO               0x3f6f9fff
#define COLOR_CHUNK_LOADED                  0x4c260715
#define COLOR_CHUNK_RENDER                  0x5e7a0aff

enum MainFlags
{
    FLAG_MAIN_ACTIVE                        = 0x00000001,
    FLAG_MAIN_PAUSED                        = 0x00000002,
    FLAG_MAIN_PARSE_CURSOR                  = 0x00000004,
    FLAG_MAIN_HUD                           = 0x00000008,
    FLAG_MAIN_DEBUG                         = 0x00000010,
    FLAG_MAIN_DEBUG_MORE                    = 0x00000020,
    FLAG_MAIN_SUPER_DEBUG                   = 0x00000040,
    FLAG_MAIN_FULLSCREEN                    = 0x00000080,
    FLAG_MAIN_MENU_OPEN                     = 0x00000100,
    FLAG_MAIN_DOUBLE_PRESS                  = 0x00000200,
    FLAG_MAIN_PARSE_TARGET                  = 0x00000400,
    FLAG_MAIN_WORLD_LOADED                  = 0x00000800,
    FLAG_MAIN_CHUNK_BUF_DIRTY               = 0x00001000,
    FLAG_MAIN_DRAW_CHUNK_QUEUE_VISUALIZER   = 0x00002000,
}; /* MainFlags */

typedef struct Settings
{
    /* ---- internal -------------------------------------------------------- */

    /* conversion from world-space to screen-space */
    v2f32 ndc_scale;

    u32 fps;
    f32 lerp_speed;

    /* for player reach (arm length basically) */
    u8 reach_distance;

    /* ---- options --------------------------------------------------------- */

    f64 mouse_sensitivity;

    /* ---- video ----------------------------------------------------------- */

    u8 render_distance;
    u32 target_fps;
    f32 gui_scale;
} Settings;

typedef struct Uniform
{
    struct /* defaults */
    {
        GLint offset;
        GLint scale;
        GLint mat_rotation;
        GLint mat_perspective;
        GLint sun_rotation;
        GLint sky_color;
    } defaults;

    struct /* ui */
    {
        GLint ndc_scale;
        GLint position;
        GLint offset;
        GLint texture_size;
        GLint size;
        GLint alignment;
        GLint tint;
    } ui;

    struct /* ui_9_slice */
    {
        GLint ndc_scale;
        GLint position;
        GLint size;
        GLint alignment;
        GLint tint;
        GLint slice;
        GLint slice_size;
        GLint texture_size;
        GLint sprite_size;
    } ui_9_slice;

    struct /* font */
    {
        GLint char_size;
        GLint font_size;
        GLint text_color;
    } font;

    struct /* skybox */
    {
        GLint camera_position;
        GLint mat_rotation;
        GLint mat_orientation;
        GLint mat_projection;
        GLint time;
        GLint sun_rotation;
        GLint sky_color;
    } skybox;

    struct /* gizmo */
    {
        GLint mat_translation;
        GLint mat_rotation;
        GLint mat_orientation;
        GLint mat_projection;
        GLint color;
    } gizmo;

    struct /* gizmo_chunk */
    {
        GLint render_size;
        GLint render_distance;
        GLint mat_translation;
        GLint mat_rotation;
        GLint mat_orientation;
        GLint mat_projection;
        GLint cursor;
        GLint size;
        GLint camera_position;
        GLint sky_color;
        GLint color;
    } gizmo_chunk;

    struct /* post_processing */
    {
        GLint time;
    } post_processing;

    struct /* voxel */
    {
        GLint mat_perspective;
        GLint player_position;
        GLint sun_rotation;
        GLint sky_color;
        GLint chunk_position;
        GLint color;
        GLint opacity;
    } voxel;

    struct /* bounding_box */
    {
        GLint mat_perspective;
        GLint position;
        GLint size;
        GLint color;
    } bounding_box;

} Uniform;

enum ShaderIndices
{
    SHADER_FBO,
    SHADER_DEFAULT,
    SHADER_UI,
    SHADER_UI_9_SLICE,
    SHADER_TEXT,
    SHADER_SKYBOX,
    SHADER_GIZMO,
    SHADER_GIZMO_CHUNK,
    SHADER_POST_PROCESSING,
    SHADER_VOXEL,
    SHADER_BOUNDING_BOX,
    SHADER_COUNT,
}; /* ShaderIndices */

enum MeshIndices
{
    MESH_UNIT,
    MESH_SKYBOX,
    MESH_CUBE_OF_HAPPINESS,
    MESH_PLAYER,
    MESH_GIZMO,
    MESH_COUNT,
}; /* MeshIndices */

enum FBOIndices
{
    FBO_SKYBOX,
    FBO_WORLD,
    FBO_WORLD_MSAA,
    FBO_HUD,
    FBO_HUD_MSAA,
    FBO_UI,
    FBO_TEXT,
    FBO_TEXT_MSAA,
    FBO_POST_PROCESSING,
    FBO_COUNT,
}; /* FBOIndices */

enum TextureIndices
{
    TEXTURE_CROSSHAIR,
    TEXTURE_ITEM_BAR,
    TEXTURE_SDB_ACTIVE,
    TEXTURE_SDB_INACTIVE,
    TEXTURE_GRASS,
    TEXTURE_DIRT,
    TEXTURE_DIRTUP,
    TEXTURE_STONE,
    TEXTURE_SAND,
    TEXTURE_COUNT,
}; /* TextureIndices */

enum FontIndices
{
    FONT_REG,
    FONT_REG_BOLD,
    FONT_MONO,
    FONT_MONO_BOLD,
    FONT_COUNT,
}; /* FontIndices */

enum PlayerFlags
{
    FLAG_PLAYER_CAN_JUMP        = 0x00000001,
    FLAG_PLAYER_SNEAKING        = 0x00000002,
    FLAG_PLAYER_SPRINTING       = 0x00000004,
    FLAG_PLAYER_FLYING          = 0x00000008,
    FLAG_PLAYER_SWIMMING        = 0x00000010,
    FLAG_PLAYER_FALLING         = 0x00000020,
    FLAG_PLAYER_VELOCITY_DIRTY  = 0x00000040,
    FLAG_PLAYER_HUNGRY          = 0x00000080,
    FLAG_PLAYER_DEAD            = 0x00000100,
    FLAG_PLAYER_ZOOMER          = 0x00000200,

    FLAG_PLAYER_OVERFLOW_X      = 0x00001000,
    FLAG_PLAYER_OVERFLOW_Y      = 0x00002000,
    FLAG_PLAYER_OVERFLOW_Z      = 0x00004000,

    /* positive overflow direction flags,
     * default is negative (or 0) */
    FLAG_PLAYER_OVERFLOW_PX     = 0x00008000,
    FLAG_PLAYER_OVERFLOW_PY     = 0x00010000,
    FLAG_PLAYER_OVERFLOW_PZ     = 0x00020000,
}; /* PlayerFlags */

enum CameraModes
{
    MODE_CAMERA_1ST_PERSON,
    MODE_CAMERA_3RD_PERSON,
    MODE_CAMERA_3RD_PERSON_FRONT,
    MODE_CAMERA_STALKER,
    MODE_CAMERA_SPECTATOR,
    MODE_CAMERA_COUNT,
}; /* CameraModes */

typedef struct Player
{
    str name[100];                  /* player in-game name */
    v3f64 pos;                      /* player current coordinates in world */
    v3f64 pos_smooth;               /* player processed pos */
    v3f64 pos_last;                 /* for collision tunneling prevention */
    v3f32 pos_lerp_speed;
    v3f64 target;                   /* player arm (or whatever) */
    v3i64 delta_target;             /* player arm floored */
    v3f32 size;                     /* player size for collision detection */
    v3f64 collision_check_pos;
    v3f64 collision_check_size;
    f32 pitch, yaw;                 /* for player camera direction and target */
    f32 sin_pitch;                  /* processed player pitch sine angle */
    f32 cos_pitch;                  /* processed player pitch cosine angle */
    f32 sin_yaw;                    /* processed player yaw sine angle */
    f32 cos_yaw;                    /* processed player yaw cosine angle */
    f32 eye_height;                 /* height of player camera, usually */
    v3f32 vel;                      /* velocity */
    f32 mass;                       /* for gravity influence */
    f32 movement_speed;             /* depends on enum: PlayerFlags */
    u64 container_state;            /* enum: ContainerFlags */
    u8 perspective;                 /* camera perspective mode */
    u64 flag;                       /* enum: PlayerFlags */

    Camera camera;
    Camera camera_hud;
    f32 camera_distance;            /* for camera collision detection */

    /* player at world edge, enum: PlayerFlags */
    u8 overflow;

    v3i16 chunk;                    /* current chunk player is in */
    v3i16 delta_chunk;              /* previous chunk player was in */

    v3i64 spawn_point;
} Player;

enum BlockFlags
{
    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 00000000 00000100 00000000] 0 */
    FLAG_BLOCK_ACTIVE       = 0x0000000000000400,

    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 00000000 00001000 00000000] 0 */
    FLAG_BLOCK_ANIMATED     = 0x0000000000000800,

    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 00000001 00000000 00000000] 0 */
    FLAG_BLOCK_FACE_PX      = 0x0000000000010000,

    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 00000010 00000000 00000000] 0 */
    FLAG_BLOCK_FACE_NX      = 0x0000000000020000,

    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 00000100 00000000 00000000] 0 */
    FLAG_BLOCK_FACE_PY      = 0x0000000000040000,

    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 00001000 00000000 00000000] 0 */
    FLAG_BLOCK_FACE_NY      = 0x0000000000080000,

    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 00010000 00000000 00000000] 0 */
    FLAG_BLOCK_FACE_PZ      = 0x0000000000100000,

    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 00100000 00000000 00000000] 0 */
    FLAG_BLOCK_FACE_NZ      = 0x0000000000200000,

    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 01000000 00000000 00000000] 0 */
    FLAG_BLOCK_NOT_EMPTY    = 0x0000000000400000,

    /* run-length encoding
     * 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 10000000 00000000 00000000] 0 */
    FLAG_BLOCK_RLE          = 0x0000000000800000,
}; /* BlockFlags */

enum BlockMasks
{
    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 00000000 00000011 11111111] 0 */
    MASK_BLOCK_ID           = 0x00000000000003ff,

    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 00000000 00111100 00000000] 0 */
    MASK_BLOCK_STATE        = 0x0000000000003c00,

    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 00111111 00000000 00000000] 0 */
    MASK_BLOCK_FACES        = 0x00000000003f0000,

    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00111111 00000000 00000000 00000000] 0 */
    MASK_BLOCK_LIGHT        = 0x000000003f000000,

    /* 63 [00000000 00000000 00000000 00001111] 32
     * 31 [00000000 00000000 00000000 00000000] 0 */
    MASK_BLOCK_X            = 0x0000000f00000000,

    /* 63 [00000000 00000000 00000000 11110000] 32
     * 31 [00000000 00000000 00000000 00000000] 0 */
    MASK_BLOCK_Y            = 0x000000f000000000,

    /* 63 [00000000 00000000 00001111 00000000] 32
     * 31 [00000000 00000000 00000000 00000000] 0 */
    MASK_BLOCK_Z            = 0x00000f0000000000,

    /* 63 [00000000 00000000 00000000 00000000] 32
     * 31 [00000000 00000000 00111111 11111111] 0 */
    MASK_BLOCK_DATA         = 0x0000000000003fff,

    /* 63 [00000000 00000000 00001111 11111111] 32
     * 31 [00000000 00000000 00000000 00000000] 0 */
    MASK_BLOCK_COORDINATES  = 0x00000fff00000000,
}; /* BlockMasks */

enum ChunkFlags
{
    FLAG_CHUNK_LOADED       = 0x01,
    FLAG_CHUNK_DIRTY        = 0x02,
    FLAG_CHUNK_QUEUED       = 0x04,
    FLAG_CHUNK_GENERATED    = 0x08,
    FLAG_CHUNK_RENDER       = 0x10,

    /* chunk marking for chunk_tab shifting logic */
    FLAG_CHUNK_EDGE         = 0x20,
}; /* ChunkFlags */

enum ChunkStates
{
    STATE_CHUNK_SHIFT_PX    = 1,
    STATE_CHUNK_SHIFT_NX    = 2,
    STATE_CHUNK_SHIFT_PY    = 3,
    STATE_CHUNK_SHIFT_NY    = 4,
    STATE_CHUNK_SHIFT_PZ    = 5,
    STATE_CHUNK_SHIFT_NZ    = 6,

}; /* ChunkStates */

typedef struct Chunk
{
    v3i16 pos;      /* (world XYZ) / CHUNK_DIAMETER */
    u64 id;         /* hash: (pos.x << 32) + (pos.y << 16) + pos.z */
    u32 color;      /* debug color: 0xrrggbbaa */
    GLuint vao;
    GLuint vbo;
    u64 vbo_len;
    u32 cursor;     /* block iterator for per-chunk generation progress */
    u32 block[CHUNK_DIAMETER][CHUNK_DIAMETER][CHUNK_DIAMETER];
    u8 flag;
} Chunk;

typedef struct ChunkQueue
{
    /* queues */
    Chunk ***priority_1;
    Chunk ***priority_2;
    Chunk ***priority_3;

    /* size of queues */
    u64 size_1;
    u64 size_2;
    u64 size_3;

    /* number of chunks queued at queues */
    u32 count_1;
    u32 count_2;
    u32 count_3;

    /* parse position at queues */
    u32 cursor_1;
    u32 cursor_2;
    u32 cursor_3;
} ChunkQueue;

extern u32 *const GAME_ERR;
extern Settings settings;
extern Texture texture[TEXTURE_COUNT];
extern Font font[FONT_COUNT];
extern u64 flag;
extern f64 game_start_time;
extern u64 game_tick;
extern u64 game_days;

#endif /* GAME_MAIN_H */
