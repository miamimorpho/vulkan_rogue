#define VK_CHECK(x)							\
  do									\
    {									\
      VkResult err = x;							\
      if (err)								\
	{								\
	  printf("VK_CHECK: %d %s %d\n", err ,__FILE__, __LINE__);	\
	  abort();							\
	}								\
    } while (0);
