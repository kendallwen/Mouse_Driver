#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char** argv){
	int fd, bytes;
	unsigned char data[3];

	const char *pDevice = "/dev/input/mouse0";

	fd = open(pDevice, O_RDWR);
	if(fd == -1){
		printf("ERROR Opening %s\n", pDevice);
		return -1;
	}

	int click;
	signed char x, y;
	while(1){
		bytes = read(fd, data, 3);
		if(bytes>0){
			printf("click: %d, x: %d, y: %d\n", data[0], data[1], data[2]);
		}
	}
	return 0;
}
