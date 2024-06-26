#include "btree.h"

uint32_t* leaf_node_num_cells(void* node)
{
    return node + LEAF_NODE_NUM_CELLS_OFFSET;
}

void* leaf_node_cell(void* node, uint32_t cell_num) 
{
    return node + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
}

uint32_t* leaf_node_key(void* node, uint32_t cell_num) 
{
    return leaf_node_cell(node, cell_num);
}

void* leaf_node_value(void* node, uint32_t cell_num) 
{
    return leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}

void initialize_leaf_node(void* node) { 
    set_node_type(node, NODE_LEAF);
    *leaf_node_num_cells(node) = 0;
}

void print_constants() {
    printf("ROW_SIZE: %d\n", ROW_SIZE);
    printf("COMMON_NODE_HEADER_SIZE: %d\n", COMMON_NODE_HEADER_SIZE);
    printf("LEAF_NODE_HEADER_SIZE: %d\n", LEAF_NODE_HEADER_SIZE);
    printf("LEAF_NODE_CELL_SIZE: %d\n", LEAF_NODE_CELL_SIZE);
    printf("LEAF_NODE_SPACE_FOR_CELLS: %d\n", LEAF_NODE_SPACE_FOR_CELLS);
    printf("LEAF_NODE_MAX_CELLS: %d\n", LEAF_NODE_MAX_CELLS);
}

void leaf_node_split_and_insert(Cursor* cursor, uint32_t key, Row* value)
{

}


void leaf_node_insert(Cursor* cursor, uint32_t key, Row* value)
{
    printf("Inserting node... \n");
    void* node = get_page(cursor->table->pager, cursor->page_num);

    uint32_t num_cells = *leaf_node_num_cells(node); 
    if(num_cells >= LEAF_NODE_MAX_CELLS)
    {
      //Node full
        printf("Implement node insert here");


        leaf_node_split_and_insert(cursor, key, value);
        return;
    }

    if(cursor->cell_num < num_cells)
    {
      //Make room for new cell
      for(uint32_t i = num_cells; i > cursor->cell_num; --i)
      {
        memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i - 1), LEAF_NODE_CELL_SIZE);
      }
    }

    *(leaf_node_num_cells(node)) += 1;
    *(leaf_node_key(node, cursor->cell_num)) = key;
    serialize_row(value, leaf_node_value(node, cursor->cell_num));
}

Cursor* leaf_node_find(Table* table, uint32_t page_num, uint32_t key)
{
    void* node = get_page(table->pager, page_num);
    uint32_t num_cells = *leaf_node_num_cells(node);

    Cursor* cursor = malloc(sizeof(Cursor));
    cursor->table = table;
    cursor->page_num = page_num;

    //Perform a binary search
    uint32_t min_index = 0;
    uint32_t one_past_max_index = num_cells;

    while(one_past_max_index != min_index)
    {
        uint32_t index = (min_index + one_past_max_index) / 2;
        uint32_t key_at_index = *leaf_node_key(node, index);
        if(key == key_at_index){
            cursor->cell_num = index;
            return cursor;
        }
        if(key < key_at_index)
        {
            one_past_max_index = index;
        }
        else
        {
            min_index = index + 1;
        }
    }

    cursor->cell_num = min_index;
    return cursor;
}

NodeType get_node_type(void* node)
{
    uint8_t type_data = *((uint8_t*)(node + NODE_TYPE_OFFSET));
    return (NodeType)type_data;
}

void set_node_type(void* node, NodeType type)
{
    uint8_t type_data = type;
    
    *((uint8_t*)(node + NODE_TYPE_OFFSET)) = type_data;
}
