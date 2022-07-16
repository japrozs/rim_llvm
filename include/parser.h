#ifndef PARSER_H
#define PARSER_H

#include "rim.h"

typedef struct
{
	token_t current;
	token_t previous;
	bool had_error;
	bool panic_mode;
} parser_t;

void parser_parse(LLVMModuleRef module, LLVMBuilderRef builder, LLVMContextRef context, const char *code);

#endif // PARSER_H
