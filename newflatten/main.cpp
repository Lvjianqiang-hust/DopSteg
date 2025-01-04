#include <cstdio>
#include <cstdlib>
#include <cstring>

struct Node {
    char log[10];
    struct Node* next;
};

struct Node* process(struct Node* tail) {
    /* int a = 1; */
    /* int b = 114; */
    /* printf("init\n"); */
    /* if (a == b) { */
    /*     b = 115; */
    /*     printf("a == b\n"); */
    /* } else { */
    /*     b = 113; */
    /*     printf("a != b\n"); */
    /* } */
    /* b += a; */
    /* printf("over!\n"); */
    /* return tail; */
    char log[10];
    char op[8];
    struct Node* head;
    struct Node* node;
    node=(struct Node*)malloc(sizeof(struct Node));
    scanf("%s",op);


    if(!strcmp(op,"push")){
        scanf("%s",log);
        strcpy(node->log,log);
        head=tail->next;
        tail->next=node;
        node->next=head;
        tail=node;
        printf("Push node log is %s.\n",tail->log);
    }else if(!strcmp(op,"pop")){
        head=tail->next;
        node=head->next;
        tail->next=node;
        printf("Pop node log is %s\n",head->log);
        free(head);
    }
    return tail;
}

int main() {
    struct Node* tail = (struct Node*)malloc(sizeof(struct Node));
    strcpy(tail->log, "Head");
    tail->next = tail;
    while (true) {
        tail = process(tail);
    }
}
