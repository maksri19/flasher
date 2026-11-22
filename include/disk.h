#ifndef FILE_H
#define FILE_H

#include "common.h"

#define DISK_MAX_PATH_LENGTH 260


typedef enum DiskFileType DiskFileType;
typedef struct DiskFileInfo DiskFileInfo;

enum DiskFileType {
	DISK_FILE_TYPE_UNKNOWN = 0,
	DISK_FILE_TYPE_REGULAR,
	DISK_FILE_TYPE_DIRECTORY,
	DISK_FILE_TYPE_SYMLINK,
	DISK_FILE_TYPE_CHAR_DEVICE,
	DISK_FILE_TYPE_BLOCK_DEVICE,
	DISK_FILE_TYPE_FIFO,
	DISK_FILE_TYPE_SOCKET
};

struct DiskFileInfo {
	// metadata
	DiskFileType	type;
	size_t			size;
	
	// timestamps
	int64_t	creatn_time_s;
	int64_t	mod_time_s;
	int64_t	access_time_s;

	// miscellenous
	bool	is_hidden;
};


void	disk_newFile(const char* file_path);
void	disk_deleteFile(const char* file_path);

int disk_getFileNameInDir(
	const char*	folder_path,
	char*		files,
	int			stride,
	int			max_file_cnt
);

int disk_readFileToBuffer(
	const char*	file_path,
	char*		buffer,
	int			buffer_length
);

void disk_writeFileFromBuffer(
	const char*	file_path,
	char*		buffer
);

bool disk_getPathInfo(const char* path, DiskFileInfo* info);

void disk_printPathInfo(DiskFileInfo* info);

void disk_getAppDataPath(char* buffer, int buffer_length, const char* prog_name);


#endif
