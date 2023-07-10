#include "backend.h"

void print_row(Row* row) 
{
  printf("(%d, %s, %s)\n", row->id, row->user_name, row->email);
}

Cursor* table_start(Table* table)
{
    Cursor* cursor = malloc(sizeof(Cursor));
    cursor->table = table;
    cursor->row_num = 0;
    cursor->end_of_table = (table->num_rows == 0);
    return cursor;
}

Cursor* table_end(Table* table)
{
    Cursor* cursor = malloc(sizeof(Cursor));
    cursor->table = table;
    cursor->row_num = table->num_rows;
    cursor->end_of_table = true;
    return cursor;
}

void cursor_advance(Cursor* cursor)
{
    cursor->row_num+=1;
    if(cursor->row_num >= cursor->table->num_rows){
        cursor->end_of_table = true;
    }
}

void serialize_row(Row* source, void* destination)
{
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(destination + USERNAME_OFFSET, &(source->user_name), USERNAME_SIZE);
    memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void* source, Row* destination) {
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->user_name), source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

ExecuteResult execute_insert(Statement* statement, Table* table)
{
    if(table->num_rows > TABLE_MAX_ROWS)
    {
        return EXECUTE_TABLE_FULL;
    }

    Cursor* cursor = table_end(table);

    Row* row_to_insert = &(statement->row_to_insert);
    serialize_row(row_to_insert, cursor_value(cursor));
    table->num_rows += 1;

    return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement* statement, Table* table)
{
    Row row;
    Cursor* cursor = table_start(table);

    while(!(cursor->end_of_table))
    {
        deserialize_row(cursor_value(cursor), &row);
        print_row(&row);
        cursor_advance(cursor);
    }

    free(cursor);

    return EXECUTE_SUCCESS;
}

ExecuteResult execute_sql_command(Statement* statement, Table* table){
    switch (statement->statement_type)
    {
    case STATEMENT_INSERT:
        printf("Executing INSERT...\n");
        return execute_insert(statement, table);
    
    case STATEMENT_SELECT:
        printf("Executing SELECT...\n");

        return execute_select(statement, table);

    default:
        printf("Unknown statement\n");
        return -1;
    }
}

void* get_page(Pager* pager, uint32_t page_num)
{
    if(page_num > TABLE_MAX_PAGES)
    {
        printf("Tried to fetch page number out of bounds. %d > %d\n", page_num,
           TABLE_MAX_PAGES);
        return NULL;
    }

    //cache miss, allocate memory and load from file
    if(pager->pages[page_num] == NULL)
    {
        void* page = malloc(PAGE_SIZE);
        uint32_t num_pages = pager->file_length / PAGE_SIZE;

        if(pager->file_length % PAGE_SIZE)
        {
            num_pages += 1;
        }

        if(page_num <= num_pages)
        {
            lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
            ssize_t bytes_read = read(pager->file_descriptor, page, PAGE_SIZE);
            if(bytes_read == -1)
            {
                printf("Error reading file: %d\n", errno);
                return NULL;
            }
        }
        pager->pages[page_num] = page;
    }

    return pager->pages[page_num];
}

void* cursor_value(Cursor* cursor)
{
    uint32_t page_num = cursor->row_num / ROWS_PER_PAGE;
    
    void* page = get_page(cursor->table->pager, page_num);
    uint32_t row_offset = cursor->row_num % ROWS_PER_PAGE;
    uint32_t bytes_offset = row_offset * ROW_SIZE;
    return page + bytes_offset;
}

void* row_slot(Table* table, uint32_t row_num)
{
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    
    void* page = get_page(table->pager, page_num);
    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t bytes_offset = row_offset * ROW_SIZE;
    return page + bytes_offset;
}