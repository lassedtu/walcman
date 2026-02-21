/**
 * update.h - Automatic update checker and installer
 *
 * This module handles checking for updates from GitHub releases and
 * automatically applying them in the background. All operations are
 * silent - if anything fails (no network, API error, etc), the app
 * continues without updating.
 *
 * Key features:
 * - Spawns background process for update check
 * - Compares versions from GitHub against local VERSION
 * - Downloads binary atomically with rollback capability
 * - Graceful failure handling with no user-facing errors
 */

#ifndef WALCMAN_UPDATE_H
#define WALCMAN_UPDATE_H

/**
 * Check for updates in background
 *
 * This function:
 * 1. Forks a background process (parent returns immediately)
 * 2. Checks GitHub API for latest release
 * 3. Compares with local version
 * 4. Downloads and installs if update available
 * 5. Silently fails if any step errors
 *
 * Returns: 0 on success (fork succeeded), -1 if fork failed
 * Note: Parent process will continue regardless
 */
int update_check_background(void);

#endif // WALCMAN_UPDATE_H
