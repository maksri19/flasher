#ifndef FILE_H
#define FILE_H

#include "file.h"


void	file_newFile(const char* file_path);
void	file_deleteFile(const char* file_path);

int file_getFileNameInDir(
	const char*	folder_path,
	char*		files,
	int			stride,
	int			max_file_cnt,
	int 		max_file_name_length
);

int file_readFileToBuffer(
	const char*	file_path,
	char*		buffer,
	int			buffer_length
);

void file_writeFileFromBuffer(
	const char*	file_path,
	char*		buffer
);


#endif
