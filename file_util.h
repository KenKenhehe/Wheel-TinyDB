#ifndef FILE_UTIL_H
#define FILE_UTIL_H


#include "db_datatype.h"
#include "btree.h"
Table* db_open(const char* filename);
void db_close(Table* table);
Pager* pager_open(const char* filename);
void* get_page(Pager* pager, uint32_t page_num);
void pager_flush(Pager* pager, uint32_t page_num);

#endif