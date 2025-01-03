// main.c
#include <stdio.h>
#include <sqlite3.h>
#include "file_indexer.h"

#define DB_NAME "file_index.db"

int main() {
    sqlite3 *db;
    char *err_msg = 0;

    // Open SQLite database
    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    // Create table if it doesn't exist
    const char *sql = "CREATE TABLE IF NOT EXISTS files ("
                      "path TEXT PRIMARY KEY, "
                      "last_modified INTEGER);";

    if (sqlite3_exec(db, sql, 0, 0, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return 1;
    }

    // Start indexing files from the current directory
    index_files(".", db);

    // Close the database connection
    sqlite3_close(db);
    return 0;
}#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sqlite3.h>

#define DB_NAME "file_index.db"

// Supported file extensions
const char *video_extensions[] = {".mp4", ".avi", ".mkv", ".mov", NULL};
const char *image_extensions[] = {".jpg", ".jpeg", ".png", ".gif", NULL};

// Function to check if a file has a specific extension
int has_extension(const char *filename, const char *ext) {
    const char *dot = strrchr(filename, '.');
    return (dot && strcmp(dot, ext) == 0);
}

// Function to check if a file is a video or image
int is_media_file(const char *filename) {
    for (int i = 0; video_extensions[i] != NULL; i++) {
        if (has_extension(filename, video_extensions[i])) {
            return 1; // Video file
        }
    }
    for (int i = 0; image_extensions[i] != NULL; i++) {
        if (has_extension(filename, image_extensions[i])) {
            return 1; // Image file
        }
    }
    return 0; // Not a media file
}

// Function to recursively index files
void index_files(const char *base_path, sqlite3 *db) {
    struct dirent *entry;
    struct stat file_stat;
    char path[1024];

    DIR *dp = opendir(base_path);
    if (dp == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dp)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue; // Skip . and ..
        }

        snprintf(path, sizeof(path), "%s/%s", base_path, entry->d_name);
        if (stat(path, &file_stat) == -1) {
            perror("stat");
            continue;
        }

        if (S_ISDIR(file_stat.st_mode)) {
            // Recursively index subdirectories
            index_files(path, db);
        } else if (is_media_file(entry->d_name)) {
            // Check if the file is a media file
            sqlite3_stmt *stmt;
            const char *sql = "INSERT OR REPLACE INTO files (path, last_modified) VALUES (?, ?);";

            sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
            sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
            sqlite3_bind_int64(stmt, 2, file_stat.st_mtime); // Last modified time

            if (sqlite3_step(stmt) != SQLITE_DONE) {
                fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
            }
            sqlite3_finalize(stmt);
        }
    }

    closedir(dp);
}

int main() {
    sqlite3 *db;
    char *err_msg = 0;

    // Open SQLite database
    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    // Create table if it doesn't exist
    const char *sql = "CREATE TABLE IF NOT EXISTS files ("
                      "path TEXT PRIMARY KEY, "
                      "last_modified INTEGER);";

    if (sqlite3_exec(db, sql, 0, 0, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return 1;
    }

    // Start indexing files from the current directory
    index_files(".", db);

    // Close the database
    sqlite3_close(db);
    return 0;
}

