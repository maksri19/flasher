#include "modules.h"


void disk_newFile(const char* file_path) {
	// open new file
	FILE* fptr = fopen(file_path, "wx");
	if (fptr == NULL) {
		log_exit("Error deleting file %s: %s\n", file_path, strerror(errno));
	}

	// greetings for new file
	fclose(fptr);
}


void disk_deleteFile(const char* file_path) {
	// delete file
	if (remove(file_path) != 0) {
		log_exit("Error deleting file %s: %s\n", file_path, strerror(errno));
	}
}


int disk_getFileNameInDir(const char* folder_path, char* files, int stride, int max_file_cnt) {
	// open directory
	DIR* dir = opendir(folder_path);
	if (dir == NULL) {
		log_exit("Error opening directory %s: %s\n", folder_path, strerror(errno));
	}

	int file_cnt = 0;
	struct dirent* entry;

	while ((entry = readdir(dir)) != NULL) {
		// sort out . and  ..
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

		// check if file count exceeds limit
		if (file_cnt >= max_file_cnt) {
			log_exit("Directory exceeds max file count (%d)", max_file_cnt);
		}

		int file_name_length = strlen(entry->d_name);
		if (file_name_length >= stride) {
			file_name_length = stride-1;
		}

		// copy file name into variable
		memset(files + file_cnt * stride, 0, stride);
		memcpy(files + file_cnt * stride, entry->d_name, file_name_length);
		files[file_cnt * stride + file_name_length] = '\0';

		file_cnt++;
	}
	
	closedir(dir);
	return file_cnt;
}


int disk_readFileToBuffer(const char* file_path, char* buffer, int buffer_length) {
	// open new file
	FILE* fptr = fopen(file_path, "r");
	if (fptr == NULL) {
		log_exit("Unable to read file\n");
	}

	// get length of data in the file
	fseek(fptr, 0, SEEK_END);
	long file_length = ftell(fptr);
	rewind(fptr);

	// put data into buffer
	if (file_length > buffer_length) {
		log_exit("Data in the file exceed buffer length\n");
	}
	memset(buffer, 0, buffer_length+1);
	fread(buffer, sizeof(char), file_length, fptr);
	buffer[file_length] = '\0';

	fclose(fptr);
	return file_length;
}


void disk_writeFileFromBuffer(const char* file_path, char* buffer) {
	// open new file
	FILE* fptr = fopen(file_path, "w");
	if (fptr == NULL) {
		log_exit("Unable to read file\n");
	}

	// put data into files
	fputs(buffer, fptr);
	fclose(fptr);
}


bool disk_getPathInfo(const char* path, DiskFileInfo* info) {
	// set 0
	memset(info, 0, sizeof(DiskFileInfo));
	struct stat st;
	if (lstat(path, &st) != 0) {
		return false;
	}

	// size
	info->size = st.st_size;

	// Timestamps
	info->creatn_time_s = st.st_ctime;
	info->mod_time_s = st.st_mtime;
	info->access_time_s = st.st_atime;
	info->creatn_time_s = st.st_ctime;

	// filetype
	if (S_ISREG(st.st_mode)) {
		info->type = DISK_FILE_TYPE_REGULAR;
	}
	else if (S_ISDIR(st.st_mode)) {
		info->type = DISK_FILE_TYPE_DIRECTORY;
	}
	else if (S_ISLNK(st.st_mode)) {
		info->type = DISK_FILE_TYPE_SYMLINK;
	}
	else if (S_ISCHR(st.st_mode)) {
		info->type = DISK_FILE_TYPE_CHAR_DEVICE;
	}
	else if (S_ISBLK(st.st_mode)) {
		info->type = DISK_FILE_TYPE_BLOCK_DEVICE;
	}
	else if (S_ISFIFO(st.st_mode)) {
		info->type = DISK_FILE_TYPE_FIFO;
	}
	else if (S_ISSOCK(st.st_mode)) {
		info->type = DISK_FILE_TYPE_SOCKET;
	}
	else info->type = DISK_FILE_TYPE_UNKNOWN;

	// hidden file
	const char* slash = strrchr(path, '/');
	const char* base = slash ? (slash+1):path;
	info->is_hidden = (base[0] == '.' && strcmp(base, ".") != 0 && strcmp(base, "..") != 0);

	return true;
}


void disk_printPathInfo(DiskFileInfo* info) {

	// filetype
	if (info->type == DISK_FILE_TYPE_UNKNOWN) {
		printf("Type:\tUnknown\n");
	}
	else if (info->type == DISK_FILE_TYPE_REGULAR) {
		printf("Type:\tRegular\n");
	}
	else if (info->type == DISK_FILE_TYPE_DIRECTORY) {
		printf("Type:\tDirectory\n");
	}
	else if (info->type == DISK_FILE_TYPE_SYMLINK) {
		printf("Type:\tSymbolic link\n");
	}
	else if (info->type == DISK_FILE_TYPE_BLOCK_DEVICE) {
		printf("Type:\tBlock device\n");
	}
	else if (info->type == DISK_FILE_TYPE_CHAR_DEVICE) {
		printf("Type:\tCharacter device\n");
	}
	else if (info->type == DISK_FILE_TYPE_FIFO) {
		printf("Type:\tFifo\n");
	}
	else if (info->type == DISK_FILE_TYPE_SOCKET) {
		printf("Type:\tSocket\n");
	}

	// file size
	printf("Size:\t%zu\n", info->size);

	// Timestamps
	printf("Created:\t%s\n", get_unix_epoch_time_string(info->creatn_time_s));
	printf("Modified:\t%s\n", get_unix_epoch_time_string(info->mod_time_s));
	printf("Accessed:\t%s\n", get_unix_epoch_time_string(info->access_time_s));

	// Hidden
	if (info->is_hidden) {
		printf("Hidden\n");
	}
}


void disk_getAppDataPath(char* buffer, int buffer_length, const char* prog_name) {
	const char* home_path = getenv("HOME");
	snprintf(buffer, buffer_length, "%s/.local/share/%s/", home_path, prog_name);

}
