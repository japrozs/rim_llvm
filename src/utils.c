#include "rim.h"

char *get_file_contents(const char *path)
{
	char *contents;
	int len;
	FILE *fp = fopen(path, "r");
	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	contents = malloc(sizeof(char) * (len + 1));
	fread(contents, 1, len, fp);
	contents[len] = '\0';
	fclose(fp);

	return contents;
}
