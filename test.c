#include <stdio.h>

int main(int ac, char **av, char **envp) {
	int new = 0;
	if (new < 2) return (0);
	for (int i = 1; i < new; i++){
		printf("%s\n", av[i]);
	}	
	return (0);
}

