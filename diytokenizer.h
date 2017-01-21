#ifndef tokenizer_H_
#define tokenizer_H_
#include <stdio.h>

typedef struct loc loc;
struct loc
{
	int row, col;
	loc *next2;
};

typedef struct sym_table sym_table;
struct sym_table
{
	char token[101];
	int type;
	loc *list;
	sym_table *next1;
};

enum
{
	Match = 256,
	Split = 257
};

typedef struct State State;
struct State
{
	int c;
	State *out;
	State *out1;
	int lastlist;
};

typedef union Ptrlist Ptrlist;
typedef struct Frag Frag;
struct Frag
{
	State *start;
	Ptrlist *out;
};

union Ptrlist
{
	Ptrlist *next;
	State *s;
};

typedef struct List List;
struct List
{
	State **s;
	int n;
};
#endif
