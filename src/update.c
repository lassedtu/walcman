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
 * Looking for: "tag_name":"v1.2.3"
 * Returns version string without 'v' prefix, or NULL on error (must be freed)
 */
static char *parse_github_version(const char *json_response)
 {
     const char *tag_key = "\"tag_name\":\"";
     const char *pos = strstr(json_response, tag_key);
     if (!pos)
         return NULL;

     pos += strlen(tag_key);

     // Skip 'v' prefix if present
     if (*pos == 'v')
         pos++;

     // Extract version until closing quote
     char *version = malloc(UPDATE_MAX_VERSION_LEN);
     if (!version)
         return NULL;

     int i = 0;
     while (*pos && *pos != '"' && i < UPDATE_MAX_VERSION_LEN - 1)
     {
         version[i++] = *pos++;
     }
     version[i] = '\0';

     if (i == 0)
     {
         free(version);
         return NULL;
     }

     return version;
 }

 /**
  * Extract download URL from GitHub API JSON response
  * Looking for: "browser_download_url":"https://..."
  * Returns URL string, or NULL on error (must be freed)
  */
 static char *parse_github_download_url(const char *json_response)
 {
     const char *url_key = "\"browser_download_url\":\"";
     const char *pos = strstr(json_response, url_key);
     if (!pos)
         return NULL;

     pos += strlen(url_key);

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
  * Background update check process
  * This runs in a forked child process
  */
 static void update_worker(void)
 {
     // Close stdout/stderr to avoid any output
     fclose(stdout);
     fclose(stderr);

     // Fetch latest release info
     char *github_response = fetch_github_release();
     if (!github_response)
         exit(1);

     // Parse version
     char *remote_version = parse_github_version(github_response);
     if (!remote_version)
     {
         free(github_response);
         exit(1);
     }

     // Get local version
     char *local_version = get_local_version();
     if (!local_version)
     {
         free(github_response);
         free(remote_version);
         exit(1);
     }

     // Compare versions - simple string comparison
     // (assumes semantic versioning: v1.1.4 < v1.1.5 < v1.2.0)
     if (strcmp(remote_version, local_version) <= 0)
     {
         // No update needed
         free(github_response);
         free(remote_version);
         free(local_version);
         exit(0);
     }

     // Extract download URL
     char *download_url = parse_github_download_url(github_response);
     if (!download_url)
     {
         free(github_response);
         free(remote_version);
         free(local_version);
         exit(1);
     }

     free(github_response);

     // Download new binary
     if (download_file(download_url, UPDATE_TEMP_BINARY) != 0)
     {
         free(remote_version);
         free(local_version);
         free(download_url);
         exit(1);
     }

     free(download_url);

     // Install binary atomically
     if (install_binary(UPDATE_TEMP_BINARY) != 0)
     {
         free(remote_version);
         free(local_version);
         exit(1);
     }

     // Update VERSION file
     update_version_file(remote_version);

     // Update last check timestamp
     update_check_timestamp();

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
         // Parent process - return immediately
         // Let child run in background without waiting
         return 0;
     }
