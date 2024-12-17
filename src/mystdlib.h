#ifndef MYSTDLIB
#define MYSTDLIB

#define container_of(ptr, type, member) ((type*)((char *)(ptr) - offsetof(type, member)))

struct MyBufferMeta{
    size_t top;
    size_t size;
};

static inline struct MyBufferMeta* myBufferMeta(void* buffer){
    return ( ( struct MyBufferMeta*)buffer ) - 1;
}

static inline void* myBufferMalloc(size_t nmemb, size_t stride){
    if(stride && nmemb > SIZE_MAX / stride){
        fprintf(stderr, "MyBufferMalloc:integer overflow\n");
        return NULL;
    }
    
    struct MyBufferMeta* info_ptr = 
        malloc( sizeof(struct MyBufferMeta) + stride * nmemb);
    if(info_ptr == NULL){
        fprintf(stderr, "MyBufferMalloc:malloc error\n");
        return NULL;
    }
    info_ptr->top = 0;
    info_ptr->size = stride * nmemb;

    return (void*)(info_ptr +1);
}

static inline void myBufferFree(void* raw_ptr){
    if(raw_ptr == NULL) return;
    free(myBufferMeta(raw_ptr));
}

static inline void* myBufferPush(void* head, void* val_ptr, size_t stride)
{
    struct MyBufferMeta* info = myBufferMeta(head);
    if(info->top >= info->size / stride){
        fprintf(stderr, "myBufferPush: buffer overflow %zu %zu\n", info->top, info->size);
        return NULL;
    }

    void* dst = (char*)head + (info->top * stride);
    if(val_ptr != NULL) memcpy(dst, val_ptr, stride);
    info->top++;
    return dst;
}

static inline int myBufferPop(void* root_ptr){
    struct MyBufferMeta* info = myBufferMeta(root_ptr);
    if(info->top == 0) return -1;

    info->top--;
    return 0;
}

#endif // MYSTDLIB
