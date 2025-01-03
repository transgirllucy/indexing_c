// file_indexer.c
#include "file_indexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

const char *video_extensions[] = {".mp4", ".avi", ".mkv", ".mov", NULL};
const char *image_extensions[] = {".jpg", ".jpeg", ".png", ".gif", NULL};

int has_extension(const char *filename, const char *ext) {
    const char *dot = strrchr(filename, '.');
    return (dot && strcmp(dot, ext) == 0);
}

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
