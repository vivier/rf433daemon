#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h> 
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

struct switch_info {
	unsigned short addr;
	const char *name;
};

struct switch_info switches[] = {
	{ 0x002B, "Centrale" },
	{ 0xB06F, "Baie 1" },
	{ 0x995C, "Baie 2" },
	{ 0x0761, "Porte Buhanderie" },
	{ 0x2173, "Porte Entrée" },
	{ 0x1268, "Porte Chambre" },
	{ 0xEB6A, "Porte Cuisine" },
	{ 0xFF73, "Porte Sud Salon" },
	{ 0xD45D, "Fenêtre Salon" },
	{ 0x785F, "Porte Ouest Salon" },

	{ 0x52FA, "Buanderie" },
	{ 0x5554, "Bureau" },
	{ 0x5CCF, "Escalier" },
	{ 0x52BE, "Salon" },
	{ 0x3075, "Commande 1" },
	{ 0xDC75, "Commande 2" },
	{ 0x1331, "Commande 3" },
	{ 0, NULL }
};

struct event_info {
	unsigned char code;
	const char *name;
};

struct event_info events[] = {
	{ 0x02, "Désarmée" },
	{ 0x04, "Alarme" },
	{ 0x08, "Armée" },
	{ 0x50, "Ouverture" },
	{ 0x5C, "Mouvement" },
	{ 0x06, "Corruption" },
	{ 0x03, "Arme (Home) " },
	{ 0x0C, "Désarme" },
	{ 0x30, "S.O.S." },
	{ 0xC0, "Arme (Away)" },
	{ 0, NULL }
};

static void print_event(int value)
{
	unsigned short addr;
	unsigned char event;
	int i;

	addr = value >> 8;
	event = value & 0xff;

	for (i = 0; switches[i].name; i++) {
		if (switches[i].addr == addr) {
			printf("%s: ", switches[i].name);
 			break;
		}
	}

	if (!switches[i].name) {
		printf("%04x: ", switches[i].addr);
	}

	for (i = 0; events[i].name; i++) {
		if (events[i].code == event) {
			printf("%s\n", events[i].name);
			break;
		}
	}

	if (!events[i].name) {
		printf("%02x\n", events[i].code);
	}
}

static int setup(int fd)
{
	struct termios port;

	tcgetattr(fd, &port);

	cfsetispeed(&port, B9600);
	cfsetospeed(&port, B9600);

	port.c_cflag &= ~PARENB;
	port.c_cflag &= ~CSTOPB;
	port.c_cflag &= ~CSIZE;
	port.c_cflag |=  CS8;
	
	port.c_cflag &= ~CRTSCTS;
	port.c_cflag |= CREAD | CLOCAL;
	
	port.c_iflag &= ~(IXON | IXOFF | IXANY);
	port.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	port.c_oflag &= ~OPOST;
	
	return tcsetattr(fd,TCSANOW,&port);
}

static int read_line(int fd, char *line, int len)
{
	int i;
	char c;
	int ret;

	for (i = 0; i < len; ) {
		ret = read(fd, &c, 1);
		if (ret == -1) {
			return ret;
		}
		if (ret == 0) {
			continue;
		}
		/* ignore CR & LF -> EOL */
		if (c == '\r') {
			continue;
		}
		if (c == '\n') {
			if (i == 0) {
				continue;
			}
			line[i] = 0;
			return i;
		}
		line[i++] = c;
	}
	return i;
}

static int run(int fd)
{
	char value[7];   /* HEX 6 digits + NUL */
	int  len;
	struct tm *tm;
	time_t t;
	char datestr[64];

	while (1) {
		len = read_line(fd, value, sizeof(value));
		if (len == -1) {
			return -1;
		}
		t = time(NULL);
		tm = localtime(&t);
		strftime(datestr, sizeof(datestr), "%c", tm);
		printf("%s %06s ", datestr, value);
		print_event(strtol(value, NULL, 16));
	}
}

int main(int argc, char **argv)
{
	int fd;
	
        fd = open(argv[1], O_RDWR | O_NOCTTY);
        if (fd == -1) {
		perror("Cannot open port");
		return 1;
	}
	if (setup(fd) == -1) {
		perror("cannot configure port");
		return 1;
	}
	tcflush(fd, TCIFLUSH);

	run(fd);

	close(fd);
}

