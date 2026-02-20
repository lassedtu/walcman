#ifndef WALCMAN_PLAYER_H
#define WALCMAN_PLAYER_H

typedef struct
{
    // Player state
    int is_playing;
    int is_paused;
    const char *current_file;
    void *context; // Opaque pointer to miniaudio context
} Player;

// Initialize player
Player *player_create(void);

// Load and play audio file
int player_play(Player *player, const char *filepath);

// Pause playback
void player_pause(Player *player);

// Resume playback
void player_resume(Player *player);

// Stop playback
void player_stop(Player *player);

// Cleanup
void player_destroy(Player *player);

// Get current playback position in seconds
float player_get_position(Player *player);

// Get total duration in seconds
float player_get_duration(Player *player);

// Check if playback has ended
int player_is_at_end(Player *player);

#endif // WALCMAN_PLAYER_H
