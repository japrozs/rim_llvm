#include "rim.h"

parser_t parser;

void declaration(LLVMModuleRef module, LLVMBuilderRef builder, LLVMContextRef context);

void error_at_current(token_t *token, const char *msg)
{
	if (parser.panic_mode)
		return;
	fprintf(stderr, "[line %d] Error", token->line);

	if (token->type == TOKEN_EOF)
	{
		fprintf(stderr, " at end");
	}
	else
	{
		fprintf(stderr, " at '%.*s'", token->length, token->start);
	}

	fprintf(stderr, ": %s\n", msg);
	parser.had_error = true;
	exit(EXIT_SUCCESS);
}

void parser_advance()
{
	parser.previous = parser.current;

	for (;;)
	{
		parser.current = scan_token();
		if (parser.current.type != TOKEN_ERROR)
			break;

		error_at_current(&parser.current, parser.current.start);
	}
}

bool check(token_type type)
{
	return parser.current.type == type;
}

bool match(token_type type)
{
	if (!check(type))
		return false;
	parser_advance();
	return true;
}

void consume(token_type type, const char *message)
{
	if (parser.current.type == type)
	{
		parser_advance();
		return;
	}

	error_at_current(&parser.current, message);
}

// BUILDERS

void build_var(LLVMModuleRef module, LLVMBuilderRef builder, LLVMContextRef context)
{
	consume(TOKEN_IDENTIFIER, "expected variable name");

	// get name
	char *name = malloc(parser.previous.length * sizeof(char));
	sprintf(name, "%.*s", parser.previous.length, parser.previous.start);

	consume(TOKEN_COLON, "expected a colon");
	consume(TOKEN_IDENTIFIER, "expected type name");

	// get type
	char *type = malloc(parser.previous.length * sizeof(char));
	sprintf(type, "%.*s", parser.previous.length, parser.previous.start);
	if (match(TOKEN_EQUAL))
	{
		if (strcmp(type, "int") == 0)
		{
			// get value
			consume(TOKEN_NUMBER, "expected value for variable with type int");
			char *value = malloc(parser.previous.length * sizeof(char));
			sprintf(value, "%.*s", parser.previous.length, parser.previous.start);

			// (builder, LLVMDoubleType(), LLVMConstReal(LLVMDoubleType(), atoi(value)), name);
			LLVMValueRef var = LLVMBuildAlloca(builder, LLVMInt32Type(), name);
			LLVMBuildStore(builder, LLVMConstInt(LLVMInt32Type(), atoi(value), false), var);

			// LLVMValueRef arr_elements[] = {
			// 	LLVMConstInt(LLVMInt8Type(), 0, false),
			// 	LLVMConstInt(LLVMInt8Type(), 1, false),
			// };

			// LLVMBuildArrayAlloca(builder, LLVMArrayType(LLVMInt8Type(), 2), arr, "arr");
			// LLVMValueRef arr = LLVMConstArray(LLVMInt8Type(), arr_elements, 2);
			//----
			// LLVMValueRef arr_ref = LLVMBuildAlloca(builder, LLVMArrayType(LLVMInt8Type(), 2), "arr");
			// LLVMBuildStore(builder, LLVMConstArray(LLVMInt8Type(), arr_elements, 2), arr_ref);
		}
		else if (strcmp(type, "string") == 0)
		{
			// get value
			consume(TOKEN_STRING, "expected value for variable");
			char *value = malloc(parser.previous.length * sizeof(char));
			sprintf(value, "%.*s", parser.previous.length, parser.previous.start);

			// LLVMBuildGlobalString(builder, value, name);
			LLVMTypeRef str_type = LLVMArrayType(LLVMInt8Type(), strlen(value));
			LLVMValueRef str = LLVMAddGlobal(module, str_type, name);
			LLVMSetInitializer(str, LLVMConstString(value, strlen(value), true));
			LLVMSetGlobalConstant(str, false);
			// LLVMBuildStore(builder, LLVMConstString("new value", strlen("new value"), false), str);

			// call the puts function whenever a variable is created
			// LLVMValueRef fn = LLVMGetNamedFunction(module, "printf");
			// LLVMValueRef print_function_args[] = {
			// 	LLVMBuildPointerCast(builder,
			// 						 LLVMBuildGlobalString(builder, value, name),
			// 						 LLVMInt8TypeInContext(context), "0")};

			// LLVMBuildCall(builder, fn, print_function_args, 1, "ret_val");
			// LLVMBuildRet(builder, LLVMConstInt(LLVMInt32TypeInContext(context), 0, false));
		}
	}
	consume(TOKEN_SEMICOLON, "semicolon expected as end of statement");
}

void build_fn(LLVMModuleRef module, LLVMBuilderRef builder, LLVMContextRef context)
{
	consume(TOKEN_IDENTIFIER, "expected variable name");

	// get name
	char *name = malloc(parser.previous.length * sizeof(char));
	sprintf(name, "%.*s", parser.previous.length, parser.previous.start);
	consume(TOKEN_LEFT_PAREN, "expected '('");

	int arg_count = 0;
	LLVMTypeRef fn_args_type[256] = {0};

	while (parser.current.type != TOKEN_RIGHT_PAREN)
	{
		consume(TOKEN_IDENTIFIER, "expected argument name");
		char *name = malloc(parser.previous.length * sizeof(char));
		sprintf(name, "%.*s", parser.previous.length, parser.previous.start);
		if (!match(TOKEN_COMMA))
		{
			break;
		}

		// printf("new arg :: %s\n", name);
		fn_args_type[arg_count] = LLVMInt8TypeInContext(context);
		arg_count++;
	}
	consume(TOKEN_RIGHT_PAREN, "expected ')' 2");
	consume(TOKEN_LEFT_BRACE, "expected '{'");

	// build LLVM node

	LLVMTypeRef fn_function_type = LLVMFunctionType(LLVMInt8TypeInContext(context), fn_args_type, arg_count, false);
	LLVMValueRef fn_type = LLVMAddFunction(module, name, fn_function_type);

	LLVMBasicBlockRef fn_entry = LLVMAppendBasicBlockInContext(context, fn_type, "entry");
	LLVMPositionBuilderAtEnd(builder, fn_entry);
	while (!match(TOKEN_RIGHT_BRACE))
	{
		declaration(module, builder, context);
	}

	LLVMValueRef main_function = LLVMGetNamedFunction(module, "main");
	LLVMBasicBlockRef main_entry = LLVMGetEntryBasicBlock(main_function);
	LLVMPositionBuilderAtEnd(builder, main_entry);
	// consume(TOKEN_RIGHT_BRACE, "expected '}'");
}

void build_fn_call(LLVMModuleRef module, LLVMBuilderRef builder, LLVMContextRef context, char *name)
{
	printf("name :: %s\n", name);
	int arg_count = 0;
	LLVMValueRef fn_args_type[256] = {0};

	while (parser.current.type != TOKEN_RIGHT_PAREN)
	{
		consume(TOKEN_IDENTIFIER, "expected argument name");
		char *arg_name = malloc(parser.previous.length * sizeof(char));
		sprintf(arg_name, "%.*s", parser.previous.length, parser.previous.start);
		if (!match(TOKEN_COMMA))
		{
			break;
		}

		fn_args_type[arg_count] = LLVMConstInt(LLVMInt8Type(), 12238974, false);
		arg_count++;
	}

	printf("arg_count :: %d\n", arg_count);
	LLVMValueRef fn = LLVMGetNamedFunction(module, name);
	if (!fn)
	{
		printf("error :: no function found with '%s'\n", name);
		exit(0);
	}
	LLVMBuildCall(builder, fn, fn_args_type, arg_count, "ret_val");

	consume(TOKEN_RIGHT_PAREN, "expected ')' 2");
	consume(TOKEN_SEMICOLON, "expected end of statement");
}

// BUILDERS END

void declaration(LLVMModuleRef module, LLVMBuilderRef builder, LLVMContextRef context)
{
	if (match(TOKEN_LET))
	{
		build_var(module, builder, context);
	}
	else if (match(TOKEN_FN))
	{
		build_fn(module, builder, context);
	}
	else if (match(TOKEN_IDENTIFIER))
	{
		char *var = malloc(sizeof(char) * parser.previous.length);
		sprintf(var, "%.*s", parser.previous.length, parser.previous.start);
		if (match(TOKEN_LEFT_PAREN))
		{
			build_fn_call(module, builder, context, var);
		}
	}
}

void parser_parse(LLVMModuleRef module, LLVMBuilderRef builder, LLVMContextRef context, const char *code)
{
	init_lexer(code);
	parser.had_error = false;
	parser.panic_mode = false;
	parser_advance();

	while (!match(TOKEN_EOF))
	{
		declaration(module, builder, context);
	}
}
