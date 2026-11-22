#ifndef UTILS_H
#define UTILS_H


void getAlphanumString(char* str, int len);

uint64_t getTimeNs(void);

int	getRandomInt(int low, int high);

size_t get_file_size(const char *filename);


static int clampInt(int low, int val, int high) {
	if (val < low) return low;
	if (val > high) return high;
	return val;
}


static double clampDouble(double low, double val, double high) {
	if (val < low) return low;
	if (val > high) return high;
	return val;
}


static int maxInt(int a, int b) {
	if (a > b) return a;
	else return b;
}


static int minInt(int a, int b) {
	if (a < b) return a;
	else return b;
}


static bool is_tab_or_space(char c) {
	if (c == '\t' || c == ' ') return true;
	else return false;
}


static const char* get_unix_epoch_time_string(time_t ep) {
	struct tm* time_info;
	static char buffer[64];
	
	time_info = localtime(&ep);
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", time_info);

	return buffer;
}


#endif
