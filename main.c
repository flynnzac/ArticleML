#define ARTICLEML_BUFFER_SIZE 10000
#include "articleml.h"

int
main (int argc, char** argv)
{
  char buffer[ARTICLEML_BUFFER_SIZE];
  char* input = NULL;

  while(fgets(buffer, sizeof(buffer), stdin)!= NULL)
    {
      if (input == NULL)
        {
          input = alloc_string(buffer);
        }
      else
        {
          input = extend_string(input, buffer);
        }
    }

  if (input == NULL) return 1;
  
  article output = create_article(input);
  write_article(stdout, &output);
  free(input);
  free_article(&output);

  return 0;
}
