// file_indexer.h
#ifndef FILE_INDEXER_H
#define FILE_INDEXER_H

#include <sqlite3.h>

void index_files(const char *base_path, sqlite3 *db);
int is_media_file(const char *filename);
int has_extension(const char *filename, const char *ext);

#endif // FILE_INDEXER_H
