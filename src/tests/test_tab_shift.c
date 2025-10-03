#include <string.h>

#include "../engine/h/core.h"
#include "../engine/h/math.h"
#include "../h/main.h"
#include "../h/settings.h"
#include "../h/chunking.h"
#include "../h/gui.h"
#include "../h/logic.h"

#define CHUNK_LOADED    1
#define CHUNK_EDGE      2
#define CHUNK_SHIFTED   3
#define TEST_TAB_DIAMETER   33
#define TEST_TAB_LAYER      (33 * 33)
#define TEST_TAB_VOLUME     (33 * 33 * 33)
static u8 test_tab[TEST_TAB_VOLUME] = {0};
static u8 test_tab_val[TEST_TAB_VOLUME] = {0};
const u32 test_tab_center = 17 + (17 * 33) + (17 * 33 * 33);
const f32 render_distance = (16 * 16) + 2;

static void
init_test_tab(void)
{
    for (u32 i = 0; i < TEST_TAB_VOLUME; ++i)
        if (distance_v3i32((v3i32){i % 33, (i / 33) % 33, i / (33 * 33)},
                    (v3i32){16, 16, 16}) < (u32)render_distance)
            test_tab[i] = CHUNK_LOADED;
}

static void
mark_test_tab(u8 direction)
{
    const i8 INCREMENT = (direction % 2 == 1) - (direction % 2 == 0);
    const u8 AXIS = (direction + 1) / 2;
    u32 mirror_index = 0;
    u8 is_on_edge = 0;

    for (u32 i = 0; i < TEST_TAB_VOLUME; ++i)
        if (test_tab[i] == CHUNK_EDGE)
            test_tab[i] = CHUNK_LOADED;

    /* ---- mark chunks on-edge --------------------------------------------- */
    for (u32 i = 0; i < TEST_TAB_VOLUME; ++i)
    {
        if (!test_tab[i]) continue;

        v3u32 coordinates =
        {
            i % TEST_TAB_DIAMETER,
            (i / TEST_TAB_DIAMETER) % TEST_TAB_DIAMETER,
            i / TEST_TAB_LAYER,
        };

        v3u32 _mirror_index =
        {
            i + TEST_TAB_DIAMETER - 1 - (coordinates.x * 2),

            (coordinates.z * TEST_TAB_LAYER) +
                ((TEST_TAB_DIAMETER - 1 - coordinates.y) *
                 TEST_TAB_DIAMETER) + coordinates.x,

            ((TEST_TAB_DIAMETER - 1 - coordinates.z) * TEST_TAB_LAYER) +
                (coordinates.y * TEST_TAB_DIAMETER) + coordinates.x,
        };

        v3u8 _is_on_edge = {0};

        switch (INCREMENT)
        {
            case -1:
                _is_on_edge =
                    (v3u8){
                        (coordinates.x == TEST_TAB_DIAMETER - 1) ||
                            (test_tab[i + 1] == 0),

                        (coordinates.y == TEST_TAB_DIAMETER - 1) ||
                            (test_tab[i + TEST_TAB_DIAMETER] == 0),

                        (coordinates.z == TEST_TAB_DIAMETER - 1) ||
                            (test_tab[i + TEST_TAB_LAYER] == 0),
                    };
                break;

            case 1:
                _is_on_edge =
                    (v3u8){
                        (coordinates.x == 0) || (test_tab[i - 1] == 0),

                        (coordinates.y == 0) ||
                            (test_tab[i - TEST_TAB_DIAMETER] == 0),

                        (coordinates.z == 0) ||
                            (test_tab[i - TEST_TAB_LAYER] == 0),
                    };
                break;
        }

        switch (AXIS)
        {
            case SHIFT_X:
                mirror_index = _mirror_index.x;
                is_on_edge = _is_on_edge.x;
                break;

            case SHIFT_Y:
                mirror_index = _mirror_index.y;
                is_on_edge = _is_on_edge.y;
                break;

            case SHIFT_Z:
                mirror_index = _mirror_index.z;
                is_on_edge = _is_on_edge.z;
                break;
        }

        if (is_on_edge && test_tab[mirror_index])
                test_tab[mirror_index] = CHUNK_EDGE;
    }
}

static void
shift_test_tab(u8 direction)
{
    i8 INCREMENT = (direction % 2 == 1) - (direction % 2 == 0);
    u8 AXIS = direction / 2;
    u32 mirror_index = 0;
    u32 target_index = 0;
    u8 is_on_edge = 0;

    /* ---- shift test_tab -------------------------------------------------- */
    for (u32 i = (INCREMENT == 1) ? 0 : TEST_TAB_VOLUME - 1;
            i < TEST_TAB_VOLUME; i += INCREMENT)
    {
        if (!test_tab[i]) continue;

        v3u32 coordinates =
        {
            i % TEST_TAB_DIAMETER,
            (i / TEST_TAB_DIAMETER) % TEST_TAB_DIAMETER,
            i / TEST_TAB_LAYER,
        };

        v3u32 _mirror_index =
        {
            i + TEST_TAB_DIAMETER - 1 - (coordinates.x * 2),

            (coordinates.z * TEST_TAB_LAYER) +
                ((TEST_TAB_DIAMETER - 1 - coordinates.y) *
                 TEST_TAB_DIAMETER) + coordinates.x,

            ((TEST_TAB_DIAMETER - 1 - coordinates.z) * TEST_TAB_LAYER) +
                (coordinates.y * TEST_TAB_DIAMETER) + coordinates.x,
        };

        v3u32 _target_index = {0};

        switch (INCREMENT)
        {
            case -1:
                _target_index = (v3u32){
                        (coordinates.x == 0) ? i : i - 1,
                        (coordinates.y == 0) ? i : i - TEST_TAB_DIAMETER,
                        (coordinates.z == 0) ? i : i - TEST_TAB_LAYER};
                break;

            case 1:
                _target_index = (v3u32){
                        (coordinates.x == TEST_TAB_DIAMETER - 1) ? i : i + 1,

                        (coordinates.y == TEST_TAB_DIAMETER - 1) ?
                            i : i + TEST_TAB_DIAMETER,

                        (coordinates.z == TEST_TAB_DIAMETER - 1) ?
                            i : i + TEST_TAB_LAYER};
                break;
        }

        switch (AXIS)
        {
            case SHIFT_X:
                mirror_index = _mirror_index.x;
                target_index = _target_index.x;
                break;

            case SHIFT_Y:
                mirror_index = _mirror_index.y;
                target_index = _target_index.y;
                break;

            case SHIFT_Z:
                mirror_index = _mirror_index.z;
                target_index = _target_index.z;
                break;
        }

        test_tab[i] = test_tab[target_index];
        if (test_tab[i] == CHUNK_EDGE)
        {
            test_tab[mirror_index] = CHUNK_SHIFTED;
            test_tab[i] = 0;
        }
    }
}

void
update_camera_movement_test(void)
{
    if (glfwGetMouseButton(render.window,
                GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        lily.camera_distance += (f32)render.mouse_delta.y * 0.3f;
        lily.camera_distance =
            clamp_f32(lily.camera_distance, 10.0f, 100.0f);
    }
    else lily.pitch +=
        (f32)render.mouse_delta.y * settings.mouse_sensitivity;
    lily.yaw += (f32)render.mouse_delta.x * settings.mouse_sensitivity;

    lily.yaw = fmodf(lily.yaw, CAMERA_RANGE_MAX);
    if (lily.yaw < 0.0f)
        lily.yaw += CAMERA_RANGE_MAX;

    lily.pitch = clamp_f32(lily.pitch, -CAMERA_ANGLE_MAX, CAMERA_ANGLE_MAX);

    lily.sin_pitch = sin(lily.pitch * DEG2RAD);
    lily.cos_pitch = cos(lily.pitch * DEG2RAD);
    lily.sin_yaw =   sin(lily.yaw * DEG2RAD);
    lily.cos_yaw =   cos(lily.yaw * DEG2RAD);
    const f32 SPCH = lily.sin_pitch;
    const f32 CPCH = lily.cos_pitch;
    const f32 SYAW = lily.sin_yaw;
    const f32 CYAW = lily.cos_yaw;
    lily.camera.sin_pitch =  SPCH;
    lily.camera.cos_pitch =  CPCH;
    lily.camera.sin_yaw =    SYAW;
    lily.camera.cos_yaw =    CYAW;

    lily.camera.pos =
        (v3f32){
            ((CYAW * CPCH) * lily.camera_distance),
            -((SYAW * CPCH) * lily.camera_distance),
            (SPCH * lily.camera_distance),
        };

    lily.camera.sin_yaw =
        sin((lily.yaw + (CAMERA_RANGE_MAX / 2.0f)) * DEG2RAD);

    lily.camera.cos_yaw =
        cos((lily.yaw + (CAMERA_RANGE_MAX / 2.0f)) * DEG2RAD);
}

void
main__shift_test_tab(void)
{
    lily.camera_distance = 64.0f;
    init_test_tab();

    while (!glfwWindowShouldClose(render.window))
    {
        render.frame_start = glfwGetTime() - game_start_time;
        render.frame_delta = render.frame_start - render.frame_last;
        render.frame_delta_square = pow(render.frame_delta, 2.0f);
        render.frame_last = render.frame_start;
        game_tick = (floor(render.frame_start * 1000)) -
            (SETTING_DAY_TICKS_MAX * game_days);
        if (game_tick >= SETTING_DAY_TICKS_MAX) ++game_days;
        glEnable(GL_DEPTH_TEST);
        update_mouse_movement(&render);
        update_key_states(&render);

        update_camera_movement_test();
        update_camera_perspective(&lily.camera, &projection);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo_skybox.fbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        draw_skybox(&lily);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_world.fbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader_voxel.id);

        glUniformMatrix4fv(uniform.voxel.mat_perspective, 1, GL_FALSE,
                (GLfloat*)&projection.perspective);

        glUniform3fv(uniform.voxel.camera_position, 1,
                (GLfloat*)&lily.camera.pos);

        glUniform3fv(uniform.voxel.sky_color, 1, (GLfloat*)&skybox_data.color);
        glUniform1f(uniform.voxel.opacity, 1.0f);
        v3f32 open_cursor = {-16.5f, -16.5f, -16.5f};
        glUniform3fv(uniform.voxel.open_cursor, 1, (GLfloat*)&open_cursor);

        static u8 direction = 0;
        if (is_key_press(KEY_KP_8))
        {
            direction = 1;
            mark_test_tab(direction);
        }
        else if (is_key_press(KEY_KP_2))
        {
            direction = 2;
            mark_test_tab(direction);
        }
        else if (is_key_press(KEY_KP_4))
        {
            direction = 3;
            mark_test_tab(direction);
        }
        else if (is_key_press(KEY_KP_6))
        {
            direction = 4;
            mark_test_tab(direction);
        }
        else if (is_key_press(KEY_KP_9))
        {
            direction = 5;
            mark_test_tab(direction);
        }
        else if (is_key_press(KEY_KP_3))
        {
            direction = 6;
            mark_test_tab(direction);
        }
        else if (is_key_press(KEY_KP_5))
        {
            direction = 0;
            for (u32 i = 0; i < TEST_TAB_VOLUME; ++i)
            {
                if (!test_tab[i]) continue;
                test_tab[i] = CHUNK_LOADED;
            }
        }

        if (is_key_press(KEY_ENTER))
            shift_test_tab(direction);

        static f32 pulse;
        static v4f32 voxel_color = {0};
        for (u32 i = 0; i < TEST_TAB_VOLUME; ++i)
        {
            if (!test_tab[i]) continue;

            v3f32 offset_cursor =
            {
                floorf(i % 33),
                floorf((i / 33) % 33),
                floorf(i / (33 * 33)),
            };
            glUniform3fv(uniform.voxel.offset_cursor, 1,
                    (GLfloat*)&offset_cursor);

            pulse = (sinf((offset_cursor.y * 0.3f) +
                        (render.frame_start * 5.0f)) * 0.2f) + 0.8f;
            glUniform1f(uniform.voxel.size, pulse);

            switch (test_tab[i])
            {
                default:
                case CHUNK_LOADED:
                    voxel_color = (v4f32){0.3f, 0.15f, 0.03f, 1.0f};
                    break;
                case CHUNK_EDGE:
                    voxel_color = (v4f32){0.37f, 0.48f, 0.04f, 1.0f};
                    break;
                case CHUNK_SHIFTED:
                    voxel_color = (v4f32){0.6f, 0.06f, 0.02f, 1.0f};
                    break;
            }
            glUniform4fv(uniform.voxel.color, 1, (GLfloat*)&voxel_color);
            draw_mesh(&mesh_cube_of_happiness);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, fbo_hud.fbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        draw_hud(&lily);

        static str string[512] = {0};
        start_text(0, FONT_SIZE_DEFAULT, &font_mono_bold,
                &render, &shader_text, &fbo_text, 1);

        snprintf(string, 511, "FPS[%d]", (u32)(1.0f / render.frame_delta));
        push_text(string, (v2f32){MARGIN, MARGIN}, 0, 0);
        render_text(0x6f9f3fff);

        snprintf(string, 511, "PITCH[%.2f] YAW[%.2f]\n", lily.pitch, lily.yaw);
        push_text(string, (v2f32){MARGIN, MARGIN + FONT_SIZE_DEFAULT}, 0, 0);
        render_text(0xffffffff);

        snprintf(string, 511, "MOUSE XY: %.2f %.2f\n" "DELTA XY: %.2f %.2f\n",
                render.mouse_position.x, render.mouse_position.y,
                render.mouse_delta.x, render.mouse_delta.y);
        push_text(string, (v2f32){MARGIN, MARGIN +
                (FONT_SIZE_DEFAULT * 2)}, 0, 0);
        render_text(0x3f6f9fff);

        snprintf(string, 511,
                "CAMERA DISTANCE: %.2f\n"
                "RENDER DISTANCE: %d\n",
                lily.camera_distance,
                (u32)sqrt(render_distance));
        push_text(string, (v2f32){MARGIN, MARGIN +
                (FONT_SIZE_DEFAULT * 4)}, 0, 0);
        render_text(0x3f9f3fff);

        snprintf(string, 511,
                "              : Look Around\n"
                "Hold    :       Zoom and Pan\n"
                "        :       Mark Shift Direction PX\n"
                "        :       Mark Shift Direction NX\n"
                "        :       Mark Shift Direction PY\n"
                "        :       Mark Shift Direction NY\n"
                "        :       Mark Shift Direction PZ\n"
                "        :       Mark Shift Direction NZ\n"
                "     :          Shift Into Marked Direction\n");
        push_text(string, (v2f32){MARGIN, render.size.y - MARGIN -
                (FONT_SIZE_DEFAULT * 9)}, 0, 0);
        render_text(0x845c5cff);

        snprintf(string, 511,
                "MOUSE MOVEMENT\n"
                "     LMB\n"
                "NUMPAD 8\n"
                "NUMPAD 2\n"
                "NUMPAD 4\n"
                "NUMPAD 6\n"
                "NUMPAD 9\n"
                "NUMPAD 3\n"
                "ENTER\n");
        push_text(string, (v2f32){MARGIN, render.size.y - MARGIN -
                (FONT_SIZE_DEFAULT * 9)}, 0, 0);
        render_text(0x9f3f3fff);
        stop_text();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(shader_fbo.id);
        glBindVertexArray(mesh_fbo.vao);
        glDisable(GL_DEPTH_TEST);
        glBindTexture(GL_TEXTURE_2D, fbo_skybox.color_buf);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindTexture(GL_TEXTURE_2D, fbo_world.color_buf);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindTexture(GL_TEXTURE_2D, fbo_hud.color_buf);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindTexture(GL_TEXTURE_2D, fbo_text.color_buf);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glfwSwapBuffers(render.window);
        glfwPollEvents();
    }
}
