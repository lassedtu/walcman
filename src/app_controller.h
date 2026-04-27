/**
 * app_controller.h - Application control layer
 *
 * Coordinates playback, queue, and repeat behavior.
 */

#ifndef WALCMAN_APP_CONTROLLER_H
#define WALCMAN_APP_CONTROLLER_H

#include "player.h"
#include "queue.h"

typedef struct AppController
{
    Player *player;
    Queue *queue;
} AppController;

/**
 * Create/destroy app controller.
 * The player instance is owned by caller.
 */
AppController *app_controller_create(Player *player);
void app_controller_destroy(AppController *controller);

/**
 * Access queue state.
 */
const Queue *app_controller_get_queue(const AppController *controller);

/**
 * Start playback immediately with a single file and reset queue to that file.
 * Returns 0 on success, -1 on failure.
 */
int app_controller_play_file_now(AppController *controller, const char *filepath);

/**
 * Load queue from folder and start playback from first track.
 * Returns number of loaded tracks on success, -1 on failure.
 */
int app_controller_load_playlist_folder(AppController *controller, const char *folderpath);

/**
 * Add one file to queue.
 * If nothing is currently playing, starts playback from first queued item.
 * Returns 0 on success, -1 on failure.
 */
int app_controller_enqueue_file(AppController *controller, const char *filepath);

/**
 * Handle track-end transition according to queue repeat mode.
 * Returns 1 if playback continues with another track, 0 if playback stops,
 * and -1 on error.
 */
int app_controller_handle_track_end(AppController *controller);

/**
 * Move to next queue item manually.
 * Returns 1 if moved and started playback, 0 if queue ended and playback stopped, -1 on error.
 */
int app_controller_play_next(AppController *controller);

/**
 * Move to previous track.
 * Returns 1 if moved and started playback, 0 if no previous track, -1 on error.
 */
int app_controller_play_previous(AppController *controller);

/**
 * Toggle shuffle mode.
 * Returns 1 when shuffle is enabled, 0 when disabled.
 */
int app_controller_toggle_shuffle(AppController *controller);

/**
 * Get shuffle mode state.
 */
int app_controller_get_shuffle(const AppController *controller);

/**
 * Cycle repeat behavior.
 * OFF -> SINGLE -> ALL -> OFF.
 * Returns resulting queue repeat mode.
 */
QueueRepeatMode app_controller_cycle_repeat(AppController *controller);

/**
 * Get repeat label for display.
 */
const char *app_controller_get_repeat_label(const AppController *controller);

/**
 * Get repeat symbol for compact display.
 * Off: "⇾", Playlist: "↺", Song: "↺1"
 */
const char *app_controller_get_repeat_symbol(const AppController *controller);

/**
 * Get shuffle symbol for compact display.
 * Off: "-", On: "~"
 */
const char *app_controller_get_shuffle_symbol(const AppController *controller);

#endif // WALCMAN_APP_CONTROLLER_H
