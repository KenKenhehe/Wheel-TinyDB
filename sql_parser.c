#include "sql_parser.h"

//For printing tree constant
#include "btree.h"

//For closing DB
#include "file_util.h"
PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement)
{
    printf("input buffer: %s, \n", input_buffer->buffer);
    if(strncmp(input_buffer->buffer, "insert", 6) == 0)
    {
        return prepare_insert(input_buffer, statement);
    }
    else if(strcmp(input_buffer->buffer, "select") == 0){
        statement->statement_type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    else
    {
        return PREPARE_UNRECOGNIZE_STATEMENT;
    }
}

PrepareResult prepare_insert(InputBuffer* input_buffer, Statement* statement)
{
    printf("Parsing insert: %s, \n", input_buffer->buffer);

    statement->statement_type = STATEMENT_INSERT;
    char* keyword = strtok(input_buffer->buffer, " ");
    char* id_str = strtok(NULL, " ");
    char* username = strtok(NULL, " ");
    char* email = strtok(NULL, " ");

    if(id_str == NULL || username == NULL || email == NULL || keyword == NULL)
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
MetaCommandResult parse_meta_command(InputBuffer* input_buffer, Table* table)
{
    if(strcmp(input_buffer->buffer, "exit") == 0)
    {
        db_close(table);
        printf("Exit DB\n");
        exit(1);
        return META_COMMAND_SUCCESS;
    }
    else if(strcmp(input_buffer->buffer, "constant") == 0)
    {
        print_constants();
        return META_COMMAND_SUCCESS;
    }
    else
    {
        return META_COMMAND_UNRECOGNIZE_COMMAND;
    }
}
