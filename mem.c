#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "mem.h"
#define INF INT_MAX
void *head;
void *tail;
int cnt = 0;
typedef struct
{
    int size;
    int state;//check if the block is available
}node;

int mem_init(int size_of_region)
{
    size_of_region += sizeof(node);
    //mem_init is called more than once
    //size_of_region is less than or equal to 0.
    if(size_of_region <=0 || cnt > 0)
    {
        m_error = E_BAD_ARGS;
        return -1;
    }
    cnt++;

    int pagesize = getpagesize();
    //request memory in units of the page size
    if(size_of_region % pagesize != 0)
    {
        size_of_region = (size_of_region / pagesize + 1) * pagesize;
    }
    // open the /dev/zero device
    int fd = open("/dev/zero", O_RDWR);
    // size_of_region (in bytes) needs to be evenly divisible by the page size
    head= mmap(NULL, size_of_region, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (head == MAP_FAILED) { perror("mmap"); exit(1); }
    // close the device (don't worry, mapping should be unaffected)
    close(fd);

    node *cur = (node*) head;
    cur ->size = size_of_region - sizeof(cur);
    cur ->state = 1;
    tail = head + size_of_region ;

    return 0;
}
void * mem_alloc(int size, int style)
{
    //8-byte aligned chunks of memory
    if(size % 8 != 0 )
    {
        size = (size / 8 + 1) * 8;
    }
    void *res =NULL;
    void *cur = head;
    //finds the first free space that is smallest in size
    if(style == M_BESTFIT)
    {
        int min = INF ;
        while(cur < tail)
        {
            node *temp = (node*) cur;
            if(size <= temp->size &&  temp->state == 1)
                if( min >= temp->size)
                    min = temp->size,res = cur;
            cur += temp->size + sizeof(node);
        }
    }
    //looks for the largest chunk
    else if(style == M_WORSTFIT)
    {
        int max = 0;
        while(cur < tail)
        {
            node *temp = (node*) cur;
            if(size < = temp->size &&  temp->state == 1)
                if( max <= temp->size)
                    max = temp->size, res = cur;
            cur += temp->size + sizeof(node);
        }
    }
    //looks for the first chunk that fits
    else if(style == M_FIRSTFIT)
    {
        while(cur < tail)
        {
            node *temp = (node*) cur;
            if(size < = temp->size &&  temp->state == 1)
            {
                res = cur;
                break;
            }
            cur += temp->size + sizeof(node);
        }
    }
    if(res == NULL)
    {
        m_error = E_NO_SPACE;
        return NULL;
    }
    node *temp = (node*)res;
    if(temp->size > size)
    {
        node *next = (node*)(res + size + sizeof(node));
        next->state = 1;
        next->size = temp->size - size - sizeof(node);
    }
    temp->state = 0;
    temp->size = size;
    return res + sizeof(node);
}

int mem_free(void *ptr)
{
    ptr -= sizeof(node);
    void *pre = head;
    node *cur =(node*)ptr;
    if( ptr ==NULL || ptr < head || ptr > tail || cur->state == 1)
    {
        m_error = E_BAD_POINTER;
        return -1;
    }
    //union the previous available block
    while(pre < ptr)
    {
        node *pre_block = (node*) pre;
        if(pre + pre_block->size + sizeof(node) == ptr)
        {
            if(pre_block->state == 1)
                pre_block->size += (cur->size + sizeof(node)),ptr=pre;
            break;
        }
        pre += (pre_block->size + sizeof(node));
    }
    //union the next available block
    cur = (node*)ptr;
    cur->state =1;
    node *next =(node*) (ptr + cur->size + sizeof(node));
    if(next->state == 1)
        cur->size += (next->size + sizeof(node));
    return 0;
}

void mem_dump(){
    void *cur = head;
    int num = 0;
    printf("|---------------------------------------------------------------|\n");
    printf("\tID\tstart\t\t\tsize\tstate\n");
    while (cur < tail) {
        node* temp =(node*) cur;
        printf("\t%3d\t%p\t%3d\t%3d\n", num++, cur, temp->size, temp->state);
        cur += (sizeof(node) + temp->size);
    }
    printf("|---------------------------------------------------------------|\n");
}
