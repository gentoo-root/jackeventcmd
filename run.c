#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

struct list_item_t {
	char *command;
	struct list_item_t *next;
};

struct list_item_t *last[12], *head[12];

int read_config()
{
	memset(head, 0, sizeof(head));
	memset(last, 0, sizeof(last));

	char path[1024];
	path[sizeof(path)-1] = 0;
	strncpy(path, getenv("HOME"), sizeof(path)-1);
	strncat(path, "/.config/jackeventcmd", sizeof(path)-strlen(path)-2);
	int configfd = open(path, O_RDONLY);
	if (configfd < 0) {
		if ((configfd = open("/usr/share/config/jackeventcmd", O_RDONLY)) < 0) {
			perror("open");
			return 0;
		}
	}

	FILE *configfile = fdopen(configfd, "rt");
	char buf[10240];
	buf[sizeof(buf)-1] = 0;
	while (!feof(configfile)) {
		if (fgets(buf, sizeof(buf)-1, configfile) == NULL)
			break;
		if (buf[0] == '#') {
			continue;
		}
		char *p = strchr(buf, ':');
		int cnt = p-buf;
		int num = -1;
		if (!strncmp(buf, "headphone removed", cnt)) {
			num = 0;
		} else
		if (!strncmp(buf, "microphone removed", cnt)) {
			num = 2;
		} else
		if (!strncmp(buf, "lineout removed", cnt)) {
			num = 4;
		} else
		if (!strncmp(buf, "videoout removed", cnt)) {
			num = 6;
		} else
		if (!strncmp(buf, "linein removed", cnt)) {
			num = 8;
		} else
		if (!strncmp(buf, "unknown removed", cnt)) {
			num = 10;
		} else
		if (!strncmp(buf, "headphone inserted", cnt)) {
			num = 1;
		} else
		if (!strncmp(buf, "microphone inserted", cnt)) {
			num = 3;
		} else
		if (!strncmp(buf, "lineout inserted", cnt)) {
			num = 5;
		} else
		if (!strncmp(buf, "videoout inserted", cnt)) {
			num = 7;
		} else
		if (!strncmp(buf, "linein inserted", cnt)) {
			num = 9;
		} else
		if (!strncmp(buf, "unknown inserted", cnt)) {
			num = 11;
		}
		if (num == -1) {
			buf[strlen(buf)-1] = 0;
			fprintf(stderr, "Invalid config line: \"%s\"\n", buf);
			continue;
		}
		int cmdlen = strlen(buf)-cnt-2;
		if (last[num] == NULL) {
			last[num] = head[num] = malloc(sizeof(struct list_item_t));
		} else {
			last[num]->next = malloc(sizeof(struct list_item_t));
			last[num] = last[num]->next;
		}
		last[num]->next = NULL;
		last[num]->command = malloc(cmdlen+1);
		strncpy(last[num]->command, p+1, cmdlen);
		last[num]->command[cmdlen] = 0;
		char *temp = malloc(cnt+1);
		strncpy(temp, buf, cnt);
		temp[cnt] = 0;
		printf("Installed handler for %s: \"%s\"\n", temp, last[num]->command);
		free(temp);
	}

	close(configfd);

	return 1;
}

void run_cmds(int num)
{
	struct list_item_t *p;
	switch (fork()) {
	case -1:
		return;
	case 0:
		p = head[num];
		while (p) {
			printf("Running \"%s\"\n", p->command);
			if (system(p->command)) ;
			p = p->next;
		}
	}
}
