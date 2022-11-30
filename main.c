#include <dirent.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int rand_number(int min, int max) { return rand() % (max - min + 1) + min; }

char *rand_text(char *s) {
	char base[] = "Abram alas para o Noddy (Noddy) Com a buzina a tocar (ai ai ai) Abram alas para o Noddy (Noddy) "
				  "Todos cá fora a brincar (ai ai ai)";
	int rand_len = rand_number(1, strlen(base));

	s = (char *)malloc(rand_len + 1);

	for (int i = 0; i < rand_len; i++) {
		s[i] = base[i];
	}

	s[rand_len] = '\0';

	return s;
}

void create_pid_file(int pid, int pid_pai) {
	char str[5 + 4 + 1], *rand_txt;

	sprintf(str, "%d.pso", pid);

	FILE *f = fopen(str, "w");

	fprintf(f, "[PID Pai, PID Filho] = [%d, %d]\n", pid_pai, pid);
	printf("[PID Pai, PID Filho] = [%d, %d]\n", pid_pai, pid);

	fprintf(f, "P1G6\n");
	fprintf(f, "%s\n", rand_text(rand_txt));

	fclose(f);
}

void criar_ficheiros() {
	if (fork() == 0) {
		int pid_filho1 = getpid();

		create_pid_file(pid_filho1, getppid());

		if (fork() == 0) {
			create_pid_file(getpid(), getppid());
		}

		if (getpid() == pid_filho1 && fork() == 0) {
			create_pid_file(getpid(), getppid());
		}

		return;
	} else {
		if (fork() == 0) {
			create_pid_file(getpid(), getppid());
			return;
		}
	}
}

int pso_extencao_filtro(const struct dirent *entry) {
	size_t len = strlen(entry->d_name);
	if (len < 5) {
		return 0;
	}

	const char *ext = &entry->d_name[len - 4];

	return !strcmp(ext, ".pso");
}

int validate_scandir(int n) {
	if (n == -1) {
		printf("Não consegui ler a pasta\n");
		return 0;
	}

	if (n == 0) {
		printf("Não existem ficheiros .pso na pasta\n");
		return 0;
	}

	return 1;
}

long get_file_size(char *filename) {
	FILE *f = fopen(filename, "r");
	if (!f)
		return -1;

	long bytes = 0;

	while (fgetc(f) != EOF)
		bytes++;

	fclose(f);
	return bytes;
}

void handle_sigint(int sig_num) {
	printf("Recebi sinal de SIGINT, a terminar com SIGKILL...\n");

	raise(SIGKILL);
}

void mostrar_valores() {
	struct dirent **namelist;
	int n = scandir(".", &namelist, pso_extencao_filtro, alphasort);
	if (!validate_scandir(n))
		return;

	while (n--) {
		if (fork() == 0) {
			char *filename = namelist[n]->d_name;
			long bytes = get_file_size(filename);

			printf("Ficheiro: %s %ld bytes\n", filename, bytes);

			free(namelist[n]);

			exit(EXIT_SUCCESS);
		}
	}

	free(namelist);
}

void eliminar_ficheiros() {
	struct dirent **namelist;
	int n = scandir(".", &namelist, pso_extencao_filtro, alphasort);
	if (!validate_scandir(n))
		return;

	int del_count = 0, fail_count = 0;

	while (n--) {
		if (unlink(namelist[n]->d_name)) {
			fail_count++;
		} else {
			del_count++;
		}
		free(namelist[n]);
	}

	printf("Foram eliminados %d ficheiros (%d falharam)\n", del_count, fail_count);

	free(namelist);
}

void terminar() {
	signal(SIGINT, handle_sigint);

	if (fork() == 0) {
		kill(getppid(), SIGINT);
	}
}

int main(int argc, char *argv[]) {
	srand(time(NULL));

	int option;

	do {
		printf("1. Criar ficheiros\n"
			   "2. Mostrar valores\n"
			   "3. Eliminar ficheiros\n"
			   "4. Terminar\n");

		scanf("%d", &option);
	} while (option < 1 || option > 4);

	switch (option) {
	case 1:
		create_pid_file(getpid(), getppid());
		criar_ficheiros();
		break;

	case 2:
		mostrar_valores();
		break;

	case 3:
		eliminar_ficheiros();
		break;

	case 4:
		terminar();
		break;
	}

	return 0;
}
