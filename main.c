#include <stdio.h>

#include "file_util.h"
#include "backend.h"
#include "sql_parser.h"

void print_row(Row* row) {
  printf("(%d, %s, %s)\n", row->id, row->user_name, row->email);
}

void close_buffer(InputBuffer* input_buffer){
    free(input_buffer->buffer);
    free(input_buffer);
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
    int bytes_read = 
        getline(&(input_buffer->buffer), &(input_buffer->buffer_len), stdin);
    
    if(bytes_read < 0){
        printf("Error reading input!\n");
        exit(1);
    }

    //Ignore trailing new line
    input_buffer->input_len = bytes_read - 1;
    input_buffer->buffer[bytes_read - 1] = 0;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("Must supply a database filename.\n");
        return -1;
    }
    InputBuffer* current_buffer = new_input_buffer();
    const char* filename = argv[1];
    Table* table = db_open(filename);
    while(true)
    {
        print_prompt();
        read_input(current_buffer);

        parse_meta_command(current_buffer, table);

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