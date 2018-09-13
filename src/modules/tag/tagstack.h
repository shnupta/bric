//Header file for tag stack

typedef struct tagdata
{
  char tagname[64];
  int linenumber;
  char filename[64];
} tagdata;

typedef struct tagstack
{
  tagdata data;
  struct tagstack *p;
} tagstack;

int isempty (tagstack * s);
void init (tagstack * s);
void push (tagstack * s, tagdata element);
tagdata pop (tagstack * s);
