/**
 * app_controller.c - Application control layer implementation
 */

#include <stdlib.h>
#include "app_controller.h"

static int app_controller_play_current(AppController *controller)
{
    if (!controller || !controller->queue)
        return -1;

    const char *path = queue_get_current_item(controller->queue);
    if (!path)
        return -1;

    player_set_loop(controller->player, 0);
    return player_play(controller->player, path);
}

AppController *app_controller_create(Player *player)
{
    if (!player)
        return NULL;

    AppController *controller = (AppController *)malloc(sizeof(AppController));
    if (!controller)
        return NULL;

    controller->player = player;
    controller->queue = queue_create();
    if (!controller->queue)
    {
        free(controller);
        return NULL;
    }

    return controller;
}

void app_controller_destroy(AppController *controller)
{
    if (!controller)
        return;

    queue_destroy(controller->queue);
    free(controller);
}

const Queue *app_controller_get_queue(const AppController *controller)
{
    return controller ? controller->queue : NULL;
}

int app_controller_play_file_now(AppController *controller, const char *filepath)
{
    if (!controller || !filepath)
        return -1;

    queue_clear(controller->queue);
    if (queue_enqueue(controller->queue, filepath) != 0)
        return -1;

    if (queue_set_current_index(controller->queue, 0) != 0)
        return -1;

    return app_controller_play_current(controller);
}

int app_controller_load_playlist_folder(AppController *controller, const char *folderpath)
{
    if (!controller || !folderpath)
        return -1;

    int loaded = queue_load_folder(controller->queue, folderpath);
    if (loaded <= 0)
        return loaded;

    int start_index = 0;
    if (queue_get_shuffle(controller->queue) && loaded > 1)
    {
        start_index = rand() % loaded;
    }

    if (queue_set_current_index(controller->queue, start_index) != 0)
        return -1;

    if (app_controller_play_current(controller) != 0)
        return -1;

    return loaded;
}

int app_controller_enqueue_file(AppController *controller, const char *filepath)
{
    if (!controller || !filepath)
        return -1;

    if (queue_enqueue(controller->queue, filepath) != 0)
        return -1;

    if (!controller->player->is_playing)
    {
        int current = queue_get_current_index(controller->queue);
        if (current < 0)
        {
            if (queue_set_current_index(controller->queue, 0) != 0)
                return -1;
        }
        if (app_controller_play_current(controller) != 0)
            return -1;
    }

    return 0;
}

int app_controller_handle_track_end(AppController *controller)
{
    if (!controller)
        return -1;

    Queue *queue = controller->queue;
    if (!queue || queue_count(queue) == 0)
    {
        player_stop(controller->player);
        queue_clear_current(controller->queue);
        return 0;
    }

    int next_index = -1;
    QueueNextResult next_result = queue_get_next_on_end(queue, &next_index);

    if (next_result == QUEUE_NEXT_ERROR)
    {
        player_stop(controller->player);
        queue_clear_current(controller->queue);
        return -1;
    }

    if (next_result == QUEUE_NEXT_STOP)
    {
        player_stop(controller->player);
        queue_clear_current(controller->queue);
        return 0;
    }

    if (queue_set_current_index(controller->queue, next_index) != 0)
    {
        player_stop(controller->player);
        queue_clear_current(controller->queue);
        return -1;
    }

    if (app_controller_play_current(controller) != 0)
    {
        player_stop(controller->player);
        queue_clear_current(controller->queue);
        return -1;
    }

    return 1;
}

int app_controller_play_next(AppController *controller)
{
    if (!controller || !controller->queue)
        return -1;

    int next_index = -1;
    QueueNextResult next_result = queue_get_next_manual(controller->queue, &next_index);

    if (next_result == QUEUE_NEXT_ERROR)
        return -1;
    if (next_result == QUEUE_NEXT_STOP)
    {
        player_stop(controller->player);
        queue_clear_current(controller->queue);
        return 0;
    }

    if (queue_set_current_index(controller->queue, next_index) != 0)
        return -1;

    if (app_controller_play_current(controller) != 0)
        return -1;

    return 1;
}

int app_controller_play_previous(AppController *controller)
{
    if (!controller || !controller->queue)
        return -1;

    int previous_index = -1;
    QueueNextResult prev_result = queue_get_previous(controller->queue, &previous_index);

    if (prev_result == QUEUE_NEXT_ERROR)
        return -1;
    if (prev_result == QUEUE_NEXT_STOP)
        return 0;

    if (queue_set_current_index(controller->queue, previous_index) != 0)
        return -1;

    if (app_controller_play_current(controller) != 0)
        return -1;

    return 1;
}

int app_controller_toggle_shuffle(AppController *controller)
{
    if (!controller || !controller->queue)
        return 0;

    return queue_toggle_shuffle(controller->queue);
}

int app_controller_get_shuffle(const AppController *controller)
{
    if (!controller || !controller->queue)
        return 0;

    return queue_get_shuffle(controller->queue);
}

QueueRepeatMode app_controller_cycle_repeat(AppController *controller)
{
    if (!controller)
        return QUEUE_REPEAT_OFF;

    // Repeat mode is always controller/queue-driven.
    // Keep low-level sound looping disabled; replay is handled on track end.
    player_set_loop(controller->player, 0);
    return queue_cycle_repeat_mode(controller->queue);
}

const char *app_controller_get_repeat_label(const AppController *controller)
{
    if (!controller || !controller->queue)
        return "Off";

    switch (queue_get_repeat_mode(controller->queue))
    {
    case QUEUE_REPEAT_SINGLE:
        return "Song";
    case QUEUE_REPEAT_ALL:
        return "Playlist";
    case QUEUE_REPEAT_OFF:
    default:
        return "Off";
    }
}

const char *app_controller_get_repeat_symbol(const AppController *controller)
{
    if (!controller || !controller->queue)
        return "⇾";

    switch (queue_get_repeat_mode(controller->queue))
    {
    case QUEUE_REPEAT_SINGLE:
        return "↺1";
    case QUEUE_REPEAT_ALL:
        return "↺";
    case QUEUE_REPEAT_OFF:
    default:
        return "⇾";
    }
}

const char *app_controller_get_shuffle_symbol(const AppController *controller)
{
    if (!controller || !controller->queue)
        return "-";

    return queue_get_shuffle(controller->queue) ? "~" : "-";
}
