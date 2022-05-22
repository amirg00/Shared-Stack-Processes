#include "Stack.hpp"


Stack* create(){
    Stack *stack = (Stack*)malloc(sizeof(Stack));
    if (!stack){
        return NULL;
    }
    stack->size = 0;
    stack->head = NULL;
    return stack;
}

void PUSH(Stack* stack, const char* text, node_ptr n){
    if (!n){
        return;
    }
    strcpy(n->data, text);
    node_ptr next = stack->head;
    stack->head = n;
    n->next = next;
    stack->size++;
}

char* POP(Stack* stack, int *err_flag) {
    if (isEmpty(stack)) {
        *err_flag = true;
        //fprintf(stderr, "ERROR: Stack is empty, thus cannot pop!");
        return NULL;
    }

    node_ptr rm_node = stack->head;
    char* rm_text = rm_node->data;

    node_ptr head_next = stack->head->next;
    stack->head = head_next;
    munmap(rm_node, sizeof(rm_node));
    //free(rm_node);
    stack->size--;
    return rm_text;
}

char* TOP(Stack* stack, int *err_flag) {
    if (isEmpty(stack)) {
        *err_flag = true;
        return NULL;
    }
    return stack->head->data;
}

bool isEmpty(Stack* stack) {
    return stack->size == 0;
}

void print_stack(Stack* stack) {
    if (isEmpty(stack)) return;
    for (node_ptr curr; curr->next; curr = curr->next) {
        printf("%s\n",curr->data);
    }
}
// Method clears/frees the stack.
void clear(Stack* stack) {
    node_ptr last = stack->head;
    while (last){
        node_ptr tmp = last;
        last = last->next;
        free(tmp);
    }
}
