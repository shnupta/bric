//Stack implementation for tag-based movement

#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include "modules/tag/tagstack.h"

#include <locale.h>
#include "gettext.h"
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

/* For testing, you may need the "exuberant-ctags" package installed */
/* if you enter `ctags --version` and the output is similar to this:
 *
 * ctags (GNU Emacs 23.1)
 * Copyright (C) 2009 Free Software Foundation, Inc.
 * This program is distributed under the terms in ETAGS.README
 *
 * that's the wrong "ctags" program. The output should be similar to this:
 *
 * Exuberant Ctags 5.9~svn20110310, Copyright (C) 1996-2009 Darren Hiebert
 * Addresses: <dhiebert@users.sourceforge.net>, http://ctags.sourceforge.net
 * Optional compiled features: +wildcards, +regex
 *
 * then use `ctags -R .` in the top level source directory to generate
 * a tags file.
 */

int
isempty (tagstack * s)
{
  return s->p == NULL;
}

void
init (tagstack * s)
{
  s->p = NULL;
}

void
push (tagstack * s, tagdata element)
{
  tagstack *foo;
  foo = s;
  while (foo->p != NULL)
  {
    foo = foo->p;
  }
  foo->p = (tagstack *) malloc (sizeof (tagstack));
  strcpy (foo->p->data.tagname, element.tagname);
  foo->p->data.linenumber = element.linenumber;
  strcpy (foo->p->data.filename, element.filename);
  foo->p->p = NULL;
}

tagdata
pop (tagstack * s)
{
  /* This function is using a "Linked List" */
  tagstack *foo;
  tagstack *node;

  tagdata element;
  foo = s;
  node = foo;
  while (foo->p != NULL)
  {
    node = foo;
    foo = foo->p;
  }
  strcpy (element.tagname, foo->data.tagname);
  element.linenumber = foo->data.linenumber;
  strcpy (element.filename, foo->data.filename);

  node->p = NULL;
  return element;
}
