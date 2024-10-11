#define VK_CHECK(x)							\
  do									\
    {									\
      VkResult err = x;							\
      if (err)								\
	{								\
	  printf("VK_CHECK: %d %s %d\n", err ,__FILE__, __LINE__);	\
	  return 1;							\
	}								\
    } while (0);


#define CHECK_GT_ZERO(func_call) do { \
    int result = (func_call); \
    if (result > 0) { \
        fprintf(stderr, "Error: %s returned %d (line %d in %s)\n", \
                #func_call, result, __LINE__, __FILE__); \
        return 1; \
    } \
} while(0)

