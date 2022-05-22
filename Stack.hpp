#include <string.h>
#include <iostream>
#include <sys/mman.h>
//#include "allocator.hpp"

#define _MMAP_SIZE 1000*1024     // Approx. 1 mb
#define MAX_TEXT_SIZE 1024       // BYTES

typedef struct Node{
    char data[MAX_TEXT_SIZE];
    struct Node *next;
} node, *node_ptr;

struct Stack{
    int size;
    node_ptr head;
};

//*****************************/
//========== Methods ==========/
//*****************************/

Stack* create();
void PUSH(Stack* stack, const char* text, node_ptr n);
char* POP(Stack* stack, int *err_flag);
char* TOP(Stack* stack, int *err_flag);
bool isEmpty(Stack* stack);
void clear(Stack* stack);
void print_stack(Stack* stack);