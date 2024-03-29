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

#define _XOPEN_SOURCE
#define _GNU_SOURCE
#include "articleml.h"
#include "m_leg.c"


/* Free things. */

void
free_section(section* sec)
{
  xmlFree(sec->title);
  xmlFree(sec->name);
  xmlFree(sec->html);
}

void
free_bibentry(bibentry* entry)
{
  xmlFree(entry->name);
  for (uint64_t i=0; i < entry->n_attr; i++)
    {
      xmlFree(entry->tags[i]);
      xmlFree(entry->values[i]);
    }
  free(entry->tags);
  free(entry->values);
}

void
free_bibliography(bibliography* bib)
{
  if (bib->format != NULL)
    xmlFree(bib->format);
  for (uint64_t i = 0; i < bib->n_entries; i++)
    {
      free_bibentry(&bib->entries[i]);
    }

  free(bib->used_entries);
  free(bib->entries);
}

void
free_meta(meta* met)
{
  if (met->title != NULL)
    xmlFree(met->title);

  if (met->author != NULL)
    xmlFree(met->author);
  
  if (met->institution != NULL)
    xmlFree(met->institution);
}

void
free_article (article* art)
{
  free_meta(&art->metadata);
  if (art->has_bib)
    free_bibliography(&art->bib);

  for (uint64_t i=0; i < art->n_sections; i++)
    free_section(&art->sections[i]);

  free(art->sections);
  xmlFree(art->style);
}


/* Find things. */

bibentry*
find_bibentry(xmlChar* name, bibliography bib)
{
  for (uint64_t i = 0; i < bib.n_entries; i++)
    {
      if (!xmlStrcmp(bib.entries[i].name, name))
        {
          return &bib.entries[i];
        }
    }

  return NULL;
  
}

theorem*
find_theorem(xmlChar* name, const xmlChar* type, article* art, uint64_t* index)
{
  theorem* list;
  uint64_t list_size;

  if (!xmlStrcmp(type, "theorem"))
    {
      list = art->theorems;
      list_size = art->n_theorems;
    }
  else if (!xmlStrcmp(type, "assumption"))
    {
      list = art->assumptions;
      list_size = art->n_assumptions;
    }
  else if (!xmlStrcmp(type, "definition"))
    {
      list = art->definitions;
      list_size = art->n_definitions;
    }
  
  for (uint64_t i=0; i < list_size; i++)
    {
      if (!xmlStrcmp(list[i].name, name))
        {
          *index = i;
          return &list[i];
        }
    }
  return NULL;
}

bool
is_bibentry_used (bibentry* entry, bibliography* bib)
{
  if (bib->used_entries == NULL)
    {
      return false;
    }

  for (uint64_t i=0; i < bib->n_used_entries; i++)
    {
      if (entry == bib->used_entries[i])
        return true;
    }

  return false;
}

void
add_bibentry_used(bibentry* entry, bibliography* bib)
{
  if (is_bibentry_used(entry, bib))
    return;

  if (bib->used_entries == NULL)
    {
      bib->used_entries = malloc(sizeof(bibentry*));
    }
  else
    {
      bib->used_entries = realloc(bib->used_entries, sizeof(bibentry*)*(bib->n_used_entries+1));
    }
  
  bib->used_entries[bib->n_used_entries] = entry;
  bib->n_used_entries++;
}
      


section*
find_section(xmlChar* name, article* art)
{
  for (uint64_t i = 0; i < art->n_sections; i++)
    {
      if (!xmlStrcmp(art->sections[i].name, name))
        return &art->sections[i];
      
    }

  return NULL;
}

/* Basic string utilities */

xmlChar*
alloc_string(const xmlChar* str)
{
  xmlChar* mem = malloc(sizeof(xmlChar)*(strlen(str)+1));
  strcpy(mem, str);

  return mem;
}

xmlChar*
extend_string(xmlChar* current, const xmlChar* add)
{
  uint64_t init_length = xmlStrlen(current);
  current = realloc(current, (init_length+xmlStrlen(add)+1)*sizeof(xmlChar));
  for (uint64_t i=0; i < xmlStrlen(add); i++)
    {
      current[init_length+i] = add[i];
    }
  current[init_length+xmlStrlen(add)] = '\0';
  return current;
}

size_t
length_of_stringlist(stringlist* head)
{
  size_t sz = 0;
  stringlist* cur = head;
  while (cur != NULL)
    {
      sz += xmlStrlen(cur->string);
      cur = cur->next;
    }

  return sz;
}

xmlChar*
add_stringlist(xmlChar* current, stringlist* head)
{
  stringlist* cur = head;
  size_t len = xmlStrlen(current);
  size_t add_len = length_of_stringlist(head);
  current = realloc(current, sizeof(xmlChar)*
                    (xmlStrlen(current) +
                     add_len + 1));

  while (cur != NULL)
    {
      for (uint64_t j=0; j < xmlStrlen(cur->string); j++)
        {
          current[len+j] = cur->string[j];
        }
      len += xmlStrlen(cur->string);
      cur = cur->next;
    }

  current[len] = '\0';
  return current;
}

stringlist
create_string(const xmlChar* string)
{
  stringlist to_add;
  to_add.string = string;
  to_add.next = NULL;

  return to_add;
}

stringlist*
append_to_stringlist(stringlist* head, const xmlChar* string)
{
  stringlist* new = malloc(sizeof(stringlist));
  new->string = string;
  new->next = NULL;

  head->next = new;
  return new;
}

void
free_stringlist(stringlist* head)
{
  stringlist* tail = head->next;
  stringlist* cur = NULL;
  while (tail != NULL)
    {
      cur = tail;
      tail = tail->next;
      free(cur);
    }
}


/* Create references */

char*
create_section_link(section* sec)
{
  char* section_link = malloc(sizeof(char)*(strlen(sec->name)+
                                            strlen(sec->title)+
                                            strlen("<a href=\"#\"></a>")+1));

  sprintf(section_link, "<a href=\"#%s\">%s</a>", sec->name, sec->title);
  return section_link;
}

char*
create_natural_inline_cite(bibentry* entry)
{
  const xmlChar* author = NULL;
  const xmlChar* year = NULL;

  for (uint64_t i=0; i < entry->n_attr; i++)
    {
      if (!xmlStrcmp(entry->tags[i], (const xmlChar*) "author"))
        {
          author = entry->values[i];
        }
      else if (!xmlStrcmp(entry->tags[i], (const xmlChar*) "year"))
        {
          year = entry->values[i];
        }
    }

  if (author == NULL)
    {
      author = entry->name;
    }

  xmlChar* cite = alloc_string(author);

  if (year != NULL)
    {
      cite = realloc(cite, sizeof(xmlChar)*(xmlStrlen(cite) +
                                            strlen(" ()") +
                                            xmlStrlen(year) +
                                            1));
      cite[xmlStrlen(author)] = ' ';
      cite[xmlStrlen(author)+1] = '(';
      for (uint64_t i=0; i < xmlStrlen(year); i++)
        {
          cite[xmlStrlen(author)+2+i] = year[i];
        }
      cite[xmlStrlen(author)+2+xmlStrlen(year)] = ')';
      cite[xmlStrlen(author)+3+xmlStrlen(year)] = '\0';
    }

  return cite;
}


xmlChar*
create_open_tag(xmlNodePtr node)
{

  xmlChar* name = alloc_string(node->name);
  name = extend_string(name, " ");
  xmlAttr* attr = node->properties;
  while (attr != NULL)
    {
      stringlist to_add = create_string(attr->name);
      stringlist* tail = &to_add;

      tail = append_to_stringlist(tail, "=\"");

      xmlChar* prop = xmlGetProp(node, attr->name);
      tail = append_to_stringlist(tail, prop);
      tail = append_to_stringlist(tail, "\" ");
      
      name = add_stringlist(name, &to_add);

      free_stringlist(&to_add);
      xmlFree(prop);
      attr = attr->next;
    }

  return name;
}

/* Parsers */

xmlChar*
copy_section_html(xmlNodePtr section_node, xmlDocPtr doc)
{
  char* output = NULL;
  size_t sz = 0;
  FILE* format_stream  = open_memstream(&output, &sz);
  xmlElemDump(format_stream, doc, section_node);
  fclose(format_stream);
  
  return (xmlChar*) output;
}

meta
parse_meta(xmlNodePtr meta_node, xmlDocPtr doc)
{
  meta output;
  output.title = NULL;
  output.author = NULL;
  output.institution = NULL;

  xmlNodePtr cur = meta_node->xmlChildrenNode;
  while (cur != NULL)
    {
      if (!xmlStrcmp(cur->name, (const xmlChar*) "title"))
        {
          xmlChar* title = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
          output.title = title;
        }
      else if (!xmlStrcmp(cur->name, (const xmlChar*) "author"))
        {
          xmlChar* author = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
          output.author = author;
        }
      else if (!xmlStrcmp(cur->name, (const xmlChar*) "institution"))
        {
          xmlChar* institution = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
          output.institution = institution;
        }
      else if (!xmlStrcmp(cur->name, (const xmlChar*) "date"))
        {
          xmlChar* date = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
          struct tm tm_date;

          if (!xmlStrcmp(date, (const xmlChar*) "today"))
            {
              time_t now = time(NULL);
              tm_date = *localtime(&now);
            }
          else
            {
              strptime(date, "%Y-%m-%d", &tm_date);
            }
          xmlFree(date);
          
          output.date = tm_date;
        }
      cur = cur->next;
    }

  return output;
}

bibentry
parse_entry(xmlNodePtr entry_node, xmlDocPtr doc)
{
  bibentry output;
  output.tags = NULL;
  output.values = NULL;
  output.n_attr = 0;
  
  output.name = xmlGetProp(entry_node, "name");

  xmlNodePtr cur = entry_node->xmlChildrenNode;
  while (cur != NULL)
    {
      if (xmlStrcmp(cur->name, (const xmlChar*) "text"))
        {
          output.n_attr++;
          output.tags = realloc(output.tags, sizeof(xmlChar*)*(output.n_attr));
          output.values = realloc(output.values, sizeof(xmlChar*)*(output.n_attr));
          output.tags[output.n_attr-1] = alloc_string(cur->name);
          output.values[output.n_attr-1] = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
        }
      cur = cur->next;
    }

  return output;
  
}

bibliography
parse_bibliography(xmlNodePtr bib_node, xmlDocPtr doc)
{

  bibliography output;
  output.format = NULL;
  output.entries = NULL;
  output.n_entries = 0;
  output.used_entries = NULL;
  output.n_used_entries = 0;
  
  xmlChar* bibfile = xmlGetProp(bib_node, "file");
  if (bibfile == NULL)
    {
      return output;
    }

  char* buffer = NULL;
  size_t sz = 0;
  FILE* format_stream  = open_memstream(&buffer, &sz);
  xmlElemDump(format_stream, doc, bib_node);
  output.format = buffer;
  fclose(format_stream);

  xmlDocPtr bibdoc = xmlParseFile(bibfile);
  xmlFree(bibfile);
  
  xmlNodePtr cur = xmlDocGetRootElement(bibdoc)->xmlChildrenNode;
  while (cur != NULL)
    {
      if (!xmlStrcmp(cur->name, (const xmlChar*) "entry"))
        {
          bibentry e = parse_entry(cur, bibdoc);
          output.n_entries++;
          if (output.entries == NULL)
            output.entries = malloc(sizeof(bibentry));
          else
            output.entries = realloc(output.entries,
                                     sizeof(bibentry)*(output.n_entries));
          
          output.entries[output.n_entries-1] = e;
        }
      cur = cur->next;
    }
  
  return output;
}

/* Deal with references. */

xmlChar*
_parse_section(xmlNodePtr node, xmlDocPtr doc, article* art)
{

  xmlChar* output = malloc(sizeof(xmlChar));
  output[0] = '\0';

  xmlNodePtr cur = node->xmlChildrenNode;

  if (cur == NULL)
    {
      cur = node;
    }

  /* Leave references on first pass.  Fill in references in second pass. */

  while (cur != NULL)
    {
      if (!xmlStrcmp(cur->name, (const xmlChar*) "text"))
        {
          output = extend_string(output, cur->content);
        }
      else if (!xmlStrcmp(cur->name, (const xmlChar*) "cite"))
        {
          xmlChar* content = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
          bibentry* entry = find_bibentry(content, art->bib);
          xmlFree(content);
          
          add_bibentry_used(entry, &art->bib);

          xmlChar* text = (xmlChar*) create_natural_inline_cite(entry);

          stringlist head = create_string("<a href=\"#");
          stringlist* tail = &head;
          tail = append_to_stringlist(tail, entry->name);
          tail = append_to_stringlist(tail, "\">");
          tail = append_to_stringlist(tail, text);
          tail = append_to_stringlist(tail, "</a>");

          output = add_stringlist(output, &head);
          free_stringlist(&head);
          xmlFree(text);
        }
      else if (!xmlStrcmp(cur->name, (const xmlChar*) "theorem") ||
               !xmlStrcmp(cur->name, (const xmlChar*) "assumption") ||
               !xmlStrcmp(cur->name, (const xmlChar*) "definition"))
        {
          xmlChar* name = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
          uint64_t thm_no;
          theorem* thm = find_theorem(name, cur->name, art, &thm_no);
          if (thm == NULL)
            {
              xmlFree(name);
              goto next_tag;
            }
              
          char thm_no_str[21];
          sprintf(thm_no_str, "%llu", thm_no+1);
          xmlFree(name);

          bool discussion = false;
          bool statement = false;
          bool refdisc = false;

          char* label = "";
          if (!xmlStrcmp(cur->name, (const xmlChar*) "theorem"))
            label = "Theorem";
          else if (!xmlStrcmp(cur->name, (const xmlChar*) "assumption"))
            label = "Assumption";
          else if (!xmlStrcmp(cur->name, (const xmlChar*) "definition"))
            label = "Definition";

          xmlChar* use_discussion = xmlGetProp(cur, "discussion");
          xmlChar* use_statement = xmlGetProp(cur, "statement");
          xmlChar* use_refdisc = xmlGetProp(cur, "refdisc");

          if ((use_discussion != NULL) && (!xmlStrcmp(use_discussion, (const xmlChar*) "yes")))
            {
              discussion = true;
            }
          if ((use_statement != NULL) && (!xmlStrcmp(use_statement, (const xmlChar*) "yes")))
            {
              statement = true;
            }
          if ((use_refdisc != NULL) && (!xmlStrcmp(use_refdisc, (const xmlChar*) "yes")))
            {
              refdisc = true;
            }

          xmlFree(use_discussion);
          xmlFree(use_statement);
          xmlFree(use_refdisc);
          
          stringlist head;
          stringlist* tail;
          if (discussion || statement)
            {
              head = create_string("<div class=\"theorem\" id=\"");
              tail = &head;
              tail = append_to_stringlist(tail, thm->name);

              if (!statement)
                {
                  tail = append_to_stringlist(tail, "-disc");
                }
              tail = append_to_stringlist(tail, "\">");
              tail = append_to_stringlist(tail, "<div class=\"theorem-title\">");
              tail = append_to_stringlist(tail, label);
              tail = append_to_stringlist(tail, " ");
              tail = append_to_stringlist(tail, thm_no_str);
              tail = append_to_stringlist(tail, "</div>");

              if (statement && (thm->statement != NULL))
                {
                  tail = append_to_stringlist(tail, "<div class=\"theorem-statement\">");
                  tail = append_to_stringlist(tail, thm->statement);
                  tail = append_to_stringlist(tail, "</div>");
                }
              if (discussion && (thm->discussion != NULL))
                {
                  tail = append_to_stringlist(tail, "<div class=\"theorem-discussion\">");
                  tail = append_to_stringlist(tail, thm->discussion);
                  tail = append_to_stringlist(tail, "</div>");
                }
              tail = append_to_stringlist(tail, "</div>");
            }
          else
            {
              head = create_string("<a href=\"#");
              tail = &head;
              tail = append_to_stringlist(tail, thm->name);

              if (refdisc)
                {
                  tail = append_to_stringlist(tail, "-disc");
                }
              tail = append_to_stringlist(tail, "\">");
              tail = append_to_stringlist(tail, label);
              tail = append_to_stringlist(tail, " ");
              tail = append_to_stringlist(tail, thm_no_str);
              tail = append_to_stringlist(tail, "</a>");
            }

          output = add_stringlist(output, &head);
          free_stringlist(&head);
          
        }
      else if (!xmlStrcmp(cur->name, (const xmlChar*) "ref"))
        {
          xmlChar* content = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
          section* sec = find_section(content, art);
          xmlFree(content);
          if (sec != NULL)
            {
              xmlChar* text = (xmlChar*) create_section_link(sec);
              output = extend_string(output, text);
              xmlFree(text);
            }
        }
      else if (!xmlStrcmp(cur->name, (const xmlChar*) "m"))
        {
          xmlChar* math_content = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
          char* mathml = NULL;
          FILE* input_math = fmemopen(math_content, xmlStrlen(math_content), "r");
          xmlChar* type = xmlGetProp(cur, "style");
          char* line = NULL;
          size_t sz = 0;
          while (getline(&line, &sz, input_math) >= 0)
            {
              char* mathml = m_parse_line((char*) line);
              if (mathml == NULL)
                {
                  output = extend_string(output, line);
                  continue;
                }
              
              stringlist head;
              if (!xmlStrcmp(type, "display"))
                {
                  head = create_string("<div class=\"math-display\">");
                }
              else
                {
                  head = create_string("<span class=\"math-inline\">");
                }

              stringlist* tail = &head;
              tail = append_to_stringlist(tail, "<math>");
              tail = append_to_stringlist(tail, mathml);
              tail = append_to_stringlist(tail, "</math>");

              if (!xmlStrcmp(type, "display"))
                {
                  tail = append_to_stringlist(tail, "</div>");
                }
              else
                {
                  tail = append_to_stringlist(tail, "</span>");
                }

              output = add_stringlist(output, &head);
              free(mathml);
              free_stringlist(&head);
            }
          xmlFree(math_content);
          if (type != NULL)
            {
              xmlFree(type);
            }

          
        }
      else if (!xmlStrcmp(cur->name, (const xmlChar*) "tag"))
        {
          xmlChar* tag = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
          stringlist head = create_string("&lt;");
          stringlist* tail = &head;
          tail = append_to_stringlist(tail, tag);
          tail = append_to_stringlist(tail, "&gt;");

          output = add_stringlist(output, &head);
          free_stringlist(&head);
          xmlFree(tag);
        }
      else if (!xmlStrcmp(cur->name, (const xmlChar*) "gt"))
        {
          output = extend_string(output, "&gt;");
        }
      else if (!xmlStrcmp(cur->name, (const xmlChar*) "lt"))
        {
          output = extend_string(output, "&lt;");
        }
      else
        {
          if (cur->type == XML_CDATA_SECTION_NODE)
            {
              xmlChar* text = xmlNodeListGetString(doc, cur, 1);
              output = extend_string(output, text);
              xmlFree(text);
            }
          else
            {
              xmlChar* tag = create_open_tag(cur);
              stringlist head = create_string("<");
              stringlist* tail = &head;

              tail = append_to_stringlist(tail, tag);
              tail = append_to_stringlist(tail, ">");
              xmlChar* child_text = NULL;
              if (cur->children != NULL)
                {
                  child_text = _parse_section(cur, doc, art);
                  tail = append_to_stringlist(tail, child_text);
                }

              tail = append_to_stringlist(tail, "</");
              tail = append_to_stringlist(tail, cur->name);
              tail = append_to_stringlist(tail, ">");

              output = add_stringlist(output, &head);

              free_stringlist(&head);
              xmlFree(tag);
              if (child_text != NULL)
                xmlFree(child_text);
            }

        }
    next_tag: cur = cur->next;
    }

  return output;

}

char*
parse_section(xmlChar* html, article* art)
{
  xmlDocPtr doc = xmlReadDoc(html,
                             NULL,
                             NULL,
                             0);

  xmlNodePtr cur = xmlDocGetRootElement(doc);

  return (char*) _parse_section(cur, doc, art);
}

/* theorems, definitions, and assumptions */

void
structure_theorem(theorem* thm, xmlNodePtr cur, xmlDocPtr doc)
{
  thm->statement = NULL;
  thm->discussion = NULL;

  xmlNodePtr tmp = cur->xmlChildrenNode;
  while (tmp != NULL)
    {
      if (!xmlStrcmp(tmp->name, (const xmlChar*) "statement"))
        {
          thm->statement = copy_section_html(tmp, doc);
        }
      else if (!xmlStrcmp(tmp->name, (const xmlChar*) "discussion"))
        {
          thm->discussion = copy_section_html(tmp, doc);
        }
      tmp = tmp->next;
    }
}

/* For writing out. */

void
write_bibliography(FILE* outf, xmlNodePtr cur, xmlDocPtr doc, bibentry* entry)
{
  while (cur != NULL)
    {
      if (!xmlStrncmp("source-", cur->name, strlen("source-")))
        {
          const xmlChar* name = cur->name + strlen("source-");
          for (uint64_t i=0; i < entry->n_attr; i++)
            {
              if (!xmlStrcmp(name, entry->tags[i]))
                {
                  fprintf(outf, "%s", entry->values[i]);
                  break;
                }
            }
        }
      else if (!xmlStrcmp(cur->name, (const xmlChar*) "text"))
        {
          fprintf(outf, "%s", cur->content);
        }
      else
        {
          xmlChar* tag = create_open_tag(cur);
          fprintf(outf, "<");
          fprintf(outf, "%s", tag);
          fprintf(outf, ">");
          
          if (cur->children != NULL)
            {
              write_bibliography(outf, cur->children, doc, entry);
            }
          fprintf(outf, "</");
          fprintf(outf, "%s", cur->name);
          fprintf(outf, ">");
          xmlFree(tag);
        }
      cur = cur->next;
    }
}


void
write_article(FILE* outf, article* art, articleml_format fmt)
{
  if (!fmt.headless)
    {
      fprintf(outf, "<html>");
      fprintf(outf, "<head>");
    }

  fprintf(outf, "<style>");
  if (fmt.style)
    {

      fprintf(outf, "#main-content { width: 70%; margin-left:15%; line-height: 1.8 } ");
      fprintf(outf, "#title { font-size: 2em; text-align: center; line-height: 1.8} ");
      fprintf(outf, ".math-display { text-align: center; font-size: 1em }");
      fprintf(outf, ".math-inline { font-size: 1em }");
    }
  
  if (art->style != NULL)
    {
      fprintf(outf, "%s", art->style);
    }

  fprintf(outf, "</style>");  
  if (!fmt.headless)
    {
      fprintf(outf, "</head><body>");
    }
  
  fprintf(outf, "<div id=\"main-content\">");
  fprintf(outf, "<header>");
  
  if (art->metadata.title != NULL)
    fprintf(outf, "<div id=\"title\">%s</div>", art->metadata.title);

  if (art->metadata.author != NULL)
    fprintf(outf, "<span id=\"author\">%s</span><br/>", art->metadata.author);
  
  if (art->metadata.institution != NULL)
    fprintf(outf, "<span id=\"institution\">%s</span><br/>", art->metadata.institution);

  fprintf(outf, "<span id=\"date\">%d-%02d-%02d</span><br/>", 
          art->metadata.date.tm_year+1900,
          art->metadata.date.tm_mon+1, art->metadata.date.tm_mday+1);
  
  fprintf(outf, "</header>");

  if (art->abstract.html != NULL)
    {
      fprintf(outf, "<section id=\"abstract\" class=\"abstract\">");
      fprintf(outf, "<h3>Abstract</h3>");
      fprintf(outf, "%s", art->abstract.html);
      fprintf(outf, "</section>");
    }

  for (uint64_t i=0; i < art->n_sections; i++)
    {
      fprintf(outf, "<section id=\"%s\" class=\"normal-section\">", art->sections[i].name);
      fprintf(outf, "<h3>%s</h3>", art->sections[i].title);
      if (art->sections[i].html != NULL)
        fprintf(outf, "%s", art->sections[i].html);
      fprintf(outf, "</section>");
    }

  if (art->has_bib && (art->bib.n_used_entries > 0))
    {
      fprintf(outf, "<section class=\"bibliography\">");
      fprintf(outf, "<h3>References</h3>");
      xmlDocPtr bibdoc = xmlReadDoc(art->bib.format,
                                    NULL,
                                    NULL,
                                    0);

      xmlNodePtr cur = xmlDocGetRootElement(bibdoc)->xmlChildrenNode;
      for (uint64_t i=0; i < art->bib.n_used_entries; i++)
        {
          fprintf(outf, "<div id=\"");
          fprintf(outf, "%s", art->bib.used_entries[i]->name);
          fprintf(outf, "\">");
          write_bibliography(outf, cur, bibdoc, art->bib.used_entries[i]);
          fprintf(outf, "</div>");
        }
      fprintf(outf, "</section>");
    }
  fprintf(outf, "</div>");
  if (!fmt.headless)
    {
      fprintf(outf, "</body></html>");
    }
}

/* Handle theorems */

void
append_to_theorem_list(theorem thm, uint64_t* counter, theorem** list)
{
  (*counter) = (*counter) + 1;
  if (*list == NULL)
    *list = malloc(sizeof(theorem));
  else
    *list = realloc(*list, sizeof(theorem)*(*counter));

  (*list)[*counter-1] = thm;
}
  

void
append_theorem(xmlNodePtr cur, xmlDocPtr doc, article* output)
{
  theorem thm;
  thm.name = xmlGetProp(cur, "name");
  structure_theorem(&thm, cur, doc);

  if (!xmlStrcmp(cur->name, "theorem"))
    {
      append_to_theorem_list(thm, &output->n_theorems, &output->theorems);
    }
  else if (!xmlStrcmp(cur->name, "assumption"))
    {
      append_to_theorem_list(thm, &output->n_assumptions, &output->assumptions);
    }
  else if (!xmlStrcmp(cur->name, "definition"))
    {
      append_to_theorem_list(thm, &output->n_definitions, &output->definitions);
    }

}
void
parse_all_theoremlist(theorem* list, uint64_t n_theorems, article* output)
{
    for (uint64_t i=0; i < n_theorems; i++)
    {
      if (list[i].statement != NULL)
        list[i].statement = parse_section(list[i].statement, output);
      if (list[i].discussion != NULL)
        list[i].discussion = parse_section(list[i].discussion, output);
    }

}

/* Build document object */

article
create_article(const char* input)
{
  xmlDocPtr doc;
  xmlNodePtr cur;
  article output;

  output.has_bib = false;
  output.sections = NULL;
  output.n_sections = 0;
  output.theorems = NULL;
  output.n_theorems = 0;
  output.assumptions = NULL;
  output.n_assumptions = 0;
  output.definitions = NULL;
  output.n_definitions = 0;
  
  output.style = NULL;
  
  output.abstract.html = NULL;

  doc = xmlReadDoc(input, NULL, NULL, 0);
  cur = xmlDocGetRootElement(doc);

  if (xmlStrcmp(cur->name, "article"))
    {
      fprintf(stderr, "Not a valid articleML document.  Root tag should be <article>.\n");
      exit(1);
    }
  cur = cur->xmlChildrenNode;

  while (cur != NULL)
    {
      if (!xmlStrcmp(cur->name, (const xmlChar*) "meta"))
        {
          output.metadata = parse_meta(cur, doc);
        }
      else if (!xmlStrcmp(cur->name, (const xmlChar*) "style"))
        {
          output.style = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
        }
      else if (!xmlStrcmp(cur->name, (const xmlChar*) "abstract"))
        {
          output.abstract.title = (xmlChar*) alloc_string("Abstract");
          output.abstract.name = (xmlChar*) alloc_string("Abstract");
          output.abstract.html = copy_section_html(cur, doc);
          
        }
      else if (!xmlStrcmp(cur->name, (const xmlChar*) "section"))
        {
          section sec;
          sec.name = xmlGetProp(cur, "name");
          sec.title = xmlGetProp(cur, "title");

          if (sec.name == NULL)
            {
              sec.name = alloc_string("");
            }
          
          if (sec.title == NULL)
            {
              sec.title = alloc_string(sec.name);
            }

          sec.html = copy_section_html(cur, doc);

          output.n_sections += 1;

          if (output.sections == NULL)
            output.sections = malloc(sizeof(section));
          else
            output.sections = realloc(output.sections, sizeof(section)*output.n_sections);

          output.sections[output.n_sections-1] = sec;
        }
      else if (!xmlStrcmp(cur->name, (const xmlChar*) "theorem") ||
               (!xmlStrcmp(cur->name, (const xmlChar*) "assumption")) ||
                (!xmlStrcmp(cur->name, (const xmlChar*) "definition")))
        {
          append_theorem(cur, doc, &output);
        }
      else if (!xmlStrcmp(cur->name, (const xmlChar*) "bibliography"))
        {
          output.bib = parse_bibliography(cur, doc);
          output.has_bib = true;
        }
      cur = cur->next;
    }

  parse_all_theoremlist(output.theorems, output.n_theorems, &output);
  parse_all_theoremlist(output.assumptions, output.n_assumptions, &output);
  parse_all_theoremlist(output.definitions, output.n_definitions, &output);
  
  
  for (uint64_t i=0; i < output.n_sections; i++)
    {
      output.sections[i].html = parse_section(output.sections[i].html, &output);
    }

  if (output.abstract.html != NULL)
    output.abstract.html = parse_section(output.abstract.html, &output);


  return output;

}

