/**
 * queue.h - Playlist and queue state management
 *
 * Provides a shared queue model used for both folder-based playlists and
 * ad-hoc queued songs.
 */

#ifndef WALCMAN_QUEUE_H
#define WALCMAN_QUEUE_H

#include <stddef.h>

// Repeat behavior for queued playback.
typedef enum
{
    QUEUE_REPEAT_OFF = 0,
    QUEUE_REPEAT_SINGLE,
    QUEUE_REPEAT_ALL
} QueueRepeatMode;

// Result for deciding what to play when a track ends.
typedef enum
{
    QUEUE_NEXT_ERROR = -1,
    QUEUE_NEXT_STOP = 0,
    QUEUE_NEXT_PLAY = 1
} QueueNextResult;

typedef struct Queue
{
    char **items;
    size_t count;
    size_t capacity;
    int current_index;
    QueueRepeatMode repeat_mode;
    int shuffle_enabled;
    int last_played_index;
    unsigned char *visited;
    int *history;
    size_t history_count;
    size_t history_capacity;
} Queue;

/**
 * Create/destroy queue state.
 */
Queue *queue_create(void);
void queue_destroy(Queue *queue);

/**
 * Remove all queued items and reset state.
 */
void queue_clear(Queue *queue);

/**
 * Add one file path to the queue.
 * Returns 0 on success, -1 on failure.
 */
int queue_enqueue(Queue *queue, const char *filepath);

/**
 * Replace queue contents with playable files from folder.
 * Files are added in deterministic (alphabetical) order.
 * Returns number of loaded files, or -1 on failure.
 */
int queue_load_folder(Queue *queue, const char *folderpath);

/**
 * Read-only queue accessors.
 */
size_t queue_count(const Queue *queue);
const char *queue_get_item(const Queue *queue, size_t index);
int queue_get_current_index(const Queue *queue);
const char *queue_get_current_item(const Queue *queue);

/**
 * Set the current index to a valid queue position.
 * Returns 0 on success, -1 on failure.
 */
int queue_set_current_index(Queue *queue, int index);

/**
 * Clear current track selection (no active track).
 */
void queue_clear_current(Queue *queue);

/**
 * Repeat mode controls.
 */
QueueRepeatMode queue_get_repeat_mode(const Queue *queue);
void queue_set_repeat_mode(Queue *queue, QueueRepeatMode mode);
QueueRepeatMode queue_cycle_repeat_mode(Queue *queue);

/**
 * Shuffle controls.
 */
int queue_get_shuffle(const Queue *queue);
int queue_toggle_shuffle(Queue *queue);
void queue_set_shuffle(Queue *queue, int enabled);

/**
 * Decide what index to play after current item ends.
 * Returns QUEUE_NEXT_PLAY and sets out_index when another item should play.
 * Returns QUEUE_NEXT_STOP when playback should stop.
 * Returns QUEUE_NEXT_ERROR for invalid arguments/state.
 */
QueueNextResult queue_get_next_on_end(Queue *queue, int *out_index);

/**
 * Decide what index to play for manual next-track action.
 * Returns QUEUE_NEXT_PLAY and sets out_index when another item should play.
 * Returns QUEUE_NEXT_STOP when no further track exists.
 * Returns QUEUE_NEXT_ERROR for invalid arguments/state.
 */
QueueNextResult queue_get_next_manual(Queue *queue, int *out_index);

/**
 * Decide what index to play for previous-track action.
 * Returns QUEUE_NEXT_PLAY and sets out_index when previous exists.
 * Returns QUEUE_NEXT_STOP when no previous track exists.
 * Returns QUEUE_NEXT_ERROR for invalid arguments/state.
 */
QueueNextResult queue_get_previous(Queue *queue, int *out_index);

/**
 * Helper for file validation based on supported extensions.
 * Returns 1 if file extension is recognized audio type, 0 otherwise.
 */
int queue_is_audio_file(const char *path);

#endif // WALCMAN_QUEUE_H
