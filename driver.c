#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <sys/stat.h>
#include <sys/types.h>

volatile sig_atomic_t writing = false;
volatile int fp;

int main(){
	char *buffer = malloc(1024);
	fp = open("/dev/KW_IA_Mouse_Driver", O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
	printf("Please enter the path to the mouse input event file: \n");
	int ret=0;
	while(ret!=1){
		scanf("%s", buffer);
		printf("file path: %s\n", buffer);
		if(write(fp, buffer, strlen(buffer))!=1) printf("WRONG FILE PATH\n");
		else ret=1;
	}
	printf("Succesfully found the mouse input event file\n");
	// printf("Please enter the path to the keyboard input event file: \n");
	// ret = 0;
	// while(ret!=1){
	// 	scanf("%s", buffer);
	// 	printf("file path: %s\n", buffer);
	// 	if(write(fp, buffer, strlen(buffer))!=1) printf("WRONG FILE PATH\n");
	// 	else ret=1;
	// }
	int pid = fork();
	if(pid<0){
		int errnum;
		printf("UNABLE TO FORK FOR READ/WRITE\n");
		printf("ERROR: %s\n", strerror(errnum));
		return errnum;
	}

	if(pid==0){
		while (1){
			if(!writing) read(fp, buffer, 1);
		}
	}else{
		while(1){
			printf("you can now assign new mouse keys: ");
			char s1[200], s2[200];
			scanf("%s %s", s1, s2);
			strcat(s1, " ");
			buffer = strcat(s1, s2);
			printf("Assigning new mouse key setting\n");
			writing = true;
			if(write(fp, buffer, strlen(buffer))==0) printf("Unable to assign the key\n");
			writing = false;
		}
	}
	return 0;
}
