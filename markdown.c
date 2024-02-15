#include <stdlib.h>
#include <stdio.h>
#include "md4c/md4c-html.h"
#include "articleml.h"

void
callback (const char* chunk, unsigned int output_size, char** output)
{
  if (output == NULL)
    {
      *output = malloc(sizeof(char)*(output_size+1));
      for (size_t i =0; i < output_size; i++)
        (*output)[i] = chunk[i];

      (*output)[output_size] = '\0';
    }
  else
    {
      size_t init_size = 0;

      if (*output != NULL)
        init_size = strlen(*output);
      
      *output = realloc(*output, sizeof(char)*(init_size+output_size+1));
      for (uint64_t i = 0; i < output_size; i++)
        {
          (*output)[init_size+i] = chunk[i];
        }

      (*output)[init_size+output_size] = '\0';
    }
}

char*
render_markdown(const char* input)
{
  char* output = NULL;
  uint64_t wedge = 0;
  while ((input[wedge] == ' ') ||
         (input[wedge] == '\t') ||
         (input[wedge] == '\n'))
    wedge++;

  const char* input_to_use = input + wedge;
  
  int error = md_html(input_to_use,
                      strlen(input_to_use),
                      callback,
                      &output,
                      MD_FLAG_COLLAPSEWHITESPACE | MD_FLAG_NOINDENTEDCODEBLOCKS,
                      0);

  if (error < 0)
    {
      return alloc_string(input);
    }
  else
    {
      return output;
    }
}

  
  
