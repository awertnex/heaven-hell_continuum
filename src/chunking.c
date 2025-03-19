#include <stdio.h>
#include <string.h>
#include <math.h>
#include "h/chunking.h"

u16 world_height = WORLD_HEIGHT_NORMAL;
chunk chunk_buf[(SETTING_RENDER_DISTANCE_MAX*2) + 1][(SETTING_RENDER_DISTANCE_MAX*2) + 1] = {0};
chunk *target_chunk = 0;
u64 block_count = 0; //debug mode
u64 quad_count = 0; //debug mode

void init_chunking()
{
	memset(&chunk_buf, 0, sizeof(*chunk_buf));
}

//TODO: check chunk barrier faces
void parse_block_state(chunk *chunk, u8 x, u8 y, u16 z)
{
	if (x < CHUNK_SIZE - 1)
		chunk->i[z][y][x + 1] ? (chunk->i[z][y][x + 1] &= ~NEGATIVE_X) : (chunk->i[z][y][x] |= POSITIVE_X);
	else chunk->i[z][y][x] |= POSITIVE_X;

	if (x > 0)
		chunk->i[z][y][x - 1] ? (chunk->i[z][y][x - 1] &= ~POSITIVE_X) : (chunk->i[z][y][x] |= NEGATIVE_X);
	else chunk->i[z][y][x] |= NEGATIVE_X;

	if (y < CHUNK_SIZE - 1)
		chunk->i[z][y + 1][x] ? (chunk->i[z][y + 1][x] &= ~NEGATIVE_Y) : (chunk->i[z][y][x] |= POSITIVE_Y);
	else chunk->i[z][y][x] |= POSITIVE_Y;

	if (y > 0)
		chunk->i[z][y - 1][x] ? (chunk->i[z][y - 1][x] &= ~POSITIVE_Y) : (chunk->i[z][y][x] |= NEGATIVE_Y);
	else chunk->i[z][y][x] |= NEGATIVE_Y;

	if (z < world_height - 1)
		chunk->i[z + 1][y][x] ? (chunk->i[z + 1][y][x] &= ~NEGATIVE_Z) : (chunk->i[z][y][x] |= POSITIVE_Z);
	else chunk->i[z][y][x] |= POSITIVE_Z;
	
	if (z > 0)
		chunk->i[z - 1][y][x] ? (chunk->i[z - 1][y][x] &= ~POSITIVE_Z) : (chunk->i[z][y][x] |= NEGATIVE_Z);
	else chunk->i[z][y][x] |= NEGATIVE_Z;

	chunk->i[z][y][x] |= NOT_EMPTY;

	/*temp
	printf("cxy[%d, %d]\t\tixyz[%d, %d, %d, %d]", chunk->pos.x, chunk->pos.y, chunk->i[z][y][x], x, y, z);
	if (x > 0) printf("\t\tnx[%d]", chunk->i[z][y][x - 1]);
	if (y > 0) printf("\t\tny[%d]", chunk->i[z][y - 1][x]);
	if (z > 0) printf("\t\tnz[%d]", chunk->i[z - 1][y][x]);
	printf("\n");
	*/
}

void parse_chunk_states(chunk *chunk, u16 height)
{
	if (!height) height = world_height; /* temp */
	for (u16 z = 0; z < height; ++z)
		for (u8 y = 0; y < CHUNK_SIZE; ++y)
			for (u8 x = 0; x < CHUNK_SIZE; ++x)
			{
				parse_block_state(chunk, x, y, z);
				if (chunk->i[z][y][x])
					++block_count;
				if (chunk->i[z][y][x] & POSITIVE_X)
					++quad_count;
				if (chunk->i[z][y][x] & NEGATIVE_X)
					++quad_count;
				if (chunk->i[z][y][x] & POSITIVE_Y)
					++quad_count;
				if (chunk->i[z][y][x] & NEGATIVE_Y)
					++quad_count;
				if (chunk->i[z][y][x] & POSITIVE_Z)
					++quad_count;
				if (chunk->i[z][y][x] & NEGATIVE_Z)
					++quad_count;
			}
}

//TODO: revise, might not be needed
chunk *get_chunk(v3i32 *coordinates, u16 *state, u16 flag)
{
	u8 x = 0, y = 0;
	for (; y < setting.render_distance*2; ++y)
	{
		for (; x < setting.render_distance*2; ++x)
		{
			if (chunk_buf[y][x].pos.x == floorf((f32)coordinates->x/CHUNK_SIZE) && chunk_buf[y][x].pos.y == floorf((f32)coordinates->y/CHUNK_SIZE))
			{
				*state |= flag;
				return &chunk_buf[y][x];
			}
		}
		x = 0;
	}
	*state &= ~flag;
	return NULL;
}

void draw_chunk(chunk *chunk, u16 height)
{
	rlPushMatrix();
	rlBegin(RL_QUADS);
	rlTranslatef(chunk->pos.x*CHUNK_SIZE, chunk->pos.y*CHUNK_SIZE, WORLD_BOTTOM);

	for (u16 z = 0; z < height; ++z)
	{
		for (u8 y = 0; y < CHUNK_SIZE; ++y)
		{
			for (u8 x = 0; x < CHUNK_SIZE; ++x)
			{
				if (chunk->i[z][y][x] & BLOCKFACES)
					draw_block(chunk->i[z][y][x]);
				rlTranslatef(1, 0, 0);
			}
			rlTranslatef(-CHUNK_SIZE, 1, 0);
		}
		rlTranslatef(0, -CHUNK_SIZE, 1);
	}

	rlEnd();
	rlPopMatrix();
}

// raylib/rmodels.c/DrawCube refactored
void draw_block(u32 block_state)
{
	if (block_state & POSITIVE_X)
	{
		rlColor4ub(200, 210, 90, opacity);	/*debug mode*/
		rlVertex3f(1.0f, 0.0f, 0.0f);
		rlVertex3f(1.0f, 1.0f, 0.0f);
		rlVertex3f(1.0f, 1.0f, 1.0f);
		rlVertex3f(1.0f, 0.0f, 1.0f);
	}

	if (block_state & NEGATIVE_X)
	{
		rlColor4ub(236, 17, 90, opacity);	/*debug mode*/
		rlVertex3f(0.0f, 0.0f, 0.0f);
		rlVertex3f(0.0f, 0.0f, 1.0f);
		rlVertex3f(0.0f, 1.0f, 1.0f);
		rlVertex3f(0.0f, 1.0f, 0.0f);
	}

	if (block_state & POSITIVE_Y)
	{
		rlColor4ub(200, 248, 246, opacity); /*debug mode*/
		rlVertex3f(0.0f, 1.0f, 0.0f);
		rlVertex3f(0.0f, 1.0f, 1.0f);
		rlVertex3f(1.0f, 1.0f, 1.0f);
		rlVertex3f(1.0f, 1.0f, 0.0f);
	}

	if (block_state & NEGATIVE_Y)
	{
		rlColor4ub(28, 14, 50, opacity);	/*debug mode*/
		rlVertex3f(0.0f, 0.0f, 0.0f);
		rlVertex3f(1.0f, 0.0f, 0.0f);
		rlVertex3f(1.0f, 0.0f, 1.0f);
		rlVertex3f(0.0f, 0.0f, 1.0f);
	}

	if (block_state & POSITIVE_Z)
	{
		rlColor4ub(250, 18, 5, opacity);	/*debug mode*/
		rlVertex3f(0.0f, 0.0f, 1.0f);
		rlVertex3f(1.0f, 0.0f, 1.0f);
		rlVertex3f(1.0f, 1.0f, 1.0f);
		rlVertex3f(0.0f, 1.0f, 1.0f);
	}

	if (block_state & NEGATIVE_Z)
	{
		rlColor4ub(200, 40, 203, opacity);	/*debug mode*/
		rlVertex3f(0.0f, 0.0f, 0.0f);
		rlVertex3f(0.0f, 1.0f, 0.0f);
		rlVertex3f(1.0f, 1.0f, 0.0f);
		rlVertex3f(1.0f, 0.0f, 0.0f);
	}
}

// raylib/rmodels.c/DrawCubeWires refactored
void draw_block_wires(v3i32 *pos)
{
	rlPushMatrix();
	rlTranslatef(pos->x, pos->y, pos->z);
	rlBegin(RL_LINES);
	rlColor4ub(0, 0, 0, 150);

	rlVertex3f(0, 0, 1);
	rlVertex3f(1, 0, 1);

	rlVertex3f(1, 0, 1);
	rlVertex3f(1, 1, 1);

	rlVertex3f(1, 1, 1);
	rlVertex3f(0, 1, 1);

	rlVertex3f(0, 1, 1);
	rlVertex3f(0, 0, 1);

	rlVertex3f(0, 0, 0);
	rlVertex3f(1, 0, 0);

	rlVertex3f(1, 0, 0);
	rlVertex3f(1, 1, 0);

	rlVertex3f(1, 1, 0);
	rlVertex3f(0, 1, 0);

	rlVertex3f(0, 1, 0);
	rlVertex3f(0, 0, 0);

	rlVertex3f(0, 0, 1);
	rlVertex3f(0, 0, 0);

	rlVertex3f(1, 0, 1);
	rlVertex3f(1, 0, 0);

	rlVertex3f(1, 1, 1);
	rlVertex3f(1, 1, 0);

	rlVertex3f(0, 1, 1);
	rlVertex3f(0, 1, 0);

	rlEnd();
	rlPopMatrix();
}

// raylib/rmodels.c/DrawCubeWires refactored
void draw_bounding_box(Vector3 *origin, Vector3 *scl)
{
	rlPushMatrix();
	rlTranslatef(
			origin->x - (scl->x/2),
			origin->y - (scl->y/2),
			origin->z);
	rlBegin(RL_LINES);
	rlColor4ub(255, 200, 220, 255);

	rlVertex3f(0,		0,		scl->z);
	rlVertex3f(scl->x,	0,		scl->z);

	rlVertex3f(scl->x,	0,		scl->z);
	rlVertex3f(scl->x,	scl->y,	scl->z);

	rlVertex3f(scl->x,	scl->y,	scl->z);
	rlVertex3f(0,		scl->y,	scl->z);

	rlVertex3f(0,		scl->y,	scl->z);
	rlVertex3f(0,		0,		scl->z);

	rlVertex3f(0,		0,		0);
	rlVertex3f(scl->x,	0,		0);

	rlVertex3f(scl->x,	0,		0);
	rlVertex3f(scl->x,	scl->y,	0);

	rlVertex3f(scl->x,	scl->y,	0);
	rlVertex3f(0,		scl->y,	0);

	rlVertex3f(0,		scl->y,	0);
	rlVertex3f(0,		0,		0);

	rlVertex3f(0,		0,		scl->z);
	rlVertex3f(0,		0,		0);

	rlVertex3f(scl->x, 	0,		scl->z);
	rlVertex3f(scl->x, 	0,		0);

	rlVertex3f(scl->x, 	scl->y,	scl->z);
	rlVertex3f(scl->x,	scl->y,	0);

	rlVertex3f(0,		scl->y,	scl->z);
	rlVertex3f(0,		scl->y,	0);

	rlEnd();
	rlPopMatrix();
}

// raylib/rmodels.c/DrawLine3D refactored
void draw_line_3d(v3i32 pos_0, v3i32 pos_1, Color color)
{
	rlColor4ub(color.r, color.g, color.b, color.a);
	rlVertex3f(pos_0.x, pos_0.y, pos_0.z);
	rlVertex3f(pos_1.x, pos_1.y, pos_1.z);
}
