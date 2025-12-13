#ifndef GAME_H
#define GAME_H

#define GAME_RELEASE_BUILD  0

#define GAME_VERSION_STABLE "-stable"
#define GAME_VERSION_BETA   "-beta"
#define GAME_VERSION_ALPHA  "-alpha"
#define GAME_VERSION_DEV    "-dev"

#define GAME_AUTHOR         "Author: Lily Awertnex"
#define GAME_NAME           "Heaven-Hell Continuum"
#define GAME_VERSION        "0.4.0"GAME_VERSION_DEV

#include <engine/h/core.h>
#include <engine/h/diagnostics.h>
#include <engine/h/types.h>

/* ---- settings ------------------------------------------------------------ */

#define MODE_INTERNAL_VSYNC                     0
#define MODE_INTERNAL_DEBUG                     1
#define MODE_INTERNAL_LOAD_CHUNKS               1
#define MODE_INTERNAL_COLLIDE                   1

#define show_cursor     glfwSetInputMode(render.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL)
#define disable_cursor  glfwSetInputMode(render.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED)
#define center_cursor   glfwSetCursorPos(render.window, render.size.x / 2.0f, render.size.y / 2.0f)

/* ---- internal ------------------------------------------------------------ */

#define SET_MARGIN                      10
#define SET_CAMERA_DISTANCE_MAX         4.0f
#define SET_REACH_DISTANCE_MAX          5.0f
#define SET_DAY_TICKS_MAX               24000
#define SET_RENDER_DISTANCE_DEFAULT     6
#define SET_RENDER_DISTANCE_MIN         2
#define SET_RENDER_DISTANCE_MAX         32
#define SET_FOV_DEFAULT                 70
#define SET_FOV_MIN                     30
#define SET_FOV_MAX                     160
#define SET_MOUSE_SENSITIVITY_DEFAULT   100
#define SET_MOUSE_SENSITIVITY_MIN       10
#define SET_MOUSE_SENSITIVITY_MAX       200
#define SET_TARGET_FPS_DEFAULT          60
#define SET_TARGET_FPS_MIN              1
#define SET_TARGET_FPS_MAX              241     /* 240 max, 241 = unlimited */
#define SET_GUI_SCALE_DEFAULT           2
#define SET_GUI_SCALE_0                 0 /* TODO: auto gui scale */
#define SET_GUI_SCALE_1                 1
#define SET_GUI_SCALE_2                 2
#define SET_GUI_SCALE_3                 3
#define SET_GUI_SCALE_4                 4
#define SET_LERP_SPEED_DEFAULT          25.0f
#define SET_LERP_SPEED_FOV_MODE         16.0f
#define SET_COLLISION_CAPSULE_PADDING   1.0f
#define SET_COLLISION_EPSILON           1e-3f
#define SET_COLLISION_SWEEP_MAX         3
#define SET_DRAG_DEFAULT                2.0f
#define SET_DRAG_AIR                    0.1f
#define SET_DRAG_FLYING                 4.0f
#define SET_DRAG_FLYING_V               13.0f
#define SET_DRAG_WALK                   18.0f
#define SET_PLAYER_EYE_HEIGHT           1.55f
#define SET_PLAYER_JUMP_INITIAL_VELOCITY 6.5f
#define SET_PLAYER_ACCELERATION_SNEAK   1.5f
#define SET_PLAYER_ACCELERATION_WALK    3.0f
#define SET_PLAYER_ACCELERATION_SPRINT  5.0f
#define SET_PLAYER_ACCELERATION_FLY     9.0f
#define SET_PLAYER_ACCELERATION_FLY_FAST 40.0f
#define SET_PLAYER_ACCELERATION_MAX     100.0f
#define SET_HOTBAR_SLOTS_MAX            10
#define SET_INVENTORY_SLOTS_MAX         (SET_HOTBAR_SLOTS_MAX * 4)

#define CHUNK_DIAMETER  16
#define CHUNK_LAYER     (CHUNK_DIAMETER * CHUNK_DIAMETER)
#define CHUNK_VOLUME    (CHUNK_DIAMETER * CHUNK_DIAMETER * CHUNK_DIAMETER)

#define WORLD_SEA_LEVEL         0
#define WORLD_RADIUS            2048    /* chunk count */
#define WORLD_RADIUS_VERTICAL   64      /* chunk count */

#define WORLD_DIAMETER          (WORLD_RADIUS * 2 + 1)
#define WORLD_DIAMETER_VERTICAL (WORLD_RADIUS_VERTICAL * 2 + 1)
#define WORLD_MAX_CHUNKS        (WORLD_DIAMETER * WORLD_DIAMETER * WORLD_DIAMETER_VERTICAL)

#define CHUNK_BUF_RADIUS_MAX    SET_RENDER_DISTANCE_MAX
#define CHUNK_BUF_DIAMETER_MAX  (CHUNK_BUF_RADIUS_MAX * 2 + 1)
#define CHUNK_BUF_LAYER_MAX     (CHUNK_BUF_DIAMETER_MAX * CHUNK_BUF_DIAMETER_MAX)
#define CHUNK_BUF_VOLUME_MAX    (CHUNK_BUF_DIAMETER_MAX * CHUNK_BUF_DIAMETER_MAX * CHUNK_BUF_DIAMETER_MAX)

#define CHUNK_QUEUES_MAX        3
#define CHUNK_QUEUE_1ST_ID      0
#define CHUNK_QUEUE_2ND_ID      1
#define CHUNK_QUEUE_3RD_ID      2
#define CHUNK_QUEUE_LAST_ID     CHUNK_QUEUE_3RD_ID
#define CHUNK_QUEUE_1ST_MAX     256
#define CHUNK_QUEUE_2ND_MAX     4096
#define CHUNK_QUEUE_3RD_MAX     16384

/*! @brief count of temporary static buffers in internal functions
 *  'chunk_mesh_init()' and 'chunk_mesh_update()'.
 */
#define BLOCK_BUFFERS_MAX       2

/*! @brief number of chunks to process per frame.
 */
#define CHUNK_PARSE_RATE_PRIORITY_LOW       128
#define CHUNK_PARSE_RATE_PRIORITY_MID       256
#define CHUNK_PARSE_RATE_PRIORITY_HIGH      CHUNK_VOLUME

/*! @brief number of blocks to process per chunk per frame.
 */
#define BLOCK_PARSE_RATE                    768

#define TERRAIN_SEED_DEFAULT    0
#define RAND_TAB_DIAMETER       128
#define RAND_TAB_VOLUME         (RAND_TAB_DIAMETER * RAND_TAB_DIAMETER * RAND_TAB_DIAMETER)

#define COLOR_TEXT_DEFAULT                  DIAGNOSTIC_COLOR_DEBUG
#define COLOR_TEXT_BRIGHT                   DIAGNOSTIC_COLOR_DEFAULT
#define COLOR_TEXT_MOSS                     0x6f9f3fff
#define COLOR_TEXT_RADIOACTIVE              0x3f9f3fff
#define COLOR_DIAGNOSTIC_NONE               0x995429ff
#define COLOR_DIAGNOSTIC_ERROR              0xec6051ff
#define COLOR_DIAGNOSTIC_INFO               0x3f6f9fff
#define COLOR_CHUNK_LOADED                  0x4c260715
#define COLOR_CHUNK_RENDER                  0x5e7a0aff

#define FILE_NAME_SETTINGS      "settings.txt"
#define FILE_NAME_WORLD_SEED    "seed.txt"

enum MainFlag
{
    FLAG_MAIN_ACTIVE                        = 0x00000001,
    FLAG_MAIN_PAUSED                        = 0x00000002,
    FLAG_MAIN_PARSE_CURSOR                  = 0x00000004,
    FLAG_MAIN_HUD                           = 0x00000008,
    FLAG_MAIN_DEBUG                         = 0x00000010,
    FLAG_MAIN_SUPER_DEBUG                   = 0x00000020,
    FLAG_MAIN_FULLSCREEN                    = 0x00000040,
    FLAG_MAIN_MENU_OPEN                     = 0x00000080,
    FLAG_MAIN_DOUBLE_PRESS                  = 0x00000100,
    FLAG_MAIN_PARSE_TARGET                  = 0x00000200,
    FLAG_MAIN_WORLD_LOADED                  = 0x00000400,
    FLAG_MAIN_CHUNK_BUF_DIRTY               = 0x00000800,
}; /* MainFlag */

enum DebugMode
{
    DEBUG_MODE_TRANS_BLOCKS,
    DEBUG_MODE_CHUNK_BOUNDS,
    DEBUG_MODE_BOUNDING_BOXES,
    DEBUG_MODE_CHUNK_GIZMO,
    DEBUG_MODE_CHUNK_QUEUE_VISUALIZER,
    DEBUG_MODE_COUNT,
}; /* DebugMode */

typedef struct WorldInfo
{
    u64 id;
    str name[NAME_MAX];
    u32 type;
    u64 seed;
    u64 tick;
    u64 days;
} WorldInfo;

struct Settings
{
    /* ---- internal -------------------------------------------------------- */

    /*! @brief conversion from world-space to screen-space.
     *
     *  @remark read-only, updated internally in 'main.c/settings_update()'.
     */
    v2f32 ndc_scale;

    u32 fps;
    f32 lerp_speed;
    u32 chunk_buf_radius;
    u32 chunk_buf_diameter;
    u32 chunk_buf_layer;
    u32 chunk_buf_volume;
    u32 chunk_tab_center;

    /*! @brief player reach (arm length).
     */
    u8 reach_distance;

    WorldInfo world;

    /* ---- controls -------------------------------------------------------- */

    f32 mouse_sensitivity;

    /* ---- video ----------------------------------------------------------- */

    u32 gui_scale;
    f32 font_size;
    u32 target_fps;

    /* ---- graphics -------------------------------------------------------- */

    f32 fov;
    u32 render_distance;
    b8 anti_aliasing;
}; /* Settings */

struct Uniform
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
        GLint mat_rotation;
        GLint mat_orientation;
        GLint mat_projection;
        GLint texture_sky;
        GLint texture_horizon;
        GLint texture_stars;
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

}; /* Uniform */

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
    TEXTURE_SKYBOX_VAL,
    TEXTURE_SKYBOX_HORIZON,
    TEXTURE_SKYBOX_STARS,
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

enum PlayerFlag
{
    FLAG_PLAYER_CAN_JUMP =      0x00000001,
    FLAG_PLAYER_SNEAKING =      0x00000002,
    FLAG_PLAYER_SPRINTING =     0x00000004,
    FLAG_PLAYER_FLYING =        0x00000008,
    FLAG_PLAYER_MID_AIR =       0x00000010,
    FLAG_PLAYER_SWIMMING =      0x00000020,
    FLAG_PLAYER_HUNGRY =        0x00000040,
    FLAG_PLAYER_DEAD =          0x00000080,
    FLAG_PLAYER_ZOOMER =        0x00000100,

    FLAG_PLAYER_OVERFLOW_X =    0x00000200,
    FLAG_PLAYER_OVERFLOW_Y =    0x00000400,
    FLAG_PLAYER_OVERFLOW_Z =    0x00000800,

    /*! @brief positive overflow direction flags,
     *  @remark default is 0 for negative overflow (underflow).
     */
    FLAG_PLAYER_OVERFLOW_PX =   0x00001000,
    FLAG_PLAYER_OVERFLOW_PY =   0x00002000,
    FLAG_PLAYER_OVERFLOW_PZ =   0x00004000,
}; /* PlayerFlag */

enum CameraModes
{
    MODE_CAMERA_1ST_PERSON,
    MODE_CAMERA_3RD_PERSON,
    MODE_CAMERA_3RD_PERSON_FRONT,
    MODE_CAMERA_STALKER,
    MODE_CAMERA_SPECTATOR,
    MODE_CAMERA_COUNT,
}; /* CameraModes */

typedef struct BoundingBox
{
    v3f64 pos;
    v3f32 size;
} BoundingBox;

/*! @brief region for which collisions checks are limited within.
 */
typedef struct CollisionCapsule
{
    v3i64 pos;
    v3i32 size;
} CollisionCapsule;

typedef struct Player
{
    str name[64];                   /* in-game name */
    u64 flag;                       /* enum: PlayerFlag */
    v3f64 pos;                      /* coordinates in world */
    v3f64 pos_last;                 /* coordinates in world of previous frame */
    v3f32 size;                     /* size (for collision detection) */
    v3f64 target;                   /* arm */
    v3i64 target_snapped;           /* floor of 'target' */

    f32 yaw, pitch;                 /* look direction */
    f32 sin_yaw;                    /* sine of 'yaw' */
    f32 cos_yaw;                    /* cosine of 'yaw' */
    f32 sin_pitch;                  /* sine of 'pitch' */
    f32 cos_pitch;                  /* cosine of 'pitch' */
    f32 eye_height;                 /* eye-level (camera height) */

    v3f32 input;                    /* raw user input */
    v3f32 acceleration;
    f32 acceleration_rate;          /* scalar for 'acceleration' */
    v3f32 velocity;
    f32 speed;                      /* derived from 'velocity' */
    v3f32 drag;
    f32 weight;

    Camera camera;
    Camera camera_hud;              /* for hud 3d elements */
    f32 camera_distance;            /* for camera collision detection */
    u8 camera_mode;                 /* enum: CameraModes */

    /*! @brief player at world edge, enum: PlayerFlag.
     */
    u8 overflow;

    v3i32 chunk;                    /* current chunk */
    v3i32 chunk_delta;              /* previous chunk */

    v3i64 spawn;                    /* spawn point */
    u64 container_state;            /* enum: ContainerFlag */
    u32 fly_natural;                /* nice flying kinematics */

    /*! @remark signed instead of unsigned so it's possible to navigate
     *  'hotbar_slots' when using mousewheel, used for wrapping around
     *  when out of range.
     */
    i32 hotbar_slot_selected;

    u32 hotbar_slots[SET_HOTBAR_SLOTS_MAX];
    u32 inventory_slots[SET_INVENTORY_SLOTS_MAX];

    BoundingBox bbox;
    CollisionCapsule capsule;
} Player;

enum BlockFlag
{
    /* 63 [00000000 00000000 00000000 00000000] 32;
     * 31 [00000000 00000001 00000000 00000000] 00; */
    FLAG_BLOCK_FACE_PX =        0x0000000000010000,

    /* 63 [00000000 00000000 00000000 00000000] 32;
     * 31 [00000000 00000010 00000000 00000000] 00; */
    FLAG_BLOCK_FACE_NX =        0x0000000000020000,

    /* 63 [00000000 00000000 00000000 00000000] 32;
     * 31 [00000000 00000100 00000000 00000000] 00; */
    FLAG_BLOCK_FACE_PY =        0x0000000000040000,

    /* 63 [00000000 00000000 00000000 00000000] 32;
     * 31 [00000000 00001000 00000000 00000000] 00; */
    FLAG_BLOCK_FACE_NY =        0x0000000000080000,

    /* 63 [00000000 00000000 00000000 00000000] 32;
     * 31 [00000000 00010000 00000000 00000000] 00; */
    FLAG_BLOCK_FACE_PZ =        0x0000000000100000,

    /* 63 [00000000 00000000 00000000 00000000] 32;
     * 31 [00000000 00100000 00000000 00000000] 00; */
    FLAG_BLOCK_FACE_NZ =        0x0000000000200000,

    /*! @brief run-length encoding, for chunk serialization.
     *
     * 63 [00000000 00000000 00000000 00000000] 32;
     * 31 [00000000 10000000 00000000 00000000] 00; */
    FLAG_BLOCK_RLE =            0x0000000000800000,
}; /* BlockFlag */

enum BlockMask
{
    /* 63 [00000000 00000000 00000000 00000000] 32;
     * 31 [00000000 00000000 00111111 11111111] 00; */
    MASK_BLOCK_DATA =           0x0000000000003fff,

    /* 63 [00000000 00000000 00000000 00000000] 32;
     * 31 [00000000 00000000 00000011 11111111] 00; */
    MASK_BLOCK_ID =             0x00000000000003ff,

    /* 63 [00000000 00000000 00000000 00000000] 32;
     * 31 [00000000 00000000 00111100 00000000] 00; */
    MASK_BLOCK_STATE =          0x0000000000003c00,

    /* 63 [00000000 00000000 00000000 00000000] 32;
     * 31 [00000000 00111111 00000000 00000000] 00; */
    MASK_BLOCK_FACES =          0x00000000003f0000,

    /* 63 [00000000 00000000 00000000 00000000] 32;
     * 31 [00111111 00000000 00000000 00000000] 00; */
    MASK_BLOCK_LIGHT =          0x000000003f000000,

    /* 63 [00000000 00000000 00001111 11111111] 32;
     * 31 [00000000 00000000 00000000 00000000] 00; */
    MASK_BLOCK_COORDINATES =    0x00000fff00000000,

    /* 63 [00000000 00000000 00000000 00001111] 32;
     * 31 [00000000 00000000 00000000 00000000] 00; */
    MASK_BLOCK_X =              0x0000000f00000000,

    /* 63 [00000000 00000000 00000000 11110000] 32;
     * 31 [00000000 00000000 00000000 00000000] 00; */
    MASK_BLOCK_Y =              0x000000f000000000,

    /* 63 [00000000 00000000 00001111 00000000] 32;
     * 31 [00000000 00000000 00000000 00000000] 00; */
    MASK_BLOCK_Z =              0x00000f0000000000,
}; /* BlockMask */

enum BlockShift
{
    SHIFT_BLOCK_DATA =          0,
    SHIFT_BLOCK_ID =            0,
    SHIFT_BLOCK_STATE =         10,
    SHIFT_BLOCK_FACES =         16,
    SHIFT_BLOCK_LIGHT =         24,
    SHIFT_BLOCK_COORDINATES =   32,
    SHIFT_BLOCK_X =             32,
    SHIFT_BLOCK_Y =             36,
    SHIFT_BLOCK_Z =             40,
}; /* BlockShift */

enum ChunkFlag
{
    FLAG_CHUNK_LOADED =     0x01,
    FLAG_CHUNK_DIRTY =      0x02,
    FLAG_CHUNK_QUEUED =     0x04,
    FLAG_CHUNK_GENERATED =  0x08,
    FLAG_CHUNK_RENDER =     0x10,
    FLAG_CHUNK_MODIFIED =   0x20,

    /*! @brief chunk marking for 'chunk_tab' shifting logic.
     */
    FLAG_CHUNK_EDGE =       0x40,
}; /* ChunkFlag */

enum ChunkShiftState
{
    STATE_CHUNK_SHIFT_PX = 1,
    STATE_CHUNK_SHIFT_NX = 2,
    STATE_CHUNK_SHIFT_PY = 3,
    STATE_CHUNK_SHIFT_NY = 4,
    STATE_CHUNK_SHIFT_PZ = 5,
    STATE_CHUNK_SHIFT_NZ = 6,
}; /* ChunkShiftState */

typedef struct Chunk
{
    v3i16 pos;      /* world position / CHUNK_DIAMETER */

    /*! @brief chunk's unique id derived from its position.
     *
     * format:
     * (pos.x & 0xffff) << 0x00 |
     * (pos.y & 0xffff) << 0x10 |
     * (pos.z & 0xffff) << 0x20.
     */
    u64 id;

    /*! @brief debug color,
     *
     *  format: 0xrrggbbaa.
     */
    u32 color;

    /*! @brief block iterator for per-chunk generation progress.
     */
    u32 cursor;

    u32 block[CHUNK_DIAMETER][CHUNK_DIAMETER][CHUNK_DIAMETER];
    GLuint vao;
    GLuint vbo;
    u64 vbo_len;
    u8 flag;
} Chunk;

typedef struct ChunkQueue
{
    u32 id;
    u32 count;          /* number of chunks queued */
    u32 offset;         /* first CHUNK_ORDER index to queue */
    u64 size;
    u32 cursor;         /* parse position */
    u32 rate_chunk;     /* number of chunks to process per frame */
    u32 rate_block;     /* number of blocks to process per chunk per frame */
    Chunk ***queue;
} ChunkQueue;

/*! @brief global pointer to variable for game/engine-specific error codes.
 *
 *  @remark must be initialized globally, tho the pointed to variable itself can be modified.
 */
extern u32 *const GAME_ERR;
extern u32 chunk_tab_index;

extern struct Settings settings;
extern Texture texture[TEXTURE_COUNT];
extern Font font[FONT_COUNT];
extern u64 flag;
extern f64 game_start_time;
extern u8 debug_mode[DEBUG_MODE_COUNT];

#endif /* GAME_MAIN_H */
