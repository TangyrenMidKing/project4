/*
	Author: Shibo Ding
					Jiahao Zhu
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "compiler.h"

instruction *code;
int code_index = 0;
int jpc_index = 0;
int jmp_index = 0;
int sym_index = 0;
int level = 0;
lexeme *token;
symbol *table;

// prototypes
void printcode();
void program();
void block();
void const_decl();
void const_decl_prime();
int var_decl();
int var_decl_prime();
void proc_decl();
void statement();
void expression();
void expression_prime();
void condition();
void term();
void term_prime();
void factor();
void unmark();
void begin();
void next_token(int num);
void emit(int op, int l , int m);
int find(int type);

instruction *generate_code(lexeme *tokens, symbol *symbols)
{
	code = malloc(500 * sizeof(instruction));
	token = tokens;
	table = symbols;
	program();
	printcode();
	return code;
}

void program()
{
	table[0].mark = 0;
	emit(7, 0, 0);
	block();
	emit(9, 0, 3);
	code[0].m = 3;
}

void block()
{
	level++;
	int proc_index = sym_index;
	const_decl();
	int num_var = var_decl();
	proc_decl();
	table[proc_index].level = code_index;
	emit(6, 0, num_var + 3);
	statement();
	for (int i = 0; i <= sym_index; i++)
		table[i].mark = 1;
	level--;
}

void const_decl()
{
	if (token->type == constsym)
	{
		next_token(4);
		unmark();
		const_decl_prime();
		next_token(1);
	}
}

void const_decl_prime()
{

	if (token->type == commasym)
	{
		next_token(4);
		unmark();
		const_decl_prime();
		next_token(1);
	}
}

int var_decl()
{

	int num_var = 0;
	if (token->type == varsym)
	{
		next_token(2);
		unmark();
		num_var = 1 + var_decl_prime();
		next_token(1);
	}
	return num_var;
}

int var_decl_prime()
{
	if (token->type == commasym)
	{
		next_token(2);
		unmark();
		return 1 + var_decl_prime();
	}
	return 0;
}

void proc_decl()
{
	if (token->type == procsym)
	{
		next_token(3);
		unmark();
		block();
		next_token(1);
		emit(2, 0, 0);
		proc_decl();
	}
}

void statement()
{

	/* const => 1 | var => 2 | proc => 3 */
	if (token->type == identsym)
	{
		int index = find(2);
		next_token(2);
		expression();
		emit(4, level - table[index].level, table[index].addr);
	}
	else if (token->type == callsym)
	{
		next_token(1);
		int index = find(3);
		next_token(1);
		emit(5, level - table[index].level, table[index].level);
	}
	else if (token->type == writesym)
	{
		next_token(1);
		expression();
		emit(9,0,1);
	}
	else if (token->type == readsym)
	{
		next_token(1);
		int index = find(2);
		next_token(1);
		emit(9,0,2);
		emit(4,level - table[index].level, table[index].addr);
	}
	else if (token->type == beginsym)
	{
		next_token(1);
		statement();
		begin();
		next_token(1);
	}
	else if (token->type == ifsym)
	{
		next_token(1);
		condition();
		jpc_index = code_index;
		emit(8,0,0);
		next_token(1);
		statement();
		if (token->type == elsesym)
		{
			next_token(1);
			jmp_index = code_index;
			emit(7, 0, 0);
			code[jpc_index].m = code_index * 3;
			statement();
			code[jmp_index].m = code_index * 3;
		}
		else
			code[jpc_index].m = code_index * 3;
	}
	else if (token->type == whilesym)
	{
		next_token(1);
		jmp_index = code_index;
		condition();
		next_token(1);
		jpc_index = code_index;
		emit(8, 0, 0);
		statement();
		emit(7, 0, jmp_index * 3);
		code[jpc_index].m = code_index;
	}
}

void begin()
{

	if (token->type == semicolonsym)
	{
		next_token(1);
		statement();
		begin();
	}
}

void condition()
{

	if (token->type == oddsym)
	{
		next_token(1);
		expression();
		emit(2,0,6);
	}
	else
	{
		expression();
		next_token(1);
		expression();
		switch(token->type)
		{
			 case eqlsym: emit(2,0,8); break;
			 case neqsym: emit(2,0,9); break;
			 case lessym: emit(2,0,10); break;
			 case leqsym: emit(2,0,11); break;
			 case gtrsym: emit(2,0,12); break;
			 case geqsym: emit(2,0,13); break;
		}
	}
}

void 	expression()
{

	if (token->type == plussym)
	{
		next_token(1);
		term();
		expression_prime();
	}
	else if (token->type == minussym)
	{
		next_token(1);
		term();
		emit(2, 0, 1);
		expression_prime();
	}
	else
	{
		term();
		expression_prime();
	}
}

void expression_prime()
{

	if (token->type == plussym)
	{
		next_token(1);
		term();
		emit(2, 0, 2);
		expression_prime();
	}
	else if (token->type == minussym)
	{
		next_token(1);
		term();
		emit(2, 0, 3);
		expression_prime();
	}
}

void term()
{
	factor();
	term_prime();
}

void term_prime()
{
	if (token->type == multsym)
	{
		next_token(1);
		factor();
		emit(2, 0, 4);
		term_prime();
	}
	else if (token->type == slashsym)
	{
		next_token(1);
		factor();
		emit(2, 0, 5);
		term_prime();
	}
	else if (token->type == modsym)
	{
		next_token(1);
		factor();
		emit(2, 0, 7);
		term_prime();
	}
}

void factor()
{

	if(token->type == identsym)
	{
		int index = find(1);
		if (table[index].kind == 2)
			emit(3, level - table[index].level, table[index].addr);
		if (table[index].kind == 1)
			emit(1, 0, table[index].val);
		next_token(1);
	}
	else if(token->type == numbersym)
	{
		emit(1, 0, token->value);
		next_token(1);
	}
	else
	{
		next_token(1);
		expression();
		next_token(1);
	}
}

void emit(int opcode, int l, int m)
{
    code[code_index].opcode = opcode;
    code[code_index].l = l;
    code[code_index].m = m;
    code_index++;
}

/**
const => 1
var => 2
porc => 3
*/
int find(int type)
{
	int min = 100, index = 0;
	for (int i = sym_index; i >= 0; i--)
	{
		if (strcmp(table[i].name,token->name) == 0)
		{
			index = i;
		}
	}
	return index;
}

void next_token(int num)
{
	token += num;
}

void unmark()
{
	sym_index++;
}

void printcode()
{
	int i;
	printf("Line\tOP Code\tOP Name\tL\tM\n");
	for (i = 0; i < code_index; i++)
	{
		printf("%d\t", i);
		printf("%d\t", code[i].opcode);
		switch (code[i].opcode)
		{
			case 1:
				printf("LIT\t");
				break;
			case 2:
				switch (code[i].m)
				{
					case 0:
						printf("RTN\t");
						break;
					case 1:
						printf("NEG\t");
						break;
					case 2:
						printf("ADD\t");
						break;
					case 3:
						printf("SUB\t");
						break;
					case 4:
						printf("MUL\t");
						break;
					case 5:
						printf("DIV\t");
						break;
					case 6:
						printf("ODD\t");
						break;
					case 7:
						printf("MOD\t");
						break;
					case 8:
						printf("EQL\t");
						break;
					case 9:
						printf("NEQ\t");
						break;
					case 10:
						printf("LSS\t");
						break;
					case 11:
						printf("LEQ\t");
						break;
					case 12:
						printf("GTR\t");
						break;
					case 13:
						printf("GEQ\t");
						break;
					default:
						printf("err\t");
						break;
				}
				break;
			case 3:
				printf("LOD\t");
				break;
			case 4:
				printf("STO\t");
				break;
			case 5:
				printf("CAL\t");
				break;
			case 6:
				printf("INC\t");
				break;
			case 7:
				printf("JMP\t");
				break;
			case 8:
				printf("JPC\t");
				break;
			case 9:
				switch (code[i].m)
				{
					case 1:
						printf("WRT\t");
						break;
					case 2:
						printf("RED\t");
						break;
					case 3:
						printf("HAL\t");
						break;
					default:
						printf("err\t");
						break;
				}
				break;
			default:
				printf("err\t");
				break;
		}
		printf("%d\t%d\n", code[i].l, code[i].m);
	}
}
