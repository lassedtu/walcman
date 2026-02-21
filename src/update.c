/**
 * update.c - Implementation of automatic update system
 *
 * Handles version checking and binary updates from GitHub releases
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "update.h"

#define UPDATE_GITHUB_API "https://api.github.com/repos/lassedtu/walcman/releases/latest"
#define UPDATE_TIMEOUT 5
#define UPDATE_MAX_VERSION_LEN 50
#define UPDATE_TEMP_BINARY "/tmp/walcman.update"
#define UPDATE_BACKUP_BINARY ".backup"

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
 * Extract version string from GitHub API JSON response
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

    free(remote_version);
    free(local_version);
    exit(0);
}

int update_check_background(void)
{
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
