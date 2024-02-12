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
parse_section_html(xmlNodePtr section_node, xmlDocPtr doc, article* art)
{
  xmlChar* output = malloc(sizeof(xmlChar));
  output[0] = '\0';

  xmlNodePtr cur = section_node->xmlChildrenNode;

  if (cur == NULL)
    {
      cur = section_node;
    }

  while (cur != NULL)
    {
      if (!xmlStrcmp(cur->name, (const xmlChar*) "text"))
        {
          output = extend_string(output, cur->content);
        }
      else if ((!xmlStrcmp(cur->name, (const xmlChar*) "m")) ||
               (cur->type == XML_CDATA_SECTION_NODE))
        {
          /* Things to dump as is */
          char* buffer = NULL;
          size_t sz = 0;
          FILE* format_stream  = open_memstream(&buffer, &sz);
          xmlElemDump(format_stream, doc, cur);
          output = extend_string(output, buffer);
          fclose(format_stream);
          free(buffer);
        }
      
      else
        {
          /* Copy attributes for tag. */
          xmlChar* tag = create_open_tag(cur);

          stringlist head = create_string("<");
          stringlist* tail = &head;

          tail = append_to_stringlist(tail, tag);
          tail = append_to_stringlist(tail, ">");

          xmlChar* child_text = NULL;
          if (cur->children != NULL)
            {
              child_text = parse_section_html(cur, doc, art);
              tail = append_to_stringlist(tail, child_text);
            }

          tail = append_to_stringlist(tail, "</");
          tail = append_to_stringlist(tail, cur->name);
          tail = append_to_stringlist(tail, ">");
          
          output = add_stringlist(output, &head);

          if (child_text != NULL)
            xmlFree(child_text);

          free_stringlist(&head);
          xmlFree(tag);
        }
      cur = cur->next;
    }


  return output;

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
replace_all_refs(xmlNodePtr node, xmlDocPtr doc, article* art)
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
      else if (!xmlStrcmp(cur->name, (const xmlChar*) "ref"))
        {
          xmlChar* content = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
          section* sec = find_section(content, art);
          /* xmlFree(content); */
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
                  child_text = replace_all_refs(cur, doc, art);
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
      cur = cur->next;
    }

  return output;

}

char*
resolve_refs(xmlChar* html, article* art)
{
  char* document = malloc(sizeof(char)*(strlen("<html></html>")+2+xmlStrlen(html)));
  sprintf(document, "<html>%s</html>", (char*) html);

  xmlDocPtr doc = xmlReadDoc(document,
                             NULL,
                             NULL,
                             0);

  free(document);
  xmlNodePtr cur = xmlDocGetRootElement(doc);

  return (char*) replace_all_refs(cur, doc, art);
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
write_article(FILE* outf, article* art)
{
  fprintf(outf, "<html>");

  fprintf(outf, "<head><style>");
  fprintf(outf, "#main-content { width: 70%; margin-left:15%; } ");
  fprintf(outf, "#title { font-size: 3em; text-align: center} ");
  fprintf(outf, ".math-display { text-align: center; font-size: 1.2em }");
  fprintf(outf, ".math-inline { font-size: 1em }");
  
  if (art->style != NULL)
    {
      fprintf(outf, "%s", art->style);
    }
  
  fprintf(outf, "</style></head><body><div id=\"main-content\">");
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

  if (art->has_bib)
    {
      fprintf(outf, "<section class=\"bibliography\">");
      fprintf(outf, "<h3>References</h3>");
      xmlDocPtr bibdoc = xmlReadDoc(art->bib.format,
                                    NULL,
                                    NULL,
                                    0);

      xmlNodePtr cur = xmlDocGetRootElement(bibdoc)->xmlChildrenNode;
      for (uint64_t i=0; i < art->bib.n_entries; i++)
        {
          fprintf(outf, "<div id=\"");
          fprintf(outf, "%s", art->bib.entries[i].name);
          fprintf(outf, "\">");
          write_bibliography(outf, cur, bibdoc, &art->bib.entries[i]);
          fprintf(outf, "</div>");
        }
      fprintf(outf, "</section>");
    }
  fprintf(outf, "</div></body>");
  fprintf(outf, "</html>");
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
          output.abstract.html = parse_section_html(cur, doc, &output);
          
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

          sec.html = parse_section_html(cur, doc, &output);

          output.n_sections += 1;

          if (output.sections == NULL)
            output.sections = malloc(sizeof(section));
          else
            output.sections = realloc(output.sections, sizeof(section)*output.n_sections);

          output.sections[output.n_sections-1] = sec;
        }
      else if (!xmlStrcmp(cur->name, (const xmlChar*) "bibliography"))
        {
          output.bib = parse_bibliography(cur, doc);
          output.has_bib = true;
        }
      cur = cur->next;
    }

  for (uint64_t i=0; i < output.n_sections; i++)
    {
      output.sections[i].html = resolve_refs(output.sections[i].html, &output);
    }

  if (output.abstract.html != NULL)
    output.abstract.html = resolve_refs(output.abstract.html, &output);

  return output;

}

