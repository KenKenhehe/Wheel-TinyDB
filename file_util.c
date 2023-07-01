#include "file_util.h"
Table* db_open(const char* filename)
{
    Pager* pager = pager_open(filename);
    uint32_t num_rows = pager->file_length / ROW_SIZE;
    Table* table = (Table*)malloc(sizeof(Table));

    table->pager = pager;
    table->num_rows = num_rows;

    return table;
}

void db_close(Table* table)
{
    Pager* pager = table->pager;
    uint32_t num_full_page = table->num_rows / ROWS_PER_PAGE;
    for(uint32_t i = 0; i < num_full_page; i++){
        if(pager->pages[i] == NULL){
            continue;
        }
        pager_flush(pager, i, PAGE_SIZE);
        free(pager->pages[i]);
        pager->pages[i] = NULL;
    }

    //may be additional rows 
    uint32_t num_additional_rows = table->num_rows % ROWS_PER_PAGE;
    if (num_additional_rows > 0) {
        uint32_t page_num = num_full_page;
        if (pager->pages[page_num] != NULL) {
        pager_flush(pager, page_num, num_additional_rows * ROW_SIZE);
        free(pager->pages[page_num]);
        pager->pages[page_num] = NULL;
        }
    }

    int result = close(pager->file_descriptor);
    if(result == -1){
        printf("fail to close");
        return;
    }

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        void* page = pager->pages[i];
        if (page) {
            free(page);
            pager->pages[i] = NULL;
        }
    }
  free(pager);
  free(table);
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

void pager_flush(Pager* pager, uint32_t page_num, uint32_t size)
{
    if(pager->pages[page_num] == NULL){
        printf("Error: try to flush a null page\n");
        return;
    }
    
    off_t offset = lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);

    if(offset == -1)
    {
        printf("Error seeking: %d\n", errno);
        return;
    }

    ssize_t bytes_written = write(pager->file_descriptor, pager->pages[page_num], size);

    if(bytes_written == -1)
    {
        printf("Error writting: %d\n", errno);
        return;
    }
}
