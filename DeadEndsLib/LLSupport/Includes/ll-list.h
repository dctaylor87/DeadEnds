#define LISTNOFREE 0
#define LISTDOFREE 1

typedef void (*ELEMENT_DESTRUCTOR)(void*);

extern void llFreeListElement (void *ptr);
