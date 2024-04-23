#ifndef DB_DATATYPE_H
#define DB_DATATYPE_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define TABLE_MAX_PAGES 100

//Application-specific row type
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 256
typedef struct 
{
    uint32_t id;
    char user_name[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
}Row;

#define SIZE_OF_ATTRIBUTE(Struct, attribute) sizeof(((Struct*)0)->attribute)

//Application-specific constant
static const uint32_t ID_SIZE = SIZE_OF_ATTRIBUTE(Row, id);
static const uint32_t USERNAME_SIZE = SIZE_OF_ATTRIBUTE(Row, user_name);
static const uint32_t EMAIL_SIZE = SIZE_OF_ATTRIBUTE(Row, email);
static const uint32_t ID_OFFSET = 0;
static const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
static const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;


//Database constant
static const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

static const uint32_t PAGE_SIZE = 4096;
static const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
static const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

typedef enum{
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZE_COMMAND
} MetaCommandResult;

typedef enum{
    PREPARE_SUCCESS,
    PREPARE_SYNTAX_ERROR,
    PREPARE_NEGITIVE_ID,
    PREPARE_STRING_TOO_LONG,
    PREPARE_UNRECOGNIZE_STATEMENT
}PrepareResult;

typedef enum {
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL
}ExecuteResult;

typedef enum{
    STATEMENT_INSERT,
    STATEMENT_SELECT,
    STATENEBT_UNKNOWN
}StatementType;

typedef struct
{
    char* buffer;
    size_t buffer_len;
    size_t input_len;
} InputBuffer;

typedef struct{
    StatementType statement_type;
    Row row_to_insert; //only used by insert statement
}Statement;

typedef struct 
{
    int file_descriptor;
    uint32_t file_length;
    uint32_t num_pages;
    void* pages[TABLE_MAX_PAGES];
    /* data */
}Pager;

typedef struct{
    Pager* pager;
    uint32_t root_page_num;
}Table;

typedef struct{
    Table* table;
    uint32_t page_num;
    uint32_t cell_num;
    bool end_of_table;
}Cursor;

#endif