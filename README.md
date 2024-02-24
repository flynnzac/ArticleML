# ArticleML

ArticleML is a human-writeable XML document format for writing articles on the internet, especially if they include math.  It currently includes a citation manager ala BibTex, inline citations, references between different sections, formatting math in both display and inline format, and the ability to write, reference, and automatically number theorems, assumptions, and definitions.  The program converts ArticleML to HTML for publishing to the web.

**ArticleML's goal is to make academic articles easier to index, snip, clip, copy, and search on the Internet by encouraging posting papers as HTML instead of PDF.**

# Installation

Type `make && sudo make install` to install.

Dependencies:

- glibc
- libxml2
- leg/peg parser generator

If you find issues installing, let me know and I'll fix.  It will likely work on Windows with MSYS and the like because it only depends on libxml2.

The plan is to make some pre-built binaries for popular systems to make this easy.

# Usage

The `articleML` programs takes the following arguments:

- `-n`: output "headless" HTML, i.e. without `<html>, <body>`, etc, tags.  Useful for embedding output in another page.
- `-s`: suppress default styles.

It reads from standard input and writes to standard output (i.e. it is a good Unix program).  Like so:

```
articleml < input.xml > output.html
```


# Syntax

ArticleML documents are valid XML.  There are some ArticleML-specific tags.  Everything else is passed through as is so that you can include (X)HTML tags. If you are familiar with HTML, the syntax will take no time at all to pick up.  See the documentation in docs/documentation.html for full details or at: https://zflynn.com/ArticleML/documentation.html.  The documentation itself is written in ArticleML so docs/documentation.xml is a good place to find examples.  There are also other examples in the `example/` folder.

Here is a brief example of the ArticleML syntax:

```
<article>
  <meta>
    <title>Hello World</title>
    <author>Zach Flynn</author>
    <date>2024-02-11</date>
  </meta>

  <section name="Introduction">
    <p>This is a simple document.</p>
  </section>

  <section name="Details">
    <p>As mentioned in the <ref>Introduction</ref>, this is a simple document.</p>

    <p>It has some math though.</p>

    <m style="display">a^[2] + b^[2] = c^[2]</m>
    <m style="display">a x^[2] + b x + c = 0</m>
    <m style="display">@sum[i=1;n] i = @frac[n(n+1);2]</m>

    <p>The End <m>qed</m></p>

  </section>
</article>
```

# How to Write Math

Math in ArticleML is written in the `<m>` sublanguage which is essentially a structured version of the syntax many math-ish people already use when they write e-mails.  It was influenced and inspired by https://asciimath.org but it makes some changes to make it easier to use in HTML/XML documents and generally has a slightly more function-like syntax.  The documentation includes the full syntax.  It's easy to use and the markup looks a lot like how you would usually type math anyway.

If you put math between `<m>` and `</m>`, then the math will be inserted inline.  If you put math between `<m style="display">` and `</m>`, then the math will follow a line-break and be centered on the page (think the `align` environment in LaTex).

# How to Write Bibliography Files

Here is an example bibliography file:

```
<bibliography>
  <entry name="Flynn2024">
    <author>Zach Flynn</author>
    <title>My Paper</title>
    <publication>A Very Famous Journal</publication>
    <volume>38</volume>
    <pages>1-23</pages>
    <year>2024</year>
  </entry>
</bibliography>
```

An `<entry>` starts a new entry in the bibliography.  The `name` attribute gives the name you use to reference the entry in the main text.  There is no fixed format to what has to be included in the bibliography.  Any tag will be accepted and treated as an attribute that can be referenced in the bibilography formatting section (see documentation for full details) as `<source-attribute/>`.  However, for inline citations to work well, you should specify at least the `<author>` entry and the `<year>` entry.

The advantage of this approach is that you can build up one large bibliography file over time and just reference what you need in each paper.  The design of the bibliography is very flexible and lets you set the format however you'd like and specify any attribute that makes sense for your sources.

# Roadmap

A list of things that are coming soon...

1. Customize in-line citations.

And more...

# Acknowledgements

I referenced https://github.com/camoy/amath/ frequently to make sure I wasn't missing any common math symbols.  Thanks!

