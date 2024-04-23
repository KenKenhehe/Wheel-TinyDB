#include "file_util.h"
Table* db_open(const char* filename)
{
    Pager* pager = pager_open(filename);
    Table* table = (Table*)malloc(sizeof(Table));

    table->pager = pager;
    table->root_page_num = 0;

    if(pager->num_pages == 0)
    {
        void* root_node = get_page(pager, 0);
        initialize_leaf_node(root_node);
    }

    return table;
}

void db_close(Table* table)
{
    Pager* pager = table->pager;
    uint32_t num_full_page = pager->num_pages;
    for(uint32_t i = 0; i < num_full_page; i++){
        if(pager->pages[i] == NULL){
            continue;
        }
        pager_flush(pager, i);
        free(pager->pages[i]);
        pager->pages[i] = NULL;
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
    pager->num_pages = file_length / PAGE_SIZE;

    if (file_length % PAGE_SIZE != 0) {
        printf("Db file is not a whole number of pages. Corrupt file.\n");
        exit(EXIT_FAILURE);
    }

    for(uint32_t i = 0; i < TABLE_MAX_PAGES; i++){
        pager->pages[i] = NULL;
    }

    return pager;
}

void* get_page(Pager* pager, uint32_t page_num)
{
    printf("Getting page...\n");
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

        if(page_num > pager->num_pages){
            pager->num_pages = page_num + 1;
        }
    }

    return pager->pages[page_num];
}

void pager_flush(Pager* pager, uint32_t page_num)
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

    ssize_t bytes_written = write(pager->file_descriptor, pager->pages[page_num], PAGE_SIZE);

    if(bytes_written == -1)
    {
        printf("Error writting: %d\n", errno);
        return;
    }
}
