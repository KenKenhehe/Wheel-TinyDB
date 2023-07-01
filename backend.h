#ifndef BACKEND_H
#define BACKEND_H

#include "db_datatype.h"
ExecuteResult execute_insert(Statement* statement, Table* table);
ExecuteResult execute_select(Statement* statement, Table* table);
ExecuteResult execute_sql_command(Statement* statement, Table* table);
void* get_page(Pager* pager, uint32_t page_num);
void* row_slot(Table* table, uint32_t row_num);
void serialize_row(Row* source, void* destination);
void deserialize_row(void* source, Row* destination);

#endif 