/**
 * queue.c - Playlist and queue state management implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include "queue.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define QUEUE_INITIAL_CAPACITY 16

static int queue_rng_seeded = 0;

static char *queue_strdup(const char *src)
{
    if (!src)
        return NULL;

    size_t len = strlen(src);
    char *copy = (char *)malloc(len + 1);
    if (!copy)
        return NULL;

    memcpy(copy, src, len + 1);
    return copy;
}

static int queue_ensure_capacity(Queue *queue, size_t needed)
{
    if (!queue)
        return -1;

    if (needed <= queue->capacity)
        return 0;

    size_t new_capacity = queue->capacity > 0 ? queue->capacity : QUEUE_INITIAL_CAPACITY;
    while (new_capacity < needed)
        new_capacity *= 2;

    char **new_items = (char **)realloc(queue->items, new_capacity * sizeof(char *));
    if (!new_items)
        return -1;

    queue->items = new_items;
    queue->capacity = new_capacity;
    return 0;
}

static void queue_seed_rng_once(void)
{
    if (!queue_rng_seeded)
    {
        srand((unsigned int)time(NULL));
        queue_rng_seeded = 1;
    }
}

static int queue_ensure_visited_capacity(Queue *queue, size_t needed)
{
    if (!queue)
        return -1;

    if (needed == 0)
    {
        free(queue->visited);
        queue->visited = NULL;
        return 0;
    }

    unsigned char *new_visited = (unsigned char *)realloc(queue->visited, needed * sizeof(unsigned char));
    if (!new_visited)
        return -1;

    if (needed > queue->count)
    {
        memset(new_visited + queue->count, 0, needed - queue->count);
    }

    queue->visited = new_visited;
    return 0;
}

static void queue_reset_visited(Queue *queue)
{
    if (!queue || !queue->visited)
        return;

    memset(queue->visited, 0, queue->count);
}

static void queue_mark_current_visited(Queue *queue)
{
    if (!queue || !queue->visited)
        return;

    if (queue->current_index < 0 || (size_t)queue->current_index >= queue->count)
        return;

    queue->visited[queue->current_index] = 1;
}

static void queue_history_clear(Queue *queue)
{
    if (!queue)
        return;

    queue->history_count = 0;
}

static int queue_history_push(Queue *queue, int index)
{
    if (!queue || index < 0)
        return -1;

    if (queue->history_count >= queue->history_capacity)
    {
        size_t new_capacity = queue->history_capacity > 0 ? queue->history_capacity * 2 : QUEUE_INITIAL_CAPACITY;
        int *new_history = (int *)realloc(queue->history, new_capacity * sizeof(int));
        if (!new_history)
            return -1;

        queue->history = new_history;
        queue->history_capacity = new_capacity;
    }

    queue->history[queue->history_count++] = index;
    return 0;
}

static int queue_history_pop(Queue *queue, int *out_index)
{
    if (!queue || !out_index || queue->history_count == 0)
        return -1;

    queue->history_count--;
    *out_index = queue->history[queue->history_count];
    return 0;
}

static int queue_pick_random_unvisited(const Queue *queue, int exclude_current)
{
    if (!queue || !queue->visited || queue->count == 0)
        return -1;

    int chosen = -1;
    int seen = 0;

    for (size_t i = 0; i < queue->count; i++)
    {
        if (queue->visited[i])
            continue;

        if (exclude_current && queue->count > 1 && (int)i == queue->current_index)
            continue;

        seen++;
        if ((rand() % seen) == 0)
        {
            chosen = (int)i;
        }
    }

    return chosen;
}

static QueueNextResult queue_select_next(Queue *queue, int *out_index, int respect_repeat_single)
{
    if (!queue || !out_index)
        return QUEUE_NEXT_ERROR;

    if (queue->count == 0)
        return QUEUE_NEXT_STOP;

    if (queue->current_index < 0 || (size_t)queue->current_index >= queue->count)
        return QUEUE_NEXT_ERROR;

    if (respect_repeat_single && queue->repeat_mode == QUEUE_REPEAT_SINGLE)
    {
        *out_index = queue->current_index;
        return QUEUE_NEXT_PLAY;
    }

    if (!queue->shuffle_enabled)
    {
        if ((size_t)(queue->current_index + 1) < queue->count)
        {
            if (queue_history_push(queue, queue->current_index) != 0)
                return QUEUE_NEXT_ERROR;
            *out_index = queue->current_index + 1;
            return QUEUE_NEXT_PLAY;
        }

        if (queue->repeat_mode == QUEUE_REPEAT_ALL)
        {
            if (queue_history_push(queue, queue->current_index) != 0)
                return QUEUE_NEXT_ERROR;
            *out_index = 0;
            return QUEUE_NEXT_PLAY;
        }

        return QUEUE_NEXT_STOP;
    }

    queue_mark_current_visited(queue);

    int next = queue_pick_random_unvisited(queue, 1);
    if (next >= 0)
    {
        if (queue_history_push(queue, queue->current_index) != 0)
            return QUEUE_NEXT_ERROR;
        *out_index = next;
        return QUEUE_NEXT_PLAY;
    }

    if (queue->repeat_mode == QUEUE_REPEAT_ALL)
    {
        queue_reset_visited(queue);
        queue_mark_current_visited(queue);

        next = queue_pick_random_unvisited(queue, 1);
        if (next >= 0)
        {
            if (queue_history_push(queue, queue->current_index) != 0)
                return QUEUE_NEXT_ERROR;
            *out_index = next;
            return QUEUE_NEXT_PLAY;
        }

        // Single-item queue in repeat-all shuffle mode.
        *out_index = queue->current_index;
        return QUEUE_NEXT_PLAY;
    }

    return QUEUE_NEXT_STOP;
}

static int queue_path_join(const char *folderpath, const char *name, char *out, size_t out_size)
{
    if (!folderpath || !name || !out || out_size == 0)
        return -1;

    size_t folder_len = strlen(folderpath);
    int needs_separator = (folder_len > 0 && folderpath[folder_len - 1] != '/');

    int written = snprintf(out, out_size, "%s%s%s", folderpath, needs_separator ? "/" : "", name);
    if (written < 0 || (size_t)written >= out_size)
        return -1;

    return 0;
}

static int queue_is_regular_file(const char *path)
{
    struct stat st;
    if (!path)
        return 0;
    if (stat(path, &st) != 0)
        return 0;
    return S_ISREG(st.st_mode) ? 1 : 0;
}

static int queue_compare_paths(const void *a, const void *b)
{
    const char *const *path_a = (const char *const *)a;
    const char *const *path_b = (const char *const *)b;
    return strcasecmp(*path_a, *path_b);
}

Queue *queue_create(void)
{
    queue_seed_rng_once();

    Queue *queue = (Queue *)malloc(sizeof(Queue));
    if (!queue)
        return NULL;

    queue->items = NULL;
    queue->count = 0;
    queue->capacity = 0;
    queue->current_index = -1;
    queue->repeat_mode = QUEUE_REPEAT_OFF;
    queue->shuffle_enabled = 0;
    queue->last_played_index = -1;
    queue->visited = NULL;
    queue->history = NULL;
    queue->history_count = 0;
    queue->history_capacity = 0;

    return queue;
}

void queue_clear(Queue *queue)
{
    if (!queue)
        return;

    for (size_t i = 0; i < queue->count; i++)
    {
        free(queue->items[i]);
    }

    queue->count = 0;
    queue->current_index = -1;
    queue->last_played_index = -1;
    free(queue->visited);
    queue->visited = NULL;
    queue_history_clear(queue);
}

void queue_destroy(Queue *queue)
{
    if (!queue)
        return;

    queue_clear(queue);
    free(queue->items);
    free(queue->history);
    free(queue);
}

int queue_is_audio_file(const char *path)
{
    if (!path)
        return 0;

    const char *ext = strrchr(path, '.');
    if (!ext || *(ext + 1) == '\0')
        return 0;

    ext++;

    return strcasecmp(ext, "mp3") == 0 ||
           strcasecmp(ext, "wav") == 0 ||
           strcasecmp(ext, "flac") == 0 ||
           strcasecmp(ext, "m4a") == 0 ||
           strcasecmp(ext, "ogg") == 0 ||
           strcasecmp(ext, "aac") == 0 ||
           strcasecmp(ext, "wma") == 0;
}

int queue_enqueue(Queue *queue, const char *filepath)
{
    if (!queue || !filepath || !queue_is_audio_file(filepath))
        return -1;

    if (queue_ensure_capacity(queue, queue->count + 1) != 0)
        return -1;

    char *copy = queue_strdup(filepath);
    if (!copy)
        return -1;

    queue->items[queue->count++] = copy;

    if (queue_ensure_visited_capacity(queue, queue->count) != 0)
    {
        free(queue->items[queue->count - 1]);
        queue->count--;
        return -1;
    }

    queue->visited[queue->count - 1] = 0;

    if (queue->current_index < 0)
    {
        queue->current_index = 0;
        queue_mark_current_visited(queue);
    }

    return 0;
}

int queue_load_folder(Queue *queue, const char *folderpath)
{
    if (!queue || !folderpath)
        return -1;

    DIR *dir = opendir(folderpath);
    if (!dir)
        return -1;

    size_t found_count = 0;
    size_t found_capacity = QUEUE_INITIAL_CAPACITY;
    char **found = (char **)malloc(found_capacity * sizeof(char *));
    if (!found)
    {
        closedir(dir);
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        const char *name = entry->d_name;

        if (!name || name[0] == '.')
            continue;

        if (!queue_is_audio_file(name))
            continue;

        char full_path[PATH_MAX];
        if (queue_path_join(folderpath, name, full_path, sizeof(full_path)) != 0)
            continue;

        if (!queue_is_regular_file(full_path))
            continue;

        if (found_count >= found_capacity)
        {
            size_t new_capacity = found_capacity * 2;
            char **new_found = (char **)realloc(found, new_capacity * sizeof(char *));
            if (!new_found)
            {
                for (size_t i = 0; i < found_count; i++)
                    free(found[i]);
                free(found);
                closedir(dir);
                return -1;
            }
            found = new_found;
            found_capacity = new_capacity;
        }

        found[found_count] = queue_strdup(full_path);
        if (!found[found_count])
        {
            for (size_t i = 0; i < found_count; i++)
                free(found[i]);
            free(found);
            closedir(dir);
            return -1;
        }
        found_count++;
    }

    closedir(dir);

    qsort(found, found_count, sizeof(char *), queue_compare_paths);

    queue_clear(queue);

    if (queue_ensure_capacity(queue, found_count) != 0)
    {
        for (size_t i = 0; i < found_count; i++)
            free(found[i]);
        free(found);
        return -1;
    }

    for (size_t i = 0; i < found_count; i++)
    {
        queue->items[i] = found[i];
    }

    queue->count = found_count;
    queue->current_index = -1;

    if (queue_ensure_visited_capacity(queue, queue->count) != 0)
    {
        queue_clear(queue);
        return -1;
    }

    queue_reset_visited(queue);

    free(found);

    return (int)found_count;
}

size_t queue_count(const Queue *queue)
{
    return queue ? queue->count : 0;
}

const char *queue_get_item(const Queue *queue, size_t index)
{
    if (!queue || index >= queue->count)
        return NULL;
    return queue->items[index];
}

int queue_get_current_index(const Queue *queue)
{
    return queue ? queue->current_index : -1;
}

const char *queue_get_current_item(const Queue *queue)
{
    if (!queue)
        return NULL;

    if (queue->current_index < 0 || (size_t)queue->current_index >= queue->count)
        return NULL;

    return queue->items[queue->current_index];
}

int queue_set_current_index(Queue *queue, int index)
{
    if (!queue || index < 0 || (size_t)index >= queue->count)
        return -1;

    queue->current_index = index;
    queue->last_played_index = index;
    queue_mark_current_visited(queue);
    return 0;
}

void queue_clear_current(Queue *queue)
{
    if (!queue)
        return;

    if (queue->current_index >= 0 && (size_t)queue->current_index < queue->count)
    {
        queue->last_played_index = queue->current_index;
    }

    queue->current_index = -1;
}

QueueRepeatMode queue_get_repeat_mode(const Queue *queue)
{
    return queue ? queue->repeat_mode : QUEUE_REPEAT_OFF;
}

void queue_set_repeat_mode(Queue *queue, QueueRepeatMode mode)
{
    if (!queue)
        return;

    if (mode < QUEUE_REPEAT_OFF || mode > QUEUE_REPEAT_ALL)
        return;

    queue->repeat_mode = mode;
}

QueueRepeatMode queue_cycle_repeat_mode(Queue *queue)
{
    if (!queue)
        return QUEUE_REPEAT_OFF;

    switch (queue->repeat_mode)
    {
    case QUEUE_REPEAT_OFF:
        queue->repeat_mode = QUEUE_REPEAT_SINGLE;
        break;
    case QUEUE_REPEAT_SINGLE:
        queue->repeat_mode = QUEUE_REPEAT_ALL;
        break;
    case QUEUE_REPEAT_ALL:
    default:
        queue->repeat_mode = QUEUE_REPEAT_OFF;
        break;
    }

    return queue->repeat_mode;
}

int queue_get_shuffle(const Queue *queue)
{
    return queue ? queue->shuffle_enabled : 0;
}

void queue_set_shuffle(Queue *queue, int enabled)
{
    if (!queue)
        return;

    queue->shuffle_enabled = enabled ? 1 : 0;
    queue_history_clear(queue);
    queue_reset_visited(queue);
    queue_mark_current_visited(queue);
}

int queue_toggle_shuffle(Queue *queue)
{
    if (!queue)
        return 0;

    queue_set_shuffle(queue, !queue->shuffle_enabled);
    return queue->shuffle_enabled;
}

QueueNextResult queue_get_next_on_end(Queue *queue, int *out_index)
{
    return queue_select_next(queue, out_index, 1);
}

QueueNextResult queue_get_next_manual(Queue *queue, int *out_index)
{
    return queue_select_next(queue, out_index, 0);
}

QueueNextResult queue_get_previous(Queue *queue, int *out_index)
{
    if (!queue || !out_index)
        return QUEUE_NEXT_ERROR;

    if (queue->count == 0)
        return QUEUE_NEXT_STOP;

    if (queue->current_index < 0)
    {
        if (queue->last_played_index >= 0 && (size_t)queue->last_played_index < queue->count)
        {
            *out_index = queue->last_played_index;

            // If we are restoring playback from ended state in shuffle mode and
            // there is no rewind history left, treat this as a fresh cycle start.
            if (queue->shuffle_enabled && queue->history_count == 0)
            {
                queue_reset_visited(queue);
                if (queue->visited)
                {
                    queue->visited[*out_index] = 1;
                }
            }

            return QUEUE_NEXT_PLAY;
        }
        return QUEUE_NEXT_STOP;
    }

    if ((size_t)queue->current_index >= queue->count)
        return QUEUE_NEXT_ERROR;

    int previous = -1;
    if (queue_history_pop(queue, &previous) == 0)
    {
        *out_index = previous;

        // Reached the beginning of rewind history: make next-track restart from
        // this point in shuffle mode instead of immediately ending.
        if (queue->shuffle_enabled && queue->history_count == 0)
        {
            queue_reset_visited(queue);
            if (queue->visited)
            {
                queue->visited[*out_index] = 1;
            }
        }

        return QUEUE_NEXT_PLAY;
    }

    if (!queue->shuffle_enabled && queue->current_index > 0)
    {
        *out_index = queue->current_index - 1;
        return QUEUE_NEXT_PLAY;
    }

    return QUEUE_NEXT_STOP;
}
