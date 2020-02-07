#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>      // Uptime display
#include <sys/ioctl.h> // Linux specific

// Constant definitions
// --------------------

#define MA_LOAD_TABLE_ENTRIES        11

// Uptime display constants
#define MA_DAY_LENGTH_MS       86400000
#define MA_HOUR_LENGTH_MS       3600000
#define MA_MINUTE_LENGTH_MS       60000
#define MA_SECOND_LENGTH_MS        1000

// Header declarations -- too few to externalize
// ---------------------------------------------

int main();
// inline here := It is only used once <=> used similar to a macro
static inline bool is_help(char* arg);  
static inline int evalparam(char* arg);
static void help();

struct color_assoc {
	double lim;
	char* color;
};

static int loadmon(unsigned int delay_sec);
static inline void drop_privileges();
static void term();
static inline void assoc_data(struct color_assoc* a);
static inline void handle_line_wraping(int* x, int* y, int olen);
static inline void print_uptime_report(int w);
static inline void print_colorized(double* l, struct color_assoc* a, char* o);
static inline void delay(int delay_sec);

// Main application
// ----------------

static bool active; // allows pro-forma cancelling

int main(int argc, char* argv[])
{
	active = true;
	if(argc > 1) {
		if(is_help(argv[1])) {
			help(argv[0]);
			return EXIT_FAILURE;
		} else {
			return evalparam(argv[1]);
		}
	} else {
		return loadmon(60);
	}
}

static inline bool is_help(char* arg) {
	return strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0;
}

static void help(char* appname)
{
	printf(
"Ma_Sys.ma Load AVG Monitor 1.1.0.0, Copyright (c) 2013 Ma_Sys.ma.\n"
"For further info send an e-mail to Ma_Sys.ma@web.de.\n\n"
"USAGE %s [delay_sec]\n\n"
"delay_sec\n"
"\tAllows you to configure a different delay between updates.\n\n"
"t := thin\n"
"gray < tblue < blue < tcyan < cyan < tyellow < yellow < tmangentha < \n"
"\tmangentha < tred < red\n",
		appname
	);
}

static inline int evalparam(char* arg)
{
	unsigned int delay_sec;
	if(sscanf(arg, "%u", &delay_sec) == 1) {
		if(delay_sec == 0) {
			fputs("WARNING: Zero delay detected. Debug only.\n",
									stderr);
		}
		return loadmon(delay_sec);
	} else {
		fputs("Failed to parse input.\n", stderr);
	}
	return EXIT_FAILURE;
}

static int loadmon(unsigned int delay_sec)
{
	drop_privileges();
	if(signal(SIGINT, term) == SIG_ERR) {
		fputs("Unable to trap SIGINT.\n", stderr);
		return EXIT_FAILURE;
	}

	struct color_assoc a[MA_LOAD_TABLE_ENTRIES];
	assoc_data(a);

	char output[32];
	double l[3];
	int olen, x = 0, y = 1;

	while(active) {
		if(getloadavg(l, 3) != 3) {
			fputs("Unable to obtain lavg.\n", stderr);
			return EXIT_FAILURE;
		}
		olen = sprintf(output, "%6.2f%6.2f%6.2f ", l[0], l[1], l[2]);
		handle_line_wraping(&x, &y, olen);
		print_colorized(l, a, output);
		delay(delay_sec);
	}

	return EXIT_SUCCESS;
}

// Try to run as "daemon" user
static inline void drop_privileges()
{
	if(geteuid() == 0) {
		if(seteuid(1) != 0) {
			fputs("Could not drop user to daemon.", stderr);
		}
	}
}

static void term()
{
	active = false;
	putc('\n', stderr);
	fflush(stdout);
}

#define MA_ALOAD(l, c) \
	a[i].lim = l; a[i].color = c; i++;

static inline void assoc_data(struct color_assoc* a)
{
	int i = 0;
	MA_ALOAD(0.05, "1;30"); // gray
	MA_ALOAD(0.12,   "34"); // thin blue
	MA_ALOAD(0.30, "1;34"); // blue
	MA_ALOAD(0.60,   "36"); // thin cyan
	MA_ALOAD(1.20, "1;36"); // cyan
	MA_ALOAD(2.00,   "33"); // thin yellow
	MA_ALOAD(3.00, "1;33"); // yellow
	MA_ALOAD(4.00,   "35"); // thin mangentha
	MA_ALOAD(7.00, "1;35"); // mangentha
	MA_ALOAD(9.00,   "31"); // thin red
	MA_ALOAD(0.00, "1;31"); // fall-through -- thick red
}

static inline void handle_line_wraping(int* x, int* y, int olen)
{
	struct winsize terminal;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminal); // Linux specific
	(*x) += olen;
	if(*x > terminal.ws_col) {
		putc('\n', stdout);
		(*x) = olen;
		(*y)++;
		if(*y >= terminal.ws_row) {
			print_uptime_report(terminal.ws_col);
			(*y) = 1;
		}
	}
}

static inline void print_uptime_report(int w)
{
	struct timespec raw;
	clock_gettime(CLOCK_MONOTONIC, &raw); // tv_sec tv_nsec
	long long int ms      = raw.tv_sec * 1000l + raw.tv_nsec / 1000000l;
	const long long int d = ms / MA_DAY_LENGTH_MS;
	ms                    = ms % MA_DAY_LENGTH_MS;
	const long long int h = ms / MA_HOUR_LENGTH_MS;
	ms                    = ms % MA_HOUR_LENGTH_MS;
	const long long int m = ms / MA_MINUTE_LENGTH_MS;
	ms                    = ms % MA_MINUTE_LENGTH_MS;
	const long long int s = ms / MA_SECOND_LENGTH_MS;
	ms                    = ms % MA_SECOND_LENGTH_MS;

	fputs("\033[30;47m", stdout);
	int written = printf("-- Uptime report: %lld d, %lld h, %lld min, "
					"%lld sec, %lld ms --", d, h, m, s, ms);
	while(written < w) {
		putc('-', stdout);
		written++;
	}
	puts("\033[0m");
}

static inline void print_colorized(double* load, struct color_assoc* a, char* o)
{
	int i;
	for(i = 0; i < MA_LOAD_TABLE_ENTRIES; i++) {
		if(load[0] <= a[i].lim || i == MA_LOAD_TABLE_ENTRIES - 1) {
			printf("\033[%s;40m%s\033[0m", a[i].color, o);
			fflush(stdout);
			return;
		}
	}
}

static inline void delay(int delay_sec)
{
	if(delay_sec == 0) {
		usleep(30000);
	} else {
		sleep(delay_sec);
	}
}
