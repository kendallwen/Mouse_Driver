#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include <linux/input.h>

int main(int argc, char** argv){
	int mf, tf;
	char brightness_buff[10];
	int brightness = 0;
	unsigned char data[sizeof(struct input_event)];

	const char *pDevice = "/dev/input/by-id/usb-Razer_Razer_DeathAdder_2013-event-mouse";

	mf = open(pDevice, O_RDWR);
	if(mf == -1){
		printf("ERROR Opening %s\n", pDevice);
		return -1;
	}

	struct input_event ie;
	printf("input event: %ld, timeval: %ld, type: %ld, code %ld, value %ld\n",
		sizeof(ie), sizeof(ie.time), sizeof(ie.type), sizeof(ie.code), sizeof(ie.value));
	while(read(mf, &data, sizeof(struct input_event))){
		if(data[16]+'0'==49 || (data[16]+'0'==50 && data[18]+'0'==56)){
			printf("new value\n");
			printf("type: %d %d\n", data[16]+'0', data[17]+'0');
			printf("code: %d %d\n", data[18]+'0', data[19]+'0');
			printf("value: %d %d %d %d\n", data[20]+'0', data[21]+'0', data[22]+'0', data[23]+'0');
			if(data[20]+'0'=='1' && data[21]+'0'=='0' && data[22]+'0'=='0' && data[23]+'0'=='0') printf("button pressed\n");
		}
	}
	return 0;
}

// NEED TO GENERALIZE THIS TO DEV_DRIVER.C
