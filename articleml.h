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


#ifndef ARTICLEML_H
#define ARTICLEML_H

#include <stdio.h>
#include <stdlib.h>
#include <libxml/xmlreader.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

/* Types */

struct bibentry
{
  xmlChar* name;
  xmlChar** tags;
  xmlChar** values;
  uint64_t n_attr;
};

typedef struct bibentry bibentry;

struct bibliography
{
  xmlChar* format;
  bibentry* entries;
  uint64_t n_entries;
  bibentry** used_entries;
  uint64_t n_used_entries;
};

typedef struct bibliography bibliography;

struct meta
{
  xmlChar* title;
  xmlChar* author;
  xmlChar* institution;
  struct tm date;
};

typedef struct meta meta;

struct section
{
  xmlChar* title;
  xmlChar* name;
  xmlChar* html;
};

typedef struct section section;

struct theorem
{
  xmlChar* name;
  xmlChar* statement;
  xmlChar* discussion;
};

typedef struct theorem theorem;

struct article
{
  meta metadata;
  bibliography bib;
  bool has_bib;
  xmlChar* style;
  section abstract;
  section* sections;
  uint64_t n_sections;
  theorem* theorems;
  uint64_t n_theorems;
  theorem* assumptions;
  uint64_t n_assumptions;
  theorem* definitions;
  uint64_t n_definitions;
  
};

typedef struct article article;

struct stringlist
{
  const xmlChar* string;
  struct stringlist* next;
};

typedef struct stringlist stringlist;

struct articleml_format
{
  bool headless;
  bool style;
};

typedef struct articleml_format articleml_format;

article create_article(const char* input);
void write_article(FILE* outf, article* art, articleml_format fmt);
void free_article(article* art);

xmlChar* alloc_string(const xmlChar* str);
xmlChar* extend_string(xmlChar* current, const xmlChar* add);


#endif
