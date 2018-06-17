// Implementaca em pilha para movimento baseado em tag

#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include "modules/tag/tagstack.h"
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
	tagstack *foo, *curr;
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

