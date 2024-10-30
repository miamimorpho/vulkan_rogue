#include <stdio.h>
#include <stdlib.h>
#include "config.h"

char* configReadFile(const char* filename) {
  FILE* file = fopen(filename, "r");
  if (file == NULL) {
    perror("Error opening file");
    return NULL;
  }

  // Get file size
  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);
  
  // Allocate memory for the string
  char* content = malloc(file_size + 1);
  if (content == NULL) {
    fclose(file);
    return NULL;
  }

  // Read file contents
  size_t bytes_read = fread(content, 1, file_size, file);
  if (bytes_read < (unsigned int)file_size) {
    perror("Error reading file");
    free(content);
    fclose(file);
    return NULL;
  }
  
  // Null-terminate the string
  content[file_size] = '\0';
  
  fclose(file);
  return content;
}
