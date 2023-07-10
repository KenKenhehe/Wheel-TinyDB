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

void print_row(Row* row);

//Cursor related functions
void* cursor_value(Cursor* cursor);
void cursor_advance(Cursor* cursor);
Cursor* table_start(Table* table);
Cursor* table_end(Table* table);


#endif 