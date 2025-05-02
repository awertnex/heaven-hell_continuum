#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdarg.h>
#include <sys/stat.h>

#include "../dependencies/raylib-5.5/src/raylib.h"
#include "../dependencies/raylib-5.5/src/rlgl.h"

#define VECTOR2_TYPES
#include "../engine/h/defines.h"

#define NAME_MAX         255	/* # chars in a file name */
#define PATH_MAX        4096	/* # chars in a path name including nul */

#define LOG(x, ...) memset(logger, 0, 255); \
    snprintf(logger, 18, "%s: ", log_level[x]); \
    strncat(logger, __VA_ARGS__, 255); \
    printf("  %s\n", logger);

u8 active = 1;
u8 text_vertical_spacing = 25;
u8 fontSize = 18;
str logger[256] = {0};
str log_level[3][16] = {
    "ERROR",
    "WARNING",
    "INFO",
};
enum LogLevel {
    ERROR = 0,
    WARNING = 1,
    INFO = 2,
}; /* LogLevel */

// -----------------------------------------------------------------------------
str* mc_c_grandpath;
str* mc_c_subpath;
str* mc_c_chunkpath;
str tokens_default[10][8] = {
    "4",    // bytes for chunk->pos;
    "1",    // bytes for each chunk->i;
    "2",    // bytes for each chunk->info;
    "x",    // formatter: repeat 'bytes to the left of x' a 'bytes to the right of x' times;
};

str** tokens_loaded;

FILE *save_file;
DIR *dir;
struct dirent *drnt;
str files[32][NAME_MAX + 1] = {0};
u16 file_count;

typedef struct Chunk {
    v2i16 pos;
    u8 i[4][4][4];
    u16 info[4][4][4];
    u8 loaded;
} Chunk;
Chunk chunk = {
    .pos = {-1, 4095},
    .i = {3, 35, 45, 34, 8, 11, 30},
    .info = {0, 0, 0, 0, 4095, 0, 0},
    .loaded = 1,
};

// -----------------------------------------------------------------------------
void tokenize_chunk();
void update_chunk();
void update_chunk_directory();
void unload_chunk();
void input();
void gui();

void load_chunk() {
    save_file = fopen(files[0], "rb");
    if (save_file) {
        tokenize_chunk();
        fclose(save_file);
        LOG(INFO, "Chunk Loaded 'chunk'");
    } else {
        LOG(INFO, "Chunk Created 'chunk'");
        update_chunk();
    }

}

void tokenize_chunk() {
    struct stat buf;
    stat(files[0], &buf);

    if (buf.st_size < 1) {
        LOG(ERROR, "Chunk file empty");
        return;
    }

    str *save_file_contents = (str*)malloc(sizeof(Chunk));
    for (u32 i = 0; i < sizeof(Chunk) && i < (u64)buf.st_size; ++i)
        save_file_contents[i] = getc(save_file);

    u8 stage = 1, parse = 1;
    for (u64 i = 0, c = 0; i < (u64)buf.st_size; ++i) {
        switch (stage) {
            case 1:
                for (u8 j = 0; j < strlen(tokens_default[stage - 1]); ++j ,++c) {
                    if (save_file_contents[c] != tokens_default[stage - 1][j]) {
                        parse = 0;
                        break;
                    }
                }
                if (parse)
                    snprintf(tokens_loaded[stage - 1], strlen(tokens_default[stage - 1]), "%s", tokens_default[stage - 1]);
                stage = 2;
                break;

            case 2:
                for (u8 j = 0; j < strlen(tokens_default[stage - 1]); ++j ,++c) {
                    if (save_file_contents[c] == '\n') ++c;
                    if (save_file_contents[c] != tokens_default[stage - 1][j]) {
                        parse = 0;
                        active = 0;
                        break;
                    }
                }
                if (parse)
                    snprintf(tokens_loaded[stage - 1], strlen(tokens_default[stage - 1]), "%s", tokens_default[stage - 1]);
                stage = 3;
                break;

            case 3:
                stage = 4;
                break;

            case 4:
                stage = 5;
                break;

            case 5:
                stage = 0;
                break;
        }

        if (!parse) {
            LOG(ERROR, "Chunk file corrupted, process terminated");
            free(save_file_contents);
            return;
        }
    }

    LOG(INFO, "Chunk Loaded 'chunk'");
    printf("  CONTENTS: %s\n", save_file_contents);
    free(save_file_contents);
}

void update_chunk() {
    save_file = fopen(files[0], "wb");
    fwrite(&chunk, sizeof(chunk), 1, save_file);
    fclose(save_file);
    return;
    printf("tokens loaded: %s\n", tokens_loaded[0]);
    save_file = fopen(files[0], "rb");
    fread(tokens_loaded[0], sizeof(tokens_loaded[0]), 1, save_file);
    fclose(save_file);
    printf("tokens loaded: %s\n", tokens_loaded[0]);
}

void update_chunk_directory() {
    dir = opendir(mc_c_chunkpath);
    if (dir) {
        file_count = 0;
        for (u16 i = 0; i < 264 && files[i][0]; ++i)
            memset(files[i], 0, NAME_MAX);

        while ((drnt = readdir(dir))) {
            snprintf(files[file_count], NAME_MAX, "%s%s", mc_c_chunkpath, drnt->d_name);
            ++file_count;
        }

        closedir(dir);
    }
}

void unload_chunk() {
}

int main(void) {
    mc_c_grandpath = (char*) malloc(PATH_MAX);
    mc_c_subpath = (char*) malloc(PATH_MAX);
    mc_c_chunkpath = (char*) malloc(PATH_MAX);
    tokens_loaded = (char**) calloc(10, 8);

    memset(mc_c_grandpath, 0, PATH_MAX);
    memset(mc_c_subpath, 0, PATH_MAX);
    memset(mc_c_chunkpath, 0, PATH_MAX);
    memset(tokens_loaded, 0, 80);

    snprintf(mc_c_grandpath, 64, "%s/minecraft.c/", getenv("HOME"));
    snprintf(mc_c_subpath, 128, "%stest_instance/", mc_c_grandpath);
    snprintf(mc_c_chunkpath, 256, "%schunk/", mc_c_subpath);

    mc_mkdir(mc_c_grandpath, 0775);
    mc_mkdir(mc_c_subpath, 0775);
    mc_mkdir(mc_c_chunkpath, 0775);

    InitWindow(1280, 720, "test: chunk_loader");
    SetTargetFPS(60);

    while (active) {
        input();
        gui();
    }

    free(mc_c_grandpath);
    free(mc_c_subpath);
    free(mc_c_chunkpath);
    free(tokens_loaded);
    unload_chunk();
    CloseWindow();
    return 0;
}

void input() {
    if (IsKeyPressed(KEY_Q))
        active = 0;

    if (IsKeyPressed(KEY_ONE))
        update_chunk_directory();

    if (IsKeyPressed(KEY_TWO))
        load_chunk();

    if (IsKeyPressed(KEY_THREE))
        update_chunk();

    if (IsKeyPressed(KEY_FOUR))
        unload_chunk();
}

void gui() {
    BeginDrawing();
    ClearBackground(DARKGRAY);

    DrawText("Controls:", 10, 10, fontSize, RAYWHITE);
    DrawText("Loaded Chunk:", 500, 10, fontSize, RAYWHITE);
    DrawText("Directory:", 1100, 10, fontSize, RAYWHITE);
    DrawText("1: read chunk directory", 10, 10 + text_vertical_spacing, fontSize, RAYWHITE);
    DrawText("2: load & read chunk file", 10, 10 + (text_vertical_spacing*2), fontSize, RAYWHITE);
    DrawText("3: read chunk file", 10, 10 + (text_vertical_spacing*3), fontSize, RAYWHITE);
    DrawText("4: unload chunk", 10, 10 + (text_vertical_spacing*4), fontSize, RAYWHITE);

    for (u8 i = 0; i < 10; ++i)
        DrawText(tokens_loaded[i], 500, 10 + ((i + 1)*text_vertical_spacing), fontSize, RAYWHITE);
    for (u16 i = 0; i < 32; ++i)
        DrawText(files[i], 1100, 10 + ((i + 1)*text_vertical_spacing), fontSize, RAYWHITE);

    rlBegin(RL_QUADS);

    DrawRectangle(0, 670, 1280, 50, (Color){70, 70, 70, 255});
    rlEnd();
    DrawText(logger, 20, 690, fontSize, (Color){245, 185, 185, 255});

    EndDrawing();
}

