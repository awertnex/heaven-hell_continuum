#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <raylib.h>
#include <rlgl.h>

#define VECTOR2_TYPES
#include <defines.h>

#define LOG(x, ...) memset(logger, 0, 255); \
    snprintf(logger, 18, "%s: ", log_level[x]); \
    strncat(logger, __VA_ARGS__, 255); \
    printf("  %s\n", logger);

u8 active = 1;
u8 text_vertical_spacing = 25;
u8 font_size = 18;
str logger[256] = {0};
str log_level[3][16] = 
{
    "ERROR",
    "WARNING",
    "INFO",
};
enum LogLevel
{
    ERROR = 0,
    WARNING = 1,
    INFO = 2,
}; /* LogLevel */

// -----------------------------------------------------------------------------
FILE *save_file;
str tokens_default[10][255] =
{
    "FF",
    "FF",
    "FFFFFF",
};
str tokens_loaded[10][255] = {0};

DIR *dir;
struct dirent *drnt;
str files[32][NAME_MAX] = {0};
u16 file_count;

typedef struct Chunk
{
    u8 loaded;
    v2i16 pos;
    u8 i[420][64][64];
    u16 id[420][64][64];
} Chunk;

// -----------------------------------------------------------------------------
void tokenize_chunk();
void update_chunk();
void update_chunk_directory();
void unload_chunk();
void input();
void gui();

void load_chunk()
{
    save_file = fopen("chunk/chunk_00_00", "r");
    if (save_file)
    {
        tokenize_chunk();
        fclose(save_file);
    }
    else
    {
        LOG(INFO, "Chunk created: chunk_00_00");
        update_chunk();
    }

}

void tokenize_chunk()
{
    struct stat buf;
    stat("chunk/chunk_00_00", &buf);

    if (buf.st_size < 1)
    {
        LOG(ERROR, "Chunk file empty");
        return;
    }

    str *save_file_contents = (str*)malloc(sizeof(Chunk));
    for (u32 i = 0; i < sizeof(Chunk) /* (2*5) + ((64*64*420)*6) */ && i < (u64)buf.st_size; ++i)
        save_file_contents[i] = getc(save_file);

    u8 stage = 1, parse = 1;
    for (u64 i = 0, c = 0; i < (u64)buf.st_size; ++i)
    {
        switch (stage)
        {
            case 1:
                for (u8 j = 0; j < strlen(tokens_default[stage - 1]); ++j ,++c)
                {
                    if (save_file_contents[c] != tokens_default[stage - 1][j])
                    {
                        parse = 0;
                        break;
                    }
                }
                if (parse)
                    snprintf(tokens_loaded[stage - 1], strlen(tokens_default[stage - 1]), "%s", tokens_default[stage - 1]);
                stage = 2;
                break;

            case 2:
                for (u8 j = 0; j < strlen(tokens_default[stage - 1]); ++j ,++c)
                {
                    if (save_file_contents[c] == '\n') ++c;
                    if (save_file_contents[c] != tokens_default[stage - 1][j])
                    {
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

        if (!parse)
        {
            LOG(ERROR, "Chunk file corrupted, process terminated");
            free(save_file_contents);
            return;
        }
    }

    LOG(INFO, "Chunk loaded: chunk_00_00");
    printf("  CONTENTS: %s\n", save_file_contents);
    free(save_file_contents);
}

void update_chunk()
{
    save_file = fopen("chunk/chunk_00_00", "w");
    fwrite("0x00,0x00\n", 10, 1, save_file);
    fclose(save_file);
}

void update_chunk_directory()
{
    dir = opendir("chunk/");
    if (dir)
    {
        file_count = 0;
        for (u16 i = 0; i < 264 /*TODO: FILES_MAX*/ && files[i][0]; ++i)
            memset(files[i], 0, NAME_MAX);

        while ((drnt = readdir(dir)))
        {
            snprintf(files[file_count], NAME_MAX + 1, "%s", drnt->d_name);
            ++file_count;
        }

        closedir(dir);
    }
}

void unload_chunk()
{
}

// -----------------------------------------------------------------------------
int main(void)
{
    InitWindow(1280, 720, "test: chunk_loader");
    printf("Chunk Size: %ld\n", sizeof(Chunk));
    SetTargetFPS(60);

    while (active)
    {
        input();
        gui();
    }

    unload_chunk();
    CloseWindow();
    return 0;
}

void input()
{
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

void gui()
{
    BeginDrawing();
    ClearBackground(DARKGRAY);

    DrawText("Controls:", 10, 10, font_size, RAYWHITE);
    DrawText("Loaded Chunk:", 500, 10, font_size, RAYWHITE);
    DrawText("Directory:", 1100, 10, font_size, RAYWHITE);
    DrawText("1: read chunk directory", 10, 10 + text_vertical_spacing, font_size, RAYWHITE);
    DrawText("2: load chunk file", 10, 10 + (text_vertical_spacing*2), font_size, RAYWHITE);
    DrawText("3: read chunk file", 10, 10 + (text_vertical_spacing*3), font_size, RAYWHITE);
    DrawText("4: unload chunk", 10, 10 + (text_vertical_spacing*4), font_size, RAYWHITE);

    for (u8 i = 0; i < 10; ++i)
        DrawText(tokens_loaded[i], 500, 10 + ((i + 1)*text_vertical_spacing), font_size, RAYWHITE);
    for (u16 i = 0; i < 32; ++i)
        DrawText(files[i], 1100, 10 + ((i + 1)*text_vertical_spacing), font_size, RAYWHITE);

    rlBegin(RL_QUADS);

    DrawRectangle(0, 670, 1280, 50, (Color){70, 70, 70, 255});
    rlEnd();
    DrawText(logger, 20, 690, font_size, (Color){245, 185, 185, 255});

    EndDrawing();
}
