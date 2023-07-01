#ifndef FILE_UTIL_H
#define FILE_UTIL_H


#include "db_datatype.h"

Table* db_open(const char* filename);
void db_close(Table* table);
Pager* pager_open(const char* filename);
void pager_flush(Pager* pager, uint32_t page_num, uint32_t size);

#endif