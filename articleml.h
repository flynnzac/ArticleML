#ifndef ARTICLEML_H
#define ARTICLEML_H
#include <stdio.h>
#include <stdlib.h>
#include <libxml/xmlreader.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

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

struct article
{
  meta metadata;
  bibliography bib;
  bool has_bib;
  section abstract;
  section* sections;
  xmlChar* style;
  uint64_t n_sections;
};

typedef struct article article;

struct stringlist
{
  const xmlChar* string;
  struct stringlist* next;
};

typedef struct stringlist stringlist;

article create_article(const char* input);
void write_article(FILE* outf, article* art);
void free_article(article* art);

xmlChar* alloc_string(const xmlChar* str);
xmlChar* extend_string(xmlChar* current, const xmlChar* add);


#endif
