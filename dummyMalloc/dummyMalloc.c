#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#define align4(x) (((((x)-1)>>3)<<3)+8)
#define BLOCK_SIZE   sizeof( struct s_block)

struct s_block{
    size_t size;            //8 bytes
    struct s_block *next;   //8 bytes
    struct s_block *prev;   //8 bytes
    int free;               //4 bytes
    char data[1];           // 1
};
void *base = NULL;

struct s_block *find_block(struct s_block **last, size_t size){
    struct s_block *b = base;//heap beigins
    while(b && !(b->free && b->size >= size)){
        *last = b;
        b = b->next;
    }
    return (b);
}
struct s_block * extend_heap(struct s_block *last, size_t s){
    struct s_block *b;
    b = sbrk(0);
    if (sbrk(BLOCK_SIZE + s) == (void *) -1){
        return (NULL);
    }
    b->size = s;
    b->next = NULL;

    while (last != NULL){
            last->next = b;
        }
    b->free = 0;
    return (b);
}
void split_block(struct s_block *block, size_t size){
        struct s_block *new;
        new = (struct s_block*)( block->data + 7 + size);
        new->size = block->size - size - BLOCK_SIZE;
        new->next = block->next;
        new->free = 1;
        block->size = size;
        block->next = new;
        block->free  = 0;
    }
struct s_block *merge(struct s_block * b){
    if (b->next != NULL && b->next->free){
        b->size += BLOCK_SIZE + b->next->size;
        b->next = b->next->next;
        if (b->next != NULL){
            b->next->prev = b;
        }
    }
    return (b);
}
void *d_malloc(size_t size){
    struct s_block * b;
    struct s_block *last;
    size_t s;
    s = align4(size);

    if (base != NULL){
            last = base;
            b = find_block(&last, size);
            if (b != NULL){
                if ((b->size-s ) >= (BLOCK_SIZE + 8)){
                    split_block(b, s);
                } else{
                        b = extend_heap(last, size);
                        if (!b){
                        fprintf(stderr, "ERROR: Couldn't extend heap: %s\n", strerror(errno));
                        return (NULL);
                    }
                }
            }
        }else{
            b = extend_heap(NULL, size);
            if (!b){
                fprintf(stderr, "ERROR: couldn't extend heap: %s\n", strerror(errno));
                return NULL;
            }
            base = b;
        }
        return (b->data + 7);
    }
int main(int argc, char *argv[]){
    
    struct s_block * block = (struct s_block*) d_malloc(sizeof(struct s_block) + 100);
    printf("%d\n", sizeof(*block));
}
