/**
 * update.c - Implementation of automatic update system
 *
 * Handles version checking and binary updates from GitHub releases
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "update.h"

/* Forward declarations */
static void update_debug_log(const char *message);
static char *get_local_version(void);
static char *parse_github_version(const char *json_response);
static char *parse_github_download_url(const char *json_response);
static int download_file(const char *url, const char *output_path);
static int install_binary(const char *new_binary_path);
static int update_version_file(const char *new_version);
static char *fetch_github_release(void);
static int is_update_enabled(void);
static time_t get_check_interval(void);
static int should_check_for_updates(void);
static void update_check_timestamp(void);
static void update_worker(void);

#define UPDATE_GITHUB_API "https://api.github.com/repos/lassedtu/walcman/releases/latest"
#define UPDATE_TIMEOUT 5
#define UPDATE_MAX_VERSION_LEN 50
#define UPDATE_TEMP_BINARY "/tmp/walcman.update"
#define UPDATE_BACKUP_BINARY ".backup"
#define UPDATE_CONFIG_FILE "/.config/walcman/config"
#define UPDATE_LAST_CHECK_FILE "/.config/walcman/.last_check"

/**
 * Extract version from VERSION file
 * Returns version string (must be freed by caller), or NULL on error
 */
static char *get_local_version(void)
{
    const char *install_dir = getenv("HOME");
    if (!install_dir)
        return NULL;

    char version_path[512];
    snprintf(version_path, sizeof(version_path), "%s/.config/walcman/VERSION", install_dir);

    FILE *f = fopen(version_path, "r");
    if (!f)
        return NULL;

    char *version = malloc(UPDATE_MAX_VERSION_LEN);
    if (!version)
    {
        fclose(f);
        return NULL;
    }

    if (fgets(version, UPDATE_MAX_VERSION_LEN, f) == NULL)
    {
        fclose(f);
        free(version);
        return NULL;
    }

    fclose(f);

    // Strip newline
    size_t len = strlen(version);
    if (len > 0 && version[len - 1] == '\n')
        version[len - 1] = '\0';

    return version;
}

/**
 * Read config file and check if updates are enabled
 * Returns 1 if enabled, 0 if disabled, -1 if config not found
 */
static int is_update_enabled(void)
{
    const char *home = getenv("HOME");
    if (!home)
        return -1;

    char config_path[512];
    snprintf(config_path, sizeof(config_path), "%s%s", home, UPDATE_CONFIG_FILE);

    FILE *f = fopen(config_path, "r");
    if (!f)
        return -1; // Config file doesn't exist, assume enabled for first run

    char line[256];
    int enabled = 1; // Default to enabled

    while (fgets(line, sizeof(line), f))
    {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n')
            continue;

        // Check for update_check_enabled setting
        if (strncmp(line, "update_check_enabled=", 20) == 0)
        {
            const char *value = line + 20;
            if (*value == '0')
                enabled = 0;
            else if (*value == '1')
                enabled = 1;
            break;
        }
    }

    fclose(f);
    return enabled;
}

/**
 * Get check interval from config file (in hours)
 * Returns interval in seconds, or 86400 (24 hours) if not found
 */
static time_t get_check_interval(void)
{
    const char *home = getenv("HOME");
    if (!home)
        return 86400; // Default: 24 hours

    char config_path[512];
    snprintf(config_path, sizeof(config_path), "%s%s", home, UPDATE_CONFIG_FILE);

    FILE *f = fopen(config_path, "r");
    if (!f)
        return 86400;

    char line[256];
    time_t interval = 86400; // Default: 24 hours

    while (fgets(line, sizeof(line), f))
    {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n')
            continue;

        // Check for check_interval_hours setting
        if (strncmp(line, "check_interval_hours=", 21) == 0)
        {
            int hours = atoi(line + 21);
            if (hours > 0)
                interval = (time_t)hours * 3600; // Convert hours to seconds
            break;
        }
    }

    fclose(f);
    return interval;
}

/**
 * Check if enough time has passed since last check
 * Returns 1 if check should run, 0 if throttled
 */
static int should_check_for_updates(void)
{
    const char *home = getenv("HOME");
    if (!home)
        return 1; // If we can't get home, do the check

    char last_check_path[512];
    snprintf(last_check_path, sizeof(last_check_path), "%s%s", home, UPDATE_LAST_CHECK_FILE);

    // Try to read last check time
    FILE *f = fopen(last_check_path, "r");
    if (!f)
        return 1; // File doesn't exist, do the check

    time_t last_check = 0;
    if (fscanf(f, "%ld", &last_check) != 1)
    {
        fclose(f);
        return 1; // Can't read, do the check
    }
    fclose(f);

    time_t now = time(NULL);
    time_t interval = get_check_interval();

    // If last check was long enough ago, do the check
    if (now - last_check >= interval)
        return 1;

    return 0; // Throttled
}

/**
 * Update the last check timestamp
 */
static void update_check_timestamp(void)
{
    const char *home = getenv("HOME");
    if (!home)
        return;

    char last_check_path[512];
    snprintf(last_check_path, sizeof(last_check_path), "%s%s", home, UPDATE_LAST_CHECK_FILE);

    FILE *f = fopen(last_check_path, "w");
    if (!f)
        return;

    fprintf(f, "%ld\n", time(NULL));
    fclose(f);
}

/**
 * Extract version string from GitHub API JSON response
 * Looks for version in "name" field (MacOS-v1.2.1) or "tag_name" field (v1.2.1)
 * Returns version string without 'v' prefix, or NULL on error (must be freed)
 */
static char *parse_github_version(const char *json_response)
{
    const char *pos = NULL;

    // Debug: log first 200 chars of response
    char debug_buf[512];
    if (json_response && strlen(json_response) > 0)
    {
        snprintf(debug_buf, sizeof(debug_buf), "Response preview: %.200s", json_response);
        update_debug_log(debug_buf);
    }

    // Try to find version from "name" field first (modern format: MacOS-v1.2.1)
    pos = strstr(json_response, "\"name\":");
    if (!pos)
    {
        // Fall back to "tag_name" field (legacy format: v1.2.1)
        pos = strstr(json_response, "\"tag_name\":");
    }

    if (!pos)
    {
        update_debug_log("ERROR: Neither 'name' nor 'tag_name' found in response");
        return NULL;
    }

    // Move past the key (skip to the colon, then past it)
    while (*pos && *pos != ':')
        pos++;
    if (*pos == ':')
        pos++; // Move past the colon

    // Skip whitespace
    while (*pos && (*pos == ' ' || *pos == '\t' || *pos == '\n' || *pos == '\r'))
        pos++;

    // Now we should be at the opening quote of the value
    if (*pos == '"')
        pos++; // Move past opening quote

    // Extract full field value until closing quote
    char *version = malloc(UPDATE_MAX_VERSION_LEN);
    if (!version)
        return NULL;

    int i = 0;
    while (*pos && *pos != '"' && i < UPDATE_MAX_VERSION_LEN - 1)
    {
        version[i++] = *pos++;
    }
    version[i] = '\0';

    // Debug log what we extracted
    snprintf(debug_buf, sizeof(debug_buf), "Extracted raw value: '%s'", version);
    update_debug_log(debug_buf);
    if (strlen(version) == 0)
    {
        free(version);
        return NULL;
    }

    // Now extract just the version number
    // Handle formats: "MacOS-v1.2.1", "v1.2.1", "1.2.1"
    char *ver_start = version;

    // Look for 'v' followed by a digit
    for (int j = 0; version[j]; j++)
    {
        if (version[j] == 'v' && (j + 1 < (int)strlen(version)) &&
            version[j + 1] >= '0' && version[j + 1] <= '9')
        {
            ver_start = version + j + 1; // Start after 'v'
            break;
        }
    }

    // If we found a different starting point, compact the version string
    if (ver_start != version)
    {
        char temp[UPDATE_MAX_VERSION_LEN];
        strncpy(temp, ver_start, UPDATE_MAX_VERSION_LEN - 1);
        temp[UPDATE_MAX_VERSION_LEN - 1] = '\0';
        strncpy(version, temp, UPDATE_MAX_VERSION_LEN - 1);
    }

    snprintf(debug_buf, sizeof(debug_buf), "Final parsed version: '%s'", version);
    update_debug_log(debug_buf);

    return version;
}

/**
 * Extract download URL from GitHub API JSON response
 * Looking for: "browser_download_url":"https://..."
 * Returns URL string, or NULL on error (must be freed)
 */
static char *parse_github_download_url(const char *json_response)
{
    const char *pos = strstr(json_response, "\"browser_download_url\":");
    if (!pos)
        return NULL;

    // Move past the key (skip to the colon, then past it)
    while (*pos && *pos != ':')
        pos++;
    if (*pos == ':')
        pos++; // Move past the colon

    // Skip whitespace
    while (*pos && (*pos == ' ' || *pos == '\t' || *pos == '\n' || *pos == '\r'))
        pos++;

    // Now we should be at the opening quote of the value
    if (*pos == '"')
        pos++; // Move past opening quote

    // Find the download URL (first one should be the binary)
    char *url = malloc(1024);
    if (!url)
        return NULL;

    int i = 0;
    while (*pos && *pos != '"' && i < 1023)
    {
        url[i++] = *pos++;
    }
    url[i] = '\0';

    if (i == 0 || i >= 1023)
    {
        free(url);
        return NULL;
    }

    return url;
}

/**
 * Download file from URL using curl
 * Returns 0 on success, -1 on failure
 */
static int download_file(const char *url, const char *output_path)
{
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
             "curl -L --max-time %d -s -o %s '%s' 2>/dev/null",
             UPDATE_TIMEOUT, output_path, url);

    if (system(cmd) != 0)
        return -1;

    // Verify file was created and has reasonable size (at least 50KB)
    FILE *f = fopen(output_path, "r");
    if (!f)
        return -1;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);

    if (size < 50000) // Binary should be larger than this
        return -1;

    return 0;
}

/**
 * Replace current binary with new one atomically
 * Returns 0 on success, -1 on failure
 */
static int install_binary(const char *new_binary_path)
{
    const char *install_dir = getenv("HOME");
    if (!install_dir)
        return -1;

    char current_binary[512];
    char backup_binary[512];
    snprintf(current_binary, sizeof(current_binary), "%s/.config/walcman/walcman", install_dir);
    snprintf(backup_binary, sizeof(backup_binary), "%s/.config/walcman/walcman%s", install_dir, UPDATE_BACKUP_BINARY);

    // Backup current binary
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "mv '%s' '%s' 2>/dev/null", current_binary, backup_binary);
    if (system(cmd) != 0)
        return -1;

    // Move new binary into place
    snprintf(cmd, sizeof(cmd), "mv '%s' '%s' 2>/dev/null", new_binary_path, current_binary);
    if (system(cmd) != 0)
    {
        // Restore backup if move failed
        snprintf(cmd, sizeof(cmd), "mv '%s' '%s' 2>/dev/null", backup_binary, current_binary);
        system(cmd);
        return -1;
    }

    // Make executable
    snprintf(cmd, sizeof(cmd), "chmod +x '%s'", current_binary);
    system(cmd);

    return 0;
}

/**
 * Update VERSION file in installation directory
 */
static int update_version_file(const char *new_version)
{
    const char *install_dir = getenv("HOME");
    if (!install_dir)
        return -1;

    char version_path[512];
    snprintf(version_path, sizeof(version_path), "%s/.config/walcman/VERSION", install_dir);

    FILE *f = fopen(version_path, "w");
    if (!f)
        return -1;

    fprintf(f, "%s\n", new_version);
    fclose(f);

    return 0;
}

/**
 * Fetch latest release info from GitHub
 * Returns JSON response string (must be freed), or NULL on error
 */
static char *fetch_github_release(void)
{
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
             "curl -L --max-time %d -s '%s' 2>/dev/null",
             UPDATE_TIMEOUT, UPDATE_GITHUB_API);

    FILE *fp = popen(cmd, "r");
    if (!fp)
        return NULL;

    // Read response into buffer
    size_t total_size = 0;
    size_t buffer_size = 8192;
    char *response = malloc(buffer_size);
    if (!response)
    {
        pclose(fp);
        return NULL;
    }

    int c;
    while ((c = fgetc(fp)) != EOF)
    {
        if (total_size >= buffer_size - 1)
        {
            char *new_response = realloc(response, buffer_size * 2);
            if (!new_response)
            {
                free(response);
                pclose(fp);
                return NULL;
            }
            response = new_response;
            buffer_size *= 2;
        }
        response[total_size++] = c;
    }

    pclose(fp);

    response[total_size] = '\0';

    // Verify we got valid JSON response
    if (total_size < 50 || strstr(response, "tag_name") == NULL)
    {
        free(response);
        return NULL;
    }

    return response;
}

/**
 * Write debug log entry to file
 */
static void update_debug_log(const char *message)
{
    const char *home = getenv("HOME");
    if (!home)
        return;

    char log_path[512];
    snprintf(log_path, sizeof(log_path), "%s/.config/walcman/.update.log", home);

    FILE *f = fopen(log_path, "a");
    if (!f)
        return;

    time_t now = time(NULL);
    fprintf(f, "[%ld] %s\n", now, message);
    fclose(f);
}

/**
 * Background update check process
 * This runs in a forked child process
 */
static void update_worker(void)
{
    // Write to log instead of stdout/stderr so we can debug
    update_debug_log("Update check started");

    fclose(stdout);
    fclose(stderr);

    // Fetch latest release info
    char *github_response = fetch_github_release();
    if (!github_response)
    {
        update_debug_log("ERROR: Failed to fetch GitHub release");
        exit(1);
    }

    char debug_msg[512];
    snprintf(debug_msg, sizeof(debug_msg), "Fetched response, length: %ld", strlen(github_response));
    update_debug_log(debug_msg);

    update_debug_log("GitHub release fetched successfully");

    // Parse version
    char *remote_version = parse_github_version(github_response);
    if (!remote_version)
    {
        update_debug_log("ERROR: Failed to parse remote version");
        free(github_response);
        exit(1);
    }

    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "Remote version: %s", remote_version);
    update_debug_log(log_msg);

    // Get local version
    char *local_version = get_local_version();
    if (!local_version)
    {
        update_debug_log("ERROR: Failed to read local version");
        free(github_response);
        free(remote_version);
        exit(1);
    }

    snprintf(log_msg, sizeof(log_msg), "Local version: %s", local_version);
    update_debug_log(log_msg);

    // Compare versions - simple string comparison
    // (assumes semantic versioning: v1.1.4 < v1.1.5 < v1.2.0)
    if (strcmp(remote_version, local_version) <= 0)
    {
        // No update needed
        update_debug_log("No update needed (remote <= local)");
        free(github_response);
        free(remote_version);
        free(local_version);
        exit(0);
    }

    update_debug_log("Update available");

    // Extract download URL
    char *download_url = parse_github_download_url(github_response);
    if (!download_url)
    {
        update_debug_log("ERROR: Failed to extract download URL");
        free(github_response);
        free(remote_version);
        free(local_version);
        exit(1);
    }

    snprintf(log_msg, sizeof(log_msg), "Download URL extracted");
    update_debug_log(log_msg);

    free(github_response);

    // Download new binary
    update_debug_log("Starting binary download...");
    if (download_file(download_url, UPDATE_TEMP_BINARY) != 0)
    {
        update_debug_log("ERROR: Failed to download binary");
        free(remote_version);
        free(local_version);
        free(download_url);
        exit(1);
    }
    update_debug_log("Binary downloaded successfully");

    free(download_url);

    // Install binary atomically
    update_debug_log("Installing binary...");
    if (install_binary(UPDATE_TEMP_BINARY) != 0)
    {
        update_debug_log("ERROR: Failed to install binary");
        free(remote_version);
        free(local_version);
        exit(1);
    }
    update_debug_log("Binary installed successfully");

    // Update VERSION file
    update_version_file(remote_version);

    // Update last check timestamp
    update_check_timestamp();

    update_debug_log("Update completed successfully");

    free(remote_version);
    free(local_version);
    exit(0);
}

int update_check_background(void)
{
    // Check if updates are enabled in config
    int enabled = is_update_enabled();
    if (enabled == 0)
        return 0; // Updates disabled, don't check

    // Check if enough time has passed since last check
    if (!should_check_for_updates())
        return 0; // Throttled, don't check yet

    pid_t pid = fork();

    if (pid < 0)
    {
        // Fork failed, but we don't want to fail startup
        return -1;
    }

    if (pid == 0)
    {
        // Child process - run update worker
        update_worker();
        exit(1); // Should never reach here
    }

    // Parent process - return immediately
    // Let child run in background without waiting
    return 0;
}
