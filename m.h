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

#ifndef M_H
#define M_H

struct string_registry
{
  char* string;
  struct string_registry* next;
};

struct string_registry M_STRING_STORE;
struct string_registry* M_STRING_TAIL;

void
init_registry()
{
  M_STRING_STORE.next = NULL;
  M_STRING_STORE.string = NULL;
  M_STRING_TAIL = &M_STRING_STORE;
}

void
free_registry()
{
  struct string_registry* cur = M_STRING_STORE.next;

  while (cur != NULL)
    {
      free(cur->string);
      struct string_registry* tmp = cur->next;
      free(cur);
      cur = tmp;
    }

  M_STRING_STORE.next = NULL;
  M_STRING_TAIL = &M_STRING_STORE;
}

void
add_to_registry(char* string)
{
  struct string_registry* new = malloc(sizeof(struct string_registry));
  new->next = NULL;
  new->string = string;

  M_STRING_TAIL->next = new;
  M_STRING_TAIL = new;
}


char*
copy_string(const char* str)
{
  char* new = malloc(sizeof(char)*(strlen(str)+1));
  strcpy(new, str);
  add_to_registry(new);
  return new;
}

char*
tag_element(const char* input, const char* tag)
{
  char* new = malloc(sizeof(char)*(strlen(input)+strlen("<></>")+2*strlen(tag)+1));
  sprintf(new, "<%s>%s</%s>", tag, input, tag);
  add_to_registry(new);
  return new;
  
}

char*
concat_element(const char* s, const char* t)
{
  char* new = malloc(sizeof(char)*(strlen(s)+strlen(t)+1));
  sprintf(new, "%s%s", s, t);
  add_to_registry(new);
  return new;
}

char*
handle_unary(const char* u, const char* e)
{

  if (!strcmp(u, "sqrt"))
    return tag_element(e, "msqrt");

  if (!strcmp(u, "sum"))
    {
      char* sum = tag_element("Σ", "mo");
      char* index = tag_element(e, "mrow");
      char* sum_expr = tag_element(concat_element(sum, index),
                                   "munder");
      return sum_expr;
    }

  if (!strcmp(u, "abs"))
    {
      char* bars = tag_element("|", "mo");
      char* abs_expr = concat_element(bars, concat_element(e,bars));
      return abs_expr;
    }
  
  if (!strcmp(u, "op"))
    return tag_element(e, "mo");

  if (!strcmp(u, "text"))
    return tag_element(e, "mtext");

  if (!strcmp(u, "hat"))
    return tag_element(concat_element(e, tag_element("^", "mo")), "mover");

  if (!strcmp(u, "overline"))
    return tag_element(concat_element(e, tag_element("¯", "mo")), "mover");

  if (!strcmp(u, "underline"))
    return tag_element(concat_element(e, tag_element("_", "mo")), "munder");

  if (!strcmp(u, "vec"))
    return tag_element(concat_element(e, tag_element("→", "mo")), "mover");

  if (!strcmp(u, "dot"))
    return tag_element(concat_element(e, tag_element(".", "mo")), "mover");

  if (!strcmp(u, "ddot"))
    return tag_element(concat_element(e, tag_element("..", "mo")), "mover");

  if (!strcmp(u, "ddot"))
    return tag_element(concat_element(e, tag_element("..", "mo")), "mover");

  if (!strcmp(u, "tilde"))
    return tag_element(concat_element(e, tag_element("~", "mo")), "mover");

  if (!strcmp(u, "bb"))
    return concat_element(
                          concat_element("<mstyle mathvariant=\"bold\">",e),
                          "</mstyle>"
                          );

  if (!strcmp(u, "bbb"))
    return concat_element(
                          concat_element("<mstyle mathvariant=\"double-struck\">",e),
                          "</mstyle>"
                          );
  
  if (!strcmp(u, "cc"))
    return concat_element(
                          concat_element("<mstyle mathvariant=\"script\">",e),
                          "</mstyle>"
                          );

  if (!strcmp(u, "tt"))
    return concat_element(
                          concat_element("<mstyle mathvariant=\"monospace\">",e),
                          "</mstyle>"
                          );

  if (!strcmp(u, "fr"))
    return concat_element(
                          concat_element("<mstyle mathvariant=\"fraktur\">",e),
                          "</mstyle>"
                          );

  if (!strcmp(u, "sf"))
    return concat_element(
                          concat_element("<mstyle mathvariant=\"sans-serif\">",e),
                          "</mstyle>"
                          );


  return concat_element(tag_element(u, "mi"), e);
}

char*
handle_binary(const char* b, const char* e1, const char* e2)
{
  if (!strcmp(b, "frac"))
    {
      char* row_e1 = tag_element(e1, "mrow");
      char* row_e2 = tag_element(e2, "mrow");
      char* new = malloc(sizeof(char)*(strlen(row_e1)+strlen(row_e2)+1));
      sprintf(new, "%s%s", row_e1, row_e2);
      char* final = tag_element(new, "mfrac");
      free(new);
      return final;
    }

  if (!strcmp(b, "sum"))
    {
      char* sum = tag_element("Σ", "mo");
      char* index = tag_element(e1, "mrow");
      char* limit = tag_element(e2, "mrow");
      char* sum_expr = tag_element(concat_element(concat_element(sum, index),
                                                  limit), "munderover");
      return sum_expr;
    }

  if (!strcmp(b, "root"))
    return tag_element(concat_element(e1,e2), "mroot");

  if (!strcmp(b, "color"))
    {
      char* open_tag = concat_element(
                                      concat_element("<mstyle mathcolor=\"", e1),
                                      "\">");
      return concat_element(open_tag, concat_element(e2, "</mstyle>"));
    }

  if (!strcmp(b, "super"))
    return tag_element(concat_element(e1,e2), "msup");
  
  if (!strcmp(b, "sub"))
    return tag_element(concat_element(e1,e2), "msub");

  if (!strcmp(b, "under"))
    return tag_element(concat_element(tag_element(e1, "mrow"),tag_element(e2, "mrow")), "munder");

  if (!strcmp(b, "over"))
    return tag_element(concat_element(e1,e2), "mover");
  

}
  

char*
handle_terinary(const char* t, const char* e1, const char* e2, const char* e3)
{
  if (!strcmp(t, "underover"))
    {
      char* op = tag_element(e1, "mo");
      char* under = tag_element(e2, "mrow");
      char* over = tag_element(e3, "mrow");
      char* composite = tag_element(concat_element(concat_element(op, under),
                                                   over), "munderover");

      return composite;
    }
  
}


#endif
