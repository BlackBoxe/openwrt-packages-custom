#define MY_USE_SUNXI_GPIO
#define MY_USE_DEV_URANDOM
#undef MY_USE_OPENSSL_RAND

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include <inttypes.h>
#if defined(MY_USE_OPENSSL_RAND)
#include <openssl/err.h>
#include <openssl/rand.h>
#endif
 
#include "gpio_lib.h"

#define MY_VERSION  "0.1"

#define MY_LOG_FATAL    0
#define MY_LOG_ERROR    1
#define MY_LOG_WARNING  2
#define MY_LOG_NOTICE   3
#define MY_LOG_INFO     4
#define MY_LOG_DEBUG    5

#define MY_GPIO_BLINK  1
#define MY_GPIO_PULSE  2
#define MY_GPIO_OFF    3
#define MY_GPIO_ON     4

static char *me;

static int my_running_flag = 1;

static char my_usage[] = 
	"Usage: %s OPTIONS...\n"
	"\n"
	"GPIO daemon\n"
	"\n"
	"Options:\n"
	"  -h    Display usage information and exit"
	"  -v    Display version and exit";

static char my_version[] = 
	"%s, version %s, Copyright (C) 2014 BlackBoxe <contact@blackboxe.org>\n";

#define MY_FIFO_PATH_FMT "/tmp/%s.fifo"
static int my_fifo_fd;
static char *my_fifo_path = 0;

#ifdef MY_USE_DEV_URANDOM
int my_rng_fd;
#endif

void my_get_options(int argc, char** argv)
{
	int opt;

	me = strrchr(argv[0], '\\');
	if (me == 0) {
		me = argv[0];
	} else {
		me++;
	}

	while ((opt = getopt(argc, argv, "hvf:")) != -1) {
		switch (opt){
		case 'f':
				my_fifo_path = optarg;
			break;
		case 'h':
			fprintf(stdout, my_usage, me);
			exit(0);
			break;
		default:
			fprintf(stderr, my_version, me, MY_VERSION);
			exit(0);
			break;
		}
	}
}


/* signal handler */

void my_loop_term_handler(int signum)
{
	my_running_flag = 0;
}


/* logging */

void my_log_open(void)
{
	openlog(me, LOG_NDELAY | LOG_PID, LOG_DAEMON);
}

void my_log_close(void)
{
	closelog();
}

void my_log(int level, const char *fmt, ...)
{
	int log_prio;
	va_list va;

	va_start(va, fmt);

	switch (level) {
	case MY_LOG_FATAL:
		log_prio = LOG_CRIT;
		break;
	case MY_LOG_ERROR:
		log_prio = LOG_ERR;
		break;
	case MY_LOG_WARNING:
		log_prio = LOG_WARNING;
		break;
	case MY_LOG_NOTICE:
		log_prio = LOG_NOTICE;
		break;
	case MY_LOG_DEBUG:
		log_prio = LOG_DEBUG;
		break;
	default:
		log_prio = LOG_INFO;
		break;
	}

	vsyslog(log_prio, fmt, va);
}


int my_random_init(void)
{
#if defined(MY_USE_DEV_URANDOM)
	if ((my_rng_fd = open("/dev/urandom", 0)) < 0) {
		my_log(MY_LOG_ERROR, "error opening '/dev/urandom'");
		return -1;
	}
#endif
	return 0;
}

int my_random(int min, int max)
{
#if defined(MY_USE_DEV_URANDOM)
	uint32_t n;
	if (read(my_rng_fd, (unsigned char *)&n, sizeof(n)) == 0) {
		my_log(MY_LOG_ERROR, "error reading '/dev/urandom'");
		return -1;
	}
	return min + (int)((double)(max - min) * (double)n / (double)0xFFFFFFFF);
#elif defined(MY_USE_OPENSSL_RAND)
	uint32_t n;
	if (RAND_bytes((unsigned char *)&n, sizeof(n)) == 0) {
		my_log(MY_LOG_ERROR, "error calling OpenSSL RAND_bytes");
		return -1;
	}
	return min + (int)((double)(max - min) * (double)n / (double)0xFFFFFFFF);
#else
	long int n = random();
	return min + (int)((double)(max - min) * (double)n / (double)RAND_MAX);
#endif
}


void my_gpio_init(int pin)
{
	sunxi_gpio_set_cfgpin(pin, SUNXI_GPIO_OUTPUT);
}

void my_gpio_blink(int pin)
{
	int i = my_random(50000, 200000);
	int o = my_random(25000, 75000);
	sunxi_gpio_output(pin, 1);
	usleep(i);
	sunxi_gpio_output(pin, 0);
	usleep(o);
}

void my_gpio_on(int pin) {
	sunxi_gpio_output(pin, 1);
}

void my_gpio_off(int pin) {
	sunxi_gpio_output(pin, 0);
}

#define MY_PULSE_GAMMA  2.1

void my_gpio_pulse(int pin)
{
	int time_on = 1000;
	int time_off = 0;
	int i;

	for (i = 0; i < time_on; i++) {
		time_off = (int)(pow((double)i / (double)time_on, (double)MY_PULSE_GAMMA) * (double)time_on);
		if (time_off > 0){
			sunxi_gpio_output(pin, 1);
			usleep(time_off);
			sunxi_gpio_output(pin, 0);
			usleep(time_on - time_off);
		}
	}
	for (i = time_on - 1; i >= 0; i--) {
		time_off = (int)(pow((double)i / (double)time_on, (double)MY_PULSE_GAMMA) * (double)time_on);
		if (time_off > 0){
			sunxi_gpio_output(pin, 1);
			usleep(time_off);
			sunxi_gpio_output(pin, 0);
			usleep(time_on - time_off);
		}
	}
	usleep(time_on);
}


int my_startup(void)
{
	struct sigaction sigold, signew;
	char path[FILENAME_MAX];

	my_log_open();

	memset(&signew, 0, sizeof(signew));
	signew.sa_handler = my_loop_term_handler;
	if (sigaction(SIGINT, &signew, &sigold) < 0 ) {
		my_log(MY_LOG_ERROR, "error setting INT signal handler");
		return 1;
	}
	if (sigaction(SIGTERM, &signew, &sigold) < 0 ) {
		my_log(MY_LOG_ERROR, "error setting TERM signal handler");
		return 1;
	}

#if defined(MY_USE_SUNXI_GPIO)
	if (sunxi_gpio_init() < 0) {
		my_log(MY_LOG_ERROR, "error initializing GPIOs");
		return 1;
	}
#endif

	if (my_random_init() < 0) {
		my_log(MY_LOG_ERROR, "error initializing Random Number Generator");
		return 1;
	}

	if (my_fifo_path == 0) {
		snprintf(path, sizeof(path), MY_FIFO_PATH_FMT, me);
		my_fifo_path = path;
	}
	if ((my_fifo_fd = mkfifo(my_fifo_path, S_IRUSR | S_IWUSR)) < 0) {
		my_log(MY_LOG_ERROR, "error creating fifo '%s'", my_fifo_path);
	}
	if ((my_fifo_fd = open(my_fifo_path, O_NONBLOCK)) < 0) {
		my_log(MY_LOG_ERROR, "error opening fifo '%s'", my_fifo_path);
		return 1;
	}

	return 0;
}

void my_cleanup(void)
{
	if (unlink(my_fifo_path) < 0) {
		my_log(MY_LOG_ERROR, "error removing fifo '%s'", my_fifo_path);
	}

	my_log_close();
}

void my_loop(void)
{
	char line[1024], action[6];
	int n;
	int state = 0;
	int pin;

	while (my_running_flag) {
		if( read(my_fifo_fd, line, sizeof(line)) > 0) {
			n = sscanf(line, "%s %d", action, &pin);
			if (n != 2) {
				my_log(MY_LOG_ERROR, "received unknown command '%s'", line);
			}
			if( strncmp(action, "INIT", sizeof(action)) == 0) {
				my_gpio_init(pin);
			} else if( strncmp(action, "BLINK", sizeof(action)) == 0) {
				state = MY_GPIO_BLINK;
			} else if( strncmp(action, "PULSE", sizeof(action)) == 0) {
				state = MY_GPIO_PULSE;
			} else if( strncmp(action, "OFF", sizeof(action)) == 0) {
				state = MY_GPIO_OFF;
				my_gpio_off(pin);
			} else if( strncmp(action, "ON", sizeof(action)) == 0) {
				state = MY_GPIO_ON;
				my_gpio_on(pin);
			} else {
				my_log(MY_LOG_ERROR, "received unknown command '%s'", line);
			}
		}
		if (state == MY_GPIO_BLINK) {
				my_gpio_blink(pin);
		} else if (state == MY_GPIO_PULSE) {
				my_gpio_pulse(pin);
		}
	}
}


int main(int argc, char** argv)
{
	my_get_options(argc, argv);

	if (!my_startup()) {
		my_loop();
		my_cleanup();
	}

	return 0;
}



