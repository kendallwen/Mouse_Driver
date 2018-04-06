#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include <linux/input.h>

int main(int argc, char** argv){
	int kf;
	const char *keyboardFile = "/dev/input/by-id/usb-CM_Storm_Quickfire_Pro_Ultimate_6_key-event-kbd";
	kf = open(keyboardFile, O_RDWR);

	struct input_event ie;
	while(read(kf, &ie, sizeof(struct input_event))){
		printf("type %d code %d value %d\n", ie.type, ie.code, ie.value);
	}
	return 0;
}

// NEED TO GENERALIZE THIS TO DEV_DRIVER.C
