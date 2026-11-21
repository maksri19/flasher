#include "modules.h"


void getAlphanumString(char* str, int len) {
	// constant charset string
    const char cs[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    int cs_len = sizeof(cs) - 1;

	// randomly select character from charset
    for (int i = 0; i < len; i++) {
        int key = rand() % cs_len;
        str[i] = cs[key];
    }
    str[len] = '\0';
}


uint64_t getTimeNs(void) {
    uint64_t value = glfwGetTimerValue();
    uint64_t freq = glfwGetTimerFrequency();
    uint64_t seconds = value / freq;
    uint64_t remainder = value % freq;
    return (seconds * 1000000000ULL) + ((remainder * 1000000000ULL) / freq);
}


int	getRandomInt(int low, int high) {
	int num = (rand() % (high-low+1)) + low;
	return num;
}


size_t get_file_size(const char *file_path) {
	// open file
    FILE *fp = fopen(file_path, "rb");
    if (fp == NULL) {
		log_exit("Error opening file\n");
	}

	// file size
    if (fseek(fp, 0L, SEEK_END) != 0) {
        fclose(fp);
        return -1;
    }
    size_t size = ftell(fp);

	// close file and return
    fclose(fp);
    return size;
}

