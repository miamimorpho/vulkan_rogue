#ifndef MYSTDLIB
#define MYSTDLIB

#define container_of(ptr, type, member) ((type*)((char *)(ptr) - offsetof(type, member)))

struct MyBufferMeta{
    size_t top;
    size_t size;
    char data[];
};

static inline struct MyBufferMeta* myBufferMeta(void* buffer_data){
    return container_of(buffer_data, struct MyBufferMeta, data);
}

static inline void* myBufferMalloc(size_t nmemb, size_t stride){
    
    if(stride && nmemb > SIZE_MAX / stride){
        fprintf(stderr, "MyBufferMalloc:integer overflow\n");
        return NULL;
    }
    size_t total_size = 
        sizeof(struct MyBufferMeta) + nmemb * stride;

    struct MyBufferMeta* meta = malloc(total_size);
    if(meta == NULL) return NULL;
    meta->top = 0;
    meta->size = nmemb * stride;
    memset(meta->data, 0, meta->size);

    return meta->data;
}

static inline int myBufferFree(void* buffer){
    if(buffer == NULL) return -1;
    struct MyBufferMeta* meta = myBufferMeta(buffer);
    free(meta);
    return 0;
}

static inline void* myBufferPush(void* head, const void* val_ptr, size_t stride)
{
    struct MyBufferMeta* info = myBufferMeta(head);
    if(info->top * stride >= info->size){
        fprintf(stderr, "myBufferPush: buffer overflow %zu %zu\n", info->top, info->size);
        return NULL;
    }

    void* dst = (char*)head + (info->top * stride);
    if(val_ptr != NULL) memcpy(dst, val_ptr, stride);
    info->top++;
    return dst;
}

static inline void* myBufferPop(void* root_ptr, size_t stride){
    struct MyBufferMeta* info = myBufferMeta(root_ptr);
    if(info->top == 0) return NULL;
    info->top--;
    return (char*)root_ptr + (info->top * stride);
}

#endif // MYSTDLIB
