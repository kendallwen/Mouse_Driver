#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include <linux/input.h>

int main(int argc, char** argv){
	int mf, bf, kf;
	char brightness_buff[10];
	int brightness = 0;
	unsigned char data[3];

	const char *pDevice = "/dev/input/event8";
	const char *brightnessFile = "/sys/class/backlight/intel_backlight/brightness";
	const char *keyboardFile = "/dev/input/by-path/platform-i8042-serio-0-event-kbd";

	mf = open(pDevice, O_RDWR);
	bf = open(brightnessFile, O_RDWR);
	kf = open(keyboardFile, O_RDWR);
	// if(mf == -1){
	// 	printf("ERROR Opening %s\n", pDevice);
	// 	return -1;
	// }
	// if(bf==-1){
	// 	printf("ERROR Opening %s\n", brightnessFile);
	// }
	// if(read(bf, &brightness_buff, 2)==-1){
	// 	printf("Unable to read brightness\n");
	// }
    //
	// brightness = (brightness_buff[0]-'0') * 10;
	// brightness += (brightness_buff[1]-0);

	struct input_event ie;
	while(read(mf, &ie, sizeof(struct input_event))){
		printf("type %d code %d value %d\n", ie.type, ie.code, ie.value);
	}
	return 0;
}

// NEED TO GENERALIZE THIS TO DEV_DRIVER.C
