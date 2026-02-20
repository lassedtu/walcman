#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "player.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct
{
    ma_engine engine;
    ma_sound sound;
    int is_initialized;
} PlayerContext;

Player *player_create(void)
{
    Player *player = (Player *)malloc(sizeof(Player));
    if (!player)
        return NULL;

    PlayerContext *ctx = (PlayerContext *)malloc(sizeof(PlayerContext));
    if (!ctx)
    {
        free(player);
        return NULL;
    }

    // Initialize miniaudio engine
    ma_result result = ma_engine_init(NULL, &ctx->engine);
    if (result != MA_SUCCESS)
    {
        fprintf(stderr, "Error: Failed to initialize audio engine\n");
        free(ctx);
        free(player);
        return NULL;
    }

    player->is_playing = 0;
    player->is_paused = 0;
    player->current_file = NULL;
    player->context = ctx;
    ctx->is_initialized = 1;

    return player;
}

int player_play(Player *player, const char *filepath)
{
    if (!player || !filepath)
        return -1;

    PlayerContext *ctx = (PlayerContext *)player->context;
    if (!ctx || !ctx->is_initialized)
        return -1;

    // Stop current sound if playing
    if (player->is_playing)
    {
        ma_sound_stop(&ctx->sound);
        ma_sound_uninit(&ctx->sound);
    }

    // Load and play new sound
    ma_result result = ma_sound_init_from_file(&ctx->engine, filepath, 0, NULL, NULL, &ctx->sound);
    if (result != MA_SUCCESS)
    {
        fprintf(stderr, "Error: Failed to load file: %s\n", filepath);
        return -1;
    }

    result = ma_sound_start(&ctx->sound);
    if (result != MA_SUCCESS)
    {
        fprintf(stderr, "Error: Failed to start playback\n");
        ma_sound_uninit(&ctx->sound);
        return -1;
    }

    player->current_file = filepath;
    player->is_playing = 1;
    player->is_paused = 0;

    return 0;
}

void player_pause(Player *player)
{
    if (!player)
        return;

    PlayerContext *ctx = (PlayerContext *)player->context;
    if (!ctx || !player->is_playing || player->is_paused)
        return;

    ma_sound_set_volume(&ctx->sound, 0.0f);
    player->is_paused = 1;
}

void player_resume(Player *player)
{
    if (!player)
        return;

    PlayerContext *ctx = (PlayerContext *)player->context;
    if (!ctx || !player->is_playing || !player->is_paused)
        return;

    ma_sound_set_volume(&ctx->sound, 1.0f);
    player->is_paused = 0;
}

void player_stop(Player *player)
{
    if (!player)
        return;

    PlayerContext *ctx = (PlayerContext *)player->context;
    if (!ctx || !ctx->is_initialized)
        return;

    if (player->is_playing)
    {
        ma_sound_stop(&ctx->sound);
        ma_sound_uninit(&ctx->sound);
    }

    player->is_playing = 0;
    player->is_paused = 0;
    player->current_file = NULL;
}

void player_destroy(Player *player)
{
    if (!player)
        return;

    player_stop(player);

    PlayerContext *ctx = (PlayerContext *)player->context;
    if (ctx && ctx->is_initialized)
    {
        ma_engine_uninit(&ctx->engine);
        free(ctx);
    }

    free(player);
}

float player_get_position(Player *player)
{
    if (!player || !player->is_playing)
        return 0.0f;

    PlayerContext *ctx = (PlayerContext *)player->context;
    if (!ctx || !ctx->is_initialized)
        return 0.0f;

    float position = 0.0f;
    ma_sound_get_cursor_in_seconds(&ctx->sound, &position);
    return position;
}

float player_get_duration(Player *player)
{
    if (!player || !player->is_playing)
        return 0.0f;

    PlayerContext *ctx = (PlayerContext *)player->context;
    if (!ctx || !ctx->is_initialized)
        return 0.0f;

    float duration = 0.0f;
    ma_sound_get_length_in_seconds(&ctx->sound, &duration);
    return duration;
}

int player_is_at_end(Player *player)
{
    if (!player || !player->is_playing)
        return 0;

    PlayerContext *ctx = (PlayerContext *)player->context;
    if (!ctx || !ctx->is_initialized)
        return 0;

    return ma_sound_at_end(&ctx->sound);
}
