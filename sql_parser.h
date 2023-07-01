#ifndef COMPILER_H
#define COMPILER_H

#include "db_datatype.h"

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement);
PrepareResult prepare_insert(InputBuffer* input_buffer, Statement* statement);
MetaCommandResult parse_meta_command(InputBuffer* input_buffer, Table* table);

#endif