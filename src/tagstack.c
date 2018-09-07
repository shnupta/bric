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

int isempty(tagstack *s) {
	return s->p == NULL;
}
void init(tagstack *s) {
	s->p = NULL;
}
void push(tagstack *s, tagdata element) {
	tagstack *foo;
	foo = s;
	while(foo->p != NULL) {
		foo = foo->p;
	}
	foo->p = (tagstack*)malloc(sizeof(tagstack));
	strcpy(foo->p->data.tagname, element.tagname);
	foo->p->data.linenumber = element.linenumber;
	strcpy(foo->p->data.filename, element.filename);
	foo->p->p = NULL;
}
tagdata pop(tagstack *s) {
	tagstack *foo = (tagstack *)calloc (sizeof (s), sizeof (char));
	if (foo == NULL)
	{
		perror(_("Error allocating memory."));
		exit (1);
	}

	tagstack *curr = (tagstack *)calloc (sizeof (s), sizeof (char));
	if (curr == NULL)
	{
		perror(_("Error allocating memory."));
		exit (1);
	}

	tagdata element;
	foo = s;
	while(foo->p != NULL) {
		curr = foo;
		foo = foo->p;
	}
	strcpy(element.tagname, foo->data.tagname);
	element.linenumber = foo->data.linenumber;
	strcpy(element.filename, foo->data.filename);
	free(curr->p);
	curr->p = NULL;
	return element;
}

