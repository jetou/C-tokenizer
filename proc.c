#pragma warning(disable:4996)
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define push(s) *stackp++ = s
#define pop() *--stackp

int tot_size = 0, cnt, key_cnt = 0, par_cnt = 0, del_cnt = 0, relop_cnt = 0, punc_cnt = 0, op_cnt = 0, id_cnt = 0, l_cnt = 0, d_cnt = 0;
char keywords[20][20], par[20], delimiter[20], relop[3][20], punctuation[20], operators[20], identifiers[100], numbers[100], config[1000], program[10000];
// �ؼ��� Բ���� �ָ��� ��ϵ����� ��� ������ ��ʶ�� ���� ���� ����
char L[100], D[100];
//�洢ÿ��tokenλ�õ�����
typedef struct loc loc;
struct loc
{
	int row, col;
	loc *next2;
};

//��Ϊ������洢�ķ��ű�Ľṹ
typedef struct sym_tab sym_tab;
struct sym_tab
{
	char token[100];
	int type;//1�ؼ��ʣ�2���ţ�3��ϵ�������4�������5��ʶ����6���֣�7 ����
	loc *list;
	sym_tab *next1;
};

sym_tab *table;

//��׺���׺ ����
void infix_postfix(char *inf, char *final)
{
	int length = strlen(inf);
	int i, j = 0, k = 0, cnt = 0, cur = 0;

	char st_sym[100];

	for (i = 0; i < length; i++)
	{
		if (inf[i] == '(')
		{
			cnt++;
		}
		else if (inf[i] == ')')
		{
			final[cur] = st_sym[k - 1];
			k--;
			cur++;
		}
		else if (inf[i] == '.' || inf[i] == '*' || inf[i] == '?' || inf[i] == '|' || inf[i] == '+')
		{
			st_sym[k] = inf[i];
			k++;
		}
		else
		{
			final[cur] = inf[i];
			cur++;
		}
	}
	final[cur] = '\0';
}



enum
{
	Match = 256,
	Split = 257
};

//״̬�Ľṹ��
typedef struct State State;
struct State
{
	int c;
	State *out;
	State *out1;
	int lastlist;
};

//����״̬
State matchstate = { Match };

int nstate;

//initial state
State*
state(int c, State *out, State *out1)
{
	State *s;

	nstate++;   //����״̬���ܼ���
	s = malloc(sizeof *s);
	s->lastlist = 0;
	s->c = c;
	s->out = out;
	s->out1 = out1;
	return s;
}

//NFA
typedef struct Frag Frag;
typedef union Ptrlist Ptrlist;
struct Frag
{
	State *start; //��ʼ״̬
	Ptrlist *out;
};

//initialize the frag structure
Frag
frag(State *start, Ptrlist *out)
{
	Frag n = { start, out };
	return n;
}

//storing the lists
union Ptrlist
{
	Ptrlist *next;
	State *s;
};

//create a single list
Ptrlist*
list1(State **outp)
{
	Ptrlist		*l;

	l = (Ptrlist*)outp;
	l->next = NULL;
	return l;
}

//��state ���� ptrlist ��
void
patch(Ptrlist *l, State *s)
{
	Ptrlist *next;

	for (; l; l = next){
		next = l->next;
		l->s = s;
	}
}

// ������������
Ptrlist*
append(Ptrlist *l1, Ptrlist *l2)
{
	Ptrlist *oldl1;

	oldl1 = l1;
	while (l1->next)
		l1 = l1->next;
	l1->next = l2;
	return oldl1;
}

//��׺תnfa
State*
post2nfa(char *postfix)
{
	char *p;
	Frag stack[1000], *stackp, e1, e2, e;
	State *s;

	if (postfix == NULL)
		return NULL;

	stackp = stack;
	for (p = postfix; *p; p++){
		switch (*p){
		default:
			s = state(*p, NULL, NULL);
			push(frag(s, list1(&s->out)));
			break;
		case '.':
			e2 = pop();
			e1 = pop();
			patch(e1.out, e2.start);
			push(frag(e1.start, e2.out));
			break;
		case '|':
			e2 = pop();
			e1 = pop();
			s = state(Split, e1.start, e2.start);
			push(frag(s, append(e1.out, e2.out)));
			break;
		case '?':
			e = pop();
			s = state(Split, e.start, NULL);
			push(frag(s, append(e.out, list1(&s->out1))));
			break;
		case '*':
			e = pop();
			s = state(Split, e.start, NULL);
			patch(e.out, s);
			push(frag(s, list1(&s->out1)));
			break;
		case '+':
			e = pop();
			s = state(Split, e.start, NULL);
			patch(e.out, s);
			push(frag(e.start, list1(&s->out1)));
			break;
		}
	}

	e = pop();
	if (stackp != stack)
		return NULL;

	patch(e.out, &matchstate);
	return e.start;
}


typedef struct List List;
struct List
{
	State **s;
	int n;
};
List l1, l2, l3, l4;
static int listid;

void addstate(List*, State*);
void step(List*, int, List*);


List*
startlist(State *start, List *l)
{
	l->n = 0;
	listid++;
	addstate(l, start);
	return l;
}


int
ismatch(List *l)
{
	int i;

	for (i = 0; i < l->n; i++)
		if (l->s[i] == &matchstate)
			return 1;
	return 0;
}


void
addstate(List *l, State *s)
{
	if (s == NULL || s->lastlist == listid)
		return;
	s->lastlist = listid;
	if (s->c == Split){

		addstate(l, s->out);
		addstate(l, s->out1);
		return;
	}
	l->s[l->n++] = s;
}


void
step(List *clist, int c, List *nlist)
{
	int i;
	State *s;

	listid++;
	nlist->n = 0;
	for (i = 0; i < clist->n; i++){
		s = clist->s[i];
		if (s->c == c)
			addstate(nlist, s->out);
	}
}


int
match(State *start, char *s, int op)
{
	int i, c;
	List *clist, *nlist, *t;

	if (op == 1)
	{
		clist = startlist(start, &l1);
		nlist = &l2;
	}
	else
	{
		clist = startlist(start, &l3);
		nlist = &l4;
	}

	for (; *s; s++){
		c = *s & 0xFF; //256
		step(clist, c, nlist);
		t = clist; clist = nlist; nlist = t;
	}
	return ismatch(clist);
}
///////////////////////////////////////////
//���ָ��������ļ�
int check_delimiter(char t)
{
	int flag = -1, i;

	for (i = 0; i < del_cnt; i++)
	{
		if (t == delimiter[i])
			flag = i;
	}
	return flag;
}

//��������ļ�������
int check_parenthesis(char t)
{
	int flag = -1, i;

	for (i = 0; i < par_cnt; i++)
	{
		if (t == par[i])
			flag = i;
	}
	return flag;
}


int check_quotes(char t)
{
	int flag = -1, i;

	if (t == '\'' || t == '"')
		flag = 1;

	return flag;
}


int keyword_search(char *token)
{
	int i, j, flag = -1, mat = 0, len = strlen(token);

	for (i = 0; i < key_cnt; i++)
	{
		mat = 0;
		for (j = 0; j < strlen(keywords[i]); j++)
		{
			if (j <= len && keywords[i][j] == token[j])
				mat++;//�����һ���ַ���ͬ��++
		}
		if (mat == strlen(keywords[i]))
		{
			flag = i;
			i = key_cnt;
		}
	}
	return flag;
}

//��ϵ�����
int relop_search(char *token)
{
	int i, j, flag = -1, mat = 0, len = strlen(token);

	for (i = 0; i < relop_cnt; i++)
	{
		mat = 0;
		for (j = 0; j < strlen(relop[i]); j++)
		{
			if (j < len && relop[i][j] == token[j])
				mat++;
		}
		if (mat == strlen(relop[i]))
		{
			flag = i;
			i = key_cnt;
		}
	}
	return flag;
}


int op_search(char *token)
{
	int i, j, flag = -1, mat = 0, len = strlen(token);

	for (i = 0; i < op_cnt; i++)
	{
		mat = 0;
		if (j <= len && operators[i] == token[0])
			mat++;
		if (strlen(token) == 1 && mat == 1)
		{
			flag = i;
			i = key_cnt;
		}
	}
	return flag;
}



void save_list(char *temp, int type, int row, int col)
{
	if (table == NULL)
	{
		table = (sym_tab *)malloc(sizeof(sym_tab));
		table->next1 = NULL;
		strcpy(table->token, temp);
		table->type = type;

		loc *t12;
		t12 = (loc *)malloc(sizeof(loc));
		t12->row = row;
		t12->col = col;
		t12->next2 = NULL;

		table->list = t12;
	}
	else
	{
		sym_tab *t123, *last;
		t123 = table;
		int fl = 0;
		while (t123 != NULL)
		{
			if (t123->type == type)
			{
				if (strcmp(t123->token, temp) == 0)
					break;
			}
			last = t123;
			t123 = t123->next1;
		}
		if (t123 != NULL)//����ҵ�һ�����ַ���ֱ�Ӽ�¼���к�
		{
			loc *t12;
			t12 = (loc *)malloc(sizeof(loc));
			t12->row = row;
			t12->col = col;
			t12->next2 = NULL;

			loc *l123;
			l123 = t123->list;

			while (l123->next2 != NULL)
			{
				l123 = l123->next2;
			}
			l123->next2 = t12;
		}
		else
		{
			sym_tab *table1;
			table1 = (sym_tab *)malloc(sizeof(sym_tab));
			table1->next1 = NULL;
			strcpy(table1->token, temp);
			table1->type = type;

			loc *t12;
			t12 = (loc *)malloc(sizeof(loc));
			t12->row = row;
			t12->col = col;
			t12->next2 = NULL;

			table1->list = t12;

			last->next1 = table1;
		}
	}
}

int main()
{
	char regex[100], final[100], input[10][10], post_num[100], post_id[100];
	int i, flag_rd = 0, flag_cur = 0, flag_box = 0;
	State *start_id, *start_num;
	table = NULL;

	char temp[20], c;
	char token[1000];

	int j, k, l, m, n;

	FILE *fp, *pro, *output, *error_file;


	fp = fopen("config.txt", "r");

	c = getc(fp);
	while (c != EOF)
	{
		config[tot_size] = c;
		tot_size++;
		c = getc(fp);
	}
	fclose(fp);

	i = 0;


	while (!(config[i] == '-' && config[i + 1] == '>'))
	{
		i++;
	}

	//�ַ�a-z����L
	while (!(config[i] == 'L' && config[i + 1] == ':'))
		i++;

	i = i + 3;
	cnt = 0;
	while (config[i] != '\n')
	{
		if (config[i] == ',')
		{
			temp[cnt] = '\0';
			L[l_cnt] = temp[0];
			l_cnt++;
			cnt = 0;
		}
		else
		{
			temp[cnt] = config[i];
			cnt++;
		}
		i++;
	}

	//ARRAY 'D'number
	while (!(config[i] == 'D' && config[i + 1] == ':'))
		i++;

	i = i + 3;
	cnt = 0;
	while (config[i] != '\n')
	{
		if (config[i] == ',')
		{
			temp[cnt] = '\0';
			D[d_cnt] = temp[0];
			d_cnt++;
			cnt = 0;
		}
		else
		{
			temp[cnt] = config[i];
			cnt++;
		}
		i++;
	}

	//key words
	while (!(config[i] == 'K' && config[i + 1] == 'e' && config[i + 2] == 'y'))
	{
		i++;
	}

	i += 10;

	cnt = 0;
	while (config[i] != '\n')
	{
		if (config[i] == ',')
		{
			temp[cnt] = '\0';
			strcpy(keywords[key_cnt], temp);
			key_cnt++;
			cnt = 0;
		}
		else
		{
			temp[cnt] = config[i];
			cnt++;
		}
		i++;
	}

	//����
	while (!(config[i] == 'p'&&config[i + 1] == 'a'&&config[i + 2] == 'r'))
		i++;

	i = i + 13;
	cnt = 0;
	while (config[i] != '\n')
	{
		if (config[i] == ',')
		{
			temp[cnt] = '\0';
			par[par_cnt] = temp[0];
			par_cnt++;
			cnt = 0;
		}
		else
		{
			temp[cnt] = config[i];
			cnt++;
		}
		i++;
	}

	//�ָ���
	while (!(config[i] == 'd' && config[i + 1] == 'e' && config[i + 2] == 'l'))
		i++;

	i = i + 11;
	cnt = 0;
	while (config[i] != '\n')
	{
		if (config[i] == ',')
		{
			temp[cnt] = '\0';
			delimiter[del_cnt] = temp[0];
			del_cnt++;
			cnt = 0;
		}
		else
		{
			temp[cnt] = config[i];
			cnt++;
		}
		i++;
	}

	delimiter[del_cnt] = '\n';
	delimiter[del_cnt + 1] = ',';
	del_cnt += 2;

	//�߼������
	while (!(config[i] == 'r' && config[i + 1] == 'e' && config[i + 2] == 'l'))
		i++;

	i = i + 7;
	cnt = 0;
	while (config[i] != '\n')
	{
		if (config[i] == ',')
		{
			temp[cnt] = '\0';
			strcpy(relop[relop_cnt], temp);
			relop_cnt++;
			cnt = 0;
		}
		else
		{
			temp[cnt] = config[i];
			cnt++;
		}
		i++;
	}

	//��ϵ�����
	while (!(config[i] == 'o' && config[i+1] == 'p' && config[i+2] == 'e'))
		i++;

	i = i + 10;
	cnt = 0;
	while (config[i] != '\n')
	{
		if (config[i] == ',')
		{
			temp[cnt] = '\0';
			operators[op_cnt] = temp[0];
			op_cnt++;
			cnt = 0;
		}
		else
		{
			temp[cnt] = config[i];
			cnt++;
		}
		i++;
	}

	//��ʶ��
	while (!(config[i] == 'i' && config[i + 1] == 'd' && config[i + 2] == 'e'))
		i++;



	i = i + 13;
	cnt = 0;
	while (config[i] != '\n')
	{
		temp[cnt] = config[i];
		cnt++;
		i++;
	}
	temp[cnt] = '\0';
	strcpy(identifiers, temp);

	//����
	while (!(config[i] == 'n' && config[i + 1] == 'u' && config[i + 2] == 'm'))
	{
		i++;
	}

	i = i + 9;
	cnt = 0;
	while (config[i] != '\n')
	{
		temp[cnt] = config[i];
		cnt++;
		i++;
	}
	temp[cnt] = '\0';
	strcpy(numbers, temp);
	/////////////////////////start
	infix_postfix(identifiers, post_id);
	start_id = post2nfa(post_id);

	infix_postfix(numbers, post_num);
	start_num = post2nfa(post_num);

	l1.s = malloc(nstate*sizeof l1.s[0]);
	l2.s = malloc(nstate*sizeof l2.s[0]);

	l3.s = malloc(nstate*sizeof l3.s[0]);
	l4.s = malloc(nstate*sizeof l4.s[0]);


	pro = fopen("sample.txt", "r");

	output = fopen("symbol_table.txt", "w");
	error_file = fopen("error_file.txt", "w");

	c = getc(pro);
	tot_size = 0;
	while (c != EOF)
	{
		program[tot_size] = c;
		tot_size++;
		c = getc(pro);
	}
	fclose(pro);

	fprintf(output, "%20s %20s %30s\n", "TOKEN", "TYPE", "LINE NUMBER (row,col)");

	int row = 1, col = 0;
	cnt = 0;
	//
	for (i = 0; i < tot_size; i++)
	{
		//����ע��
		if (i != tot_size - 1)
		{
			if (program[i] == '/' && program[i + 1] == '/')
			{
				while ((program[i] != '\n') && i < tot_size)
					i++;
				row++;
			}
		}

		//( { [
		if (flag_rd < 0)
			fprintf(error_file, "\nError on line %d column %d.\n \t Extra parenthesis ')'.", row, col);

		if (flag_cur < 0)
			fprintf(error_file, "\nError on line %d column %d.\n \t Extra parenthesis '}'.", row, col);

		if (flag_box < 0)
			fprintf(error_file, "\nError on line %d column %d.\n \t Extra parenthesis ']'.", row, col);

		//�����������ַ�
		if (check_quotes(program[i]) != -1)
		{
			char abc = program[i];
			j = 1;
			temp[0] = program[i];
			int prev_row = row;
			int prev_col = col;
			while (((i + j) < tot_size) && (program[i + j] != abc))
			{
				temp[j] = program[i + j];
				j++;
				if (program[i + j] == '\n')
				{
					row++;
					col = 0;
				}
				else
					col++;
			}

			if (i + j == tot_size)
			{
				fprintf(error_file, "\nError on line %d colum %d.\n \t Quotes does not have matching pair.", prev_row, prev_col);
				break;
			}

			temp[j] = abc;
			temp[j + 1] = '\0';



			save_list(temp, 7, row, col);

			i = i + j;
		}

		//�ָ���
		else if (check_delimiter(program[i]) != -1)
		{
			token[cnt] = '\0';

			if (cnt != 0)
			{
				j = 0;
				while (j < strlen(token))
				{
					int my_fl = 0; //l_cnt=26
					for (m = 0; m < l_cnt; m++)
					{
						if (L[m] == token[j])
						{
							temp[j] = 'L';
							my_fl = 1;
						}
					}
					for (m = 0; m < d_cnt; m++)
					{
						if (D[m] == token[j])
						{
							temp[j] = 'D';
							my_fl = 1;
						}
					}
					if (token[j] == '.')
						temp[j] = 'W';

					else if (my_fl == 0)
						temp[j] = token[j];

					j++;
				}
				temp[j] = '\0';

				//����ǹؼ���
				if (keyword_search(token) != -1)
				{


					save_list(token, 1, row, (col - strlen(token) - 1));
				}

				//������߼������
				else if (relop_search(token) != -1)
				{


					save_list(token, 3, row, (col - strlen(token)));
				}

				//����Ƿ��������
				else if (op_search(token) != -1)
				{


					save_list(token, 4, row, (col - 1));
				}

				//����Ƿ��Ǳ�ʶ��
				else if (match(start_id, temp, 1))
				{


					save_list(token, 5, row, (col - strlen(token)));
				}

				//����Ƿ��Ǹ�����
				else if (match(start_num, temp, 2))
				{


					save_list(token, 6, row, (col - strlen(token)));
				}

				//����Ķ�δƥ�����Ǹ�����
				else
				{
					fprintf(error_file, "\nError on line %d coulum %d.\n \t %s does not match any known type of token.", row, (col - strlen(token)), token);
				}
			}

			//������п�ʼ�Ƿָ����ͼ���Ƿ�������
			if (check_parenthesis(program[i]) != -1)
			{


				if (program[i] == '(')
					flag_rd++;
				else if (program[i] == ')')
					flag_rd--;

				if (program[i] == '{')
					flag_cur++;
				else if (program[i] == '}')
					flag_cur--;

				if (program[i] == '[')
					flag_box++;
				else if (program[i] == ']')
					flag_box--;

				char temp12[1000];
				strcpy(temp12, token);
				token[0] = program[i];
				token[1] = '\0';

				save_list(token, 2, row, (col - 1));
			}
			if (program[i] == '\n')
			{
				row++;
				col = 0;
			}

			cnt = 0;
		}
		else
		{
			token[cnt] = program[i];
			cnt++;
		}
		col++;
	}

	sym_tab *temp12;
	temp12 = table;

	//������ļ���
	while (temp12 != NULL)
	{
		if (temp12->type == 1)
			fprintf(output, "%20s %20s %5s", temp12->token, "keyword", "");
		else if (temp12->type == 2)
			fprintf(output, "%20s %20s %5s", temp12->token, "parenthesis", "");
		else if (temp12->type == 3)
			fprintf(output, "%20s %20s %5s", temp12->token, "rel. operator", "");
		else if (temp12->type == 4)
			fprintf(output, "%20s %20s %5s", temp12->token, "operator", "");
		else if (temp12->type == 5)
			fprintf(output, "%20s %20s %5s", temp12->token, "identifier", "");
		else if (temp12->type == 6)
			fprintf(output, "%20s %20s %5s", temp12->token, "number", "");
		else if (temp12->type == 7)
			fprintf(output, "%20s %20s %5s", temp12->token, "literal", "");

		loc *ter12 = temp12->list;
		while (ter12 != NULL)
		{
			fprintf(output, "(%d,%d),", ter12->row, ter12->col);
			ter12 = ter12->next2;
		}
		temp12 = temp12->next1;
		fprintf(output, "\n");


	}
	fclose(output);

	//���������޹ر�
	if (flag_rd != 0)
		fprintf(error_file, "\nError on line %d column %d.\n \t '(' not closed.", row, col);

	if (flag_cur != 0)
		fprintf(error_file, "\nError on line %d column %d. \n \t '{' not closed.", row, col);

	if (flag_box != 0)
		fprintf(error_file, "\nError on line %d column %d.\n \t '[' not closed.", row, col);

	printf("\n");

	return 0;
}
