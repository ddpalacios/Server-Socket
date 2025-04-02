char *get_file_buffer(char* filename){
	FILE *html_pcontent;
	long content_size;
	char *buffer;
	html_pcontent = fopen(filename, "r");
	if (!html_pcontent){
		perror(filename);
		exit(1);
	
	}
	fseek(html_pcontent, 0L, SEEK_END);
	content_size = ftell(html_pcontent);
	rewind(html_pcontent);
	buffer = malloc(content_size+1);
	if (!buffer){
		fclose(html_pcontent);
		free(buffer);
		fputs("entire read fails", stderr);
		exit(1);
	}
	if (1!=fread(buffer, content_size, 1, html_pcontent)){
		fclose(html_pcontent);
		free(buffer);
		fputs("entire read fails", stderr);
		exit(1);
	}
	buffer[content_size] = '\0';
	fclose(html_pcontent);
	return buffer;



}
