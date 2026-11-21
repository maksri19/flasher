#include "modules.h"


void file_newFile(const char* file_path) {
	// open new file
	FILE* fptr = fopen(file_path, "wx");
	if (fptr == NULL) {
		log_exit("Unable to write file\n");
	}

	// greetings for new file
	fclose(fptr);
}


void file_deleteFile(const char* file_path) {
	// delete file
	if (remove(file_path) != 0) {
		log_exit("Error deleting file\n");
	}
}


int file_getFileNameInDir(const char* folder_path, char* files, int stride, int max_file_cnt, int max_file_name_length) {
	// open directory
    DIR* dir = opendir(folder_path);
    if (dir == NULL) {
		log_exit("Unable to open directory");
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
		if (file_name_length > max_file_name_length) {
			file_name_length = max_file_name_length;
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


int file_readFileToBuffer(const char* file_path, char* buffer, int buffer_length) {
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


void file_writeFileFromBuffer(const char* file_path, char* buffer) {
	// open new file
	FILE* fptr = fopen(file_path, "w");
	if (fptr == NULL) {
		log_exit("Unable to read file\n");
	}

	// put data into files
	fputs(buffer, fptr);
	fclose(fptr);
}

