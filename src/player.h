/**
 * player.h - Audio playback engine interface
 *
 * Provides a high-level wrapper around the miniaudio library for:
 * - Playing audio files (MP3, WAV, FLAC, M4A, OGG, AAC, WMA)
 * - Playback control (play, pause, resume, stop)
 * - State management and position tracking
 *
 * The Player struct maintains playback state while PlayerContext
 * (internal) wraps the actual miniaudio engine.
 */

#ifndef WALCMAN_PLAYER_H
#define WALCMAN_PLAYER_H

// Playback state enum
typedef enum
{
    STATE_STOPPED, // No audio playing
    STATE_PLAYING, // Audio actively playing
    STATE_PAUSED   // Playback paused
} PlayerState;

// Audio player instance
typedef struct Player
{
    int is_playing;           // 1 if audio is loaded and playing/paused
    int is_paused;            // 1 if currently paused
    const char *current_file; // Path to currently loaded file
    void *audio_context;      // Opaque pointer to miniaudio context
} Player;

/**
 * Create a new audio player instance
 * Returns: Player pointer on success, NULL on failure
 */
Player *player_create(void);

/**
 * Load and play an audio file
 * player: Player instance
 * filepath: Path to audio file
 * Returns: 0 on success, -1 on failure
 */
int player_play(Player *player, const char *filepath);

/**
 * Pause playback (can be resumed)
 * player: Player instance
 */
void player_pause(Player *player);

/**
 * Resume paused playback
 * player: Player instance
 */
void player_resume(Player *player);

/**
 * Stop playback and unload audio
 * player: Player instance
 */
void player_stop(Player *player);

/**
 * Destroy player and free resources
 * player: Player instance
 */
void player_destroy(Player *player);

/**
 * Get current playback position in seconds
 * player: Player instance
 * Returns: Position in seconds, or 0.0 if not playing
 */
float player_get_position(Player *player);

/**
 * Get total duration of current audio in seconds
 * player: Player instance
 * Returns: Duration in seconds, or 0.0 if not available
 */
float player_get_duration(Player *player);

/**
 * Check if current audio has finished playing
 * player: Player instance
 * Returns: 1 if finished, 0 otherwise
 */
int player_has_finished(Player *player);

/**
 * Get current playback state
 * player: Player instance
 * Returns: PlayerState enum value
 */
PlayerState player_get_state(Player *player);

/**
 * Get path to currently loaded file
 * player: Player instance
 * Returns: File path string, or NULL if nothing loaded
 */
const char *player_get_current_file(Player *player);

#endif // WALCMAN_PLAYER_H
