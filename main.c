#define ARTICLEML_BUFFER_SIZE 10000
#include "articleml.h"

/*

  Copyright 2024 Zach Flynn <zlflynn@gmail.com>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the following
  disclaimer in the documentation and/or other materials provided
  with the distribution.

  3. Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived
  from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
  OF THE POSSIBILITY OF SUCH DAMAGE.

*/


int
main (int argc, char** argv)
{
  char buffer[ARTICLEML_BUFFER_SIZE];
  char* input = NULL;

  char c;
  articleml_format fmt;
  fmt.headless = false;
  fmt.style = true;
  
  while ((c = getopt(argc, argv, "ns")) != -1)
    {
      switch (c)
        {
        case 'n':
          fmt.headless = true;
          break;
        case 's':
          fmt.style = false;
          break;
        case '?':
          fprintf(stderr, "Unrecognized option: %c\n", optopt);
          return 1;
        default:
          abort();
        }
    }

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
  write_article(stdout, &output, fmt);
  free(input);
  free_article(&output);

  return 0;
}
