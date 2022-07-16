#include "rim.h"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("%serror%s: incorrect number of arguments\n", RED_BOLD, RESET);
		printf("\nusage:: rimc <filename>\n");
		exit(0);
	}

	LLVMExecutionEngineRef engine;
	LLVMModuleRef module = LLVMModuleCreateWithName(argv[1]);
	LLVMContextRef context = LLVMContextCreate();
	LLVMBuilderRef builder = LLVMCreateBuilder();

	char *msg;
	if (LLVMCreateExecutionEngineForModule(&engine, module, &msg) == 1)
	{
		fprintf(stderr, "%serror%s: %s\n", RED_BOLD, RESET, msg);
		LLVMDisposeMessage(msg);
	}

	// initialize LLVM stuff..
	LLVMInitializeNativeTarget();
	LLVMLinkInMCJIT();
	LLVMPassManagerRef pass_manager = LLVMCreateFunctionPassManagerForModule(module);
	LLVMAddPromoteMemoryToRegisterPass(pass_manager);
	LLVMAddInstructionCombiningPass(pass_manager);
	LLVMAddReassociatePass(pass_manager);
	LLVMAddGVNPass(pass_manager);
	LLVMAddCFGSimplificationPass(pass_manager);
	LLVMInitializeFunctionPassManager(pass_manager);

	// create main function
	LLVMTypeRef main_function_type = LLVMFunctionType(LLVMInt32TypeInContext(context), NULL, 0, false);
	LLVMValueRef main_function = LLVMAddFunction(module, "main", main_function_type);

	LLVMValueRef func = LLVMGetNamedFunction(module, "printf");
	if (!func)
	{
		LLVMTypeRef print_args_type[] = {
			LLVMInt8TypeInContext(context)};

		LLVMTypeRef print_function_type = LLVMFunctionType(LLVMInt8TypeInContext(context), print_args_type, 1, false);
		LLVMAddFunction(module, "printf", print_function_type);
	};
	LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(context, main_function, "entry");
	LLVMPositionBuilderAtEnd(builder, entry);

	char *code = get_file_contents(argv[1]);
	parser_parse(module, builder, context, code);

	// return exit code 0
	LLVMBuildRet(builder, LLVMConstInt(LLVMInt32TypeInContext(context), 0, false));

	// int line = -1;
	// for (;;)
	// {
	// 	token_t token = scan_token();
	// 	if (token.line != line)
	// 	{
	// 		printf("%4d ", token.line);
	// 		line = token.line;
	// 	}
	// 	else
	// 	{
	// 		printf("   | ");
	// 	}
	// 	printf("%2d '%.*s'\n", token.type, token.length, token.start);

	// 	if (token.type == TOKEN_EOF)
	// 		break;
	// }

	// free up everything
	LLVMDumpModule(module);
	LLVMDisposePassManager(pass_manager);
	LLVMDisposeBuilder(builder);
	LLVMDisposeModule(module);

	return EXIT_SUCCESS;
}
