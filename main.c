#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 256
#define TABLE_MAX_PAGES 100

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
    STATEMENT_SELECT
}StatementType;

typedef struct 
{
    uint32_t id;
    char user_name[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
}Row;

void print_row(Row* row) {
  printf("(%d, %s, %s)\n", row->id, row->user_name, row->email);
}

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
    void* pages[TABLE_MAX_PAGES];
    /* data */
}Pager;

typedef struct{
    uint32_t num_rows;
    Pager* pager;
}Table;


void close_buffer(InputBuffer* input_buffer){
    free(input_buffer->buffer);
    free(input_buffer);
}

#define SIZE_OF_ATTRIBUTE(Struct, attribute) sizeof(((Struct*)0)->attribute)

const uint32_t ID_SIZE = SIZE_OF_ATTRIBUTE(Row, id);
const uint32_t USERNAME_SIZE = SIZE_OF_ATTRIBUTE(Row, user_name);
const uint32_t EMAIL_SIZE = SIZE_OF_ATTRIBUTE(Row, email);

const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;

const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;


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

        if(pager->file_length % PAGE_SIZE){
            num_pages += 1;
        }

        if(page_num <= num_pages){
            lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
            
        }

    }

    return pager->pages[page_num];
}

void* row_slot(Table* table, uint32_t row_num)
{
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    void* page = table->pages[page_num];
    if(page == NULL)
    {
        // Allocate memory only when we try to access page
        page = table->pages[page_num] = malloc(PAGE_SIZE);
    }
    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t bytes_offset = row_offset * ROW_SIZE;
    return page + bytes_offset;
}


MetaCommandResult parse_meta_command(InputBuffer* input_buffer)
{
    if(strcmp(input_buffer->buffer, "exit") == 0)
    {
        close_buffer(input_buffer);
        printf("Exit DB\n");
        exit(1);
        return META_COMMAND_SUCCESS;
    }
    else{
        return META_COMMAND_UNRECOGNIZE_COMMAND;
    }
}

PrepareResult prepare_insert(InputBuffer* input_buffer, Statement* statement)
{
    char* keyword = strtok(input_buffer->buffer, " ");
    char* id_str = strtok(NULL, " ");
    char* username = strtok(NULL, " ");
    char* email = strtok(NULL, " ");

    if(id_str == NULL || username == NULL || email == NULL)
    {
        return PREPARE_SYNTAX_ERROR;
    }

    int id = atoi(id_str);

    if(id < 0)
    {
        return PREPARE_NEGITIVE_ID;
    }

    if(strlen(username) > COLUMN_USERNAME_SIZE){
        return PREPARE_STRING_TOO_LONG;
    }
    if(strlen(email) > COLUMN_EMAIL_SIZE){
        return PREPARE_STRING_TOO_LONG;
    }

    statement->row_to_insert.id = id;
    strcpy(statement->row_to_insert.user_name, username);
    strcpy(statement->row_to_insert.email, email);

    return PREPARE_SUCCESS;
}

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement)
{
    if(strncmp(input_buffer->buffer, "insert", 6) == 0)
    {
        return prepare_insert(input_buffer, statement);
    }
    if(strcmp(input_buffer->buffer, "select") == 0){
        statement->statement_type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZE_STATEMENT;
}

Pager* pager_open(const char* filename)
{
    int fd = open(filename, 
                    O_RDWR | 
                    O_CREAT,
                    S_IWUSR|
                    S_IRUSR);

    if(fd == -1){
        printf("Unable to open file\n");
        return NULL;
    }

    off_t file_length = lseek(fd, 0, SEEK_END);

    Pager* pager = malloc(sizeof(Pager));
    pager->file_descriptor = fd;
    pager->file_length = file_length;

    for(uint32_t i = 0; i < TABLE_MAX_PAGES; i++){
        pager->pages[i] = NULL;
    }

    return pager;
}

Table* db_open(const char* filename){
    Pager* pager = pager_open(filename);
    uint32_t num_rows = pager->file_length / ROW_SIZE;
    Table* table = (Table*)malloc(sizeof(Table));

    table->pager = pager;
    table->num_rows = num_rows;

    return table;
}

void free_table(Table* table){
    for (uint32_t i = 0; table->pages[i]; ++i)
    {
        /* code */
        free(table->pager->pages[i]);
    }

    free(table);
}

ExecuteResult execute_insert(Statement* statement, Table* table)
{
    if(table->num_rows > TABLE_MAX_ROWS)
    {
        EXECUTE_TABLE_FULL;
    }
    Row* row_to_insert = &(statement->row_to_insert);
    serialize_row(row_to_insert, row_slot(table, table->num_rows));
    table->num_rows += 1;

    return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement* statement, Table* table)
{
    Row row;
    for (uint32_t i = 0; i < table->num_rows; ++i)
    {
        deserialize_row(row_slot(table, i), &row);
        print_row(&row);
    }

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

InputBuffer* new_input_buffer()
{
    InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(input_buffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_len = 0;
    input_buffer->input_len = 0;
    return input_buffer;
}

void print_prompt(){
    printf("db > ");
}

void read_input(InputBuffer* input_buffer)
{
    size_t bytes_read = 
        getline(&(input_buffer->buffer), &(input_buffer->buffer_len), stdin);
    
    // printf("buffer: %s\n", input_buffer->buffer);

    if(bytes_read < 0){
        printf("Error reading input!\n");
        exit(1);
    }

    //Ignore trailing new line
    input_buffer->input_len = bytes_read - 1;
    input_buffer->buffer[bytes_read - 1] = 0;
}

int main(int argc, char* atgv[])
{
    Table* table = new_table();
    InputBuffer* current_buffer = new_input_buffer();
    while(true)
    {
        print_prompt();
        read_input(current_buffer);

        parse_meta_command(current_buffer);

        Statement statement;
        switch (prepare_statement(current_buffer, &statement))
        {
        case PREPARE_SUCCESS:
            break;

        case PREPARE_STRING_TOO_LONG:
            printf("String is too long\n");
            continue;

        case PREPARE_UNRECOGNIZE_STATEMENT:
            printf("Unrecognized keyword at start of '%s'.\n", current_buffer->buffer);
            continue;       
        
        case PREPARE_NEGITIVE_ID:
            printf("Id cannot be negitive\n");
            continue;

        case PREPARE_SYNTAX_ERROR:
            printf("Syntax error\n");
            continue;
        }

        ExecuteResult result = execute_sql_command(&statement, table);
        switch (result) {
        case (EXECUTE_SUCCESS):
            printf("Executed.\n");
            break;
        case (EXECUTE_TABLE_FULL):
            printf("Error: Table full.\n");
            break;
    }
    }
    return 0;
}