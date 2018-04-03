#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/unistd.h>
#include <linux/buffer_head.h>
#include <linux/input.h>
#include <asm/segment.h>
#include <asm/uaccess.h>

#define DEVICE_NAME "KW_IA_Mouse_Driver"
#define CLASS_NAME "KWIAMD"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kendall Wen and Imdad Ali");
MODULE_DESCRIPTION("Zelotes mouse driver");

static int majorNumber;
static struct class* KIMClass = NULL;
static struct device* KIMDevice = NULL;
char brightness_buff[10];
char mouse_buff[sizeof(struct input_event)];
int type, code, value;
char mfName[512];
char kfName[512];
struct file *mouseFile, *brightnessFile;
int readNum = 0;

struct mouse_keys{
	struct input_event *left;
	struct input_event *right;
	struct input_event *middle;
	struct input_event *forward;
	struct input_event *back;
	struct input_event *scroll_up;
	struct input_event *scroll_down;
};

static struct mouse_keys * mk = NULL;

struct file* file_open(const char* path, int flags, int rights){
	struct file* filp = NULL;
	mm_segment_t oldfs;
	int err = 0;
	oldfs = get_fs();
	set_fs(get_ds());
	filp = filp_open(path, flags, rights);
	set_fs(oldfs);
	if (IS_ERR(filp))
	{
		err = PTR_ERR(filp);
		return NULL;
	}
	return filp;
}

void file_close(struct file* file){
	filp_close(file, NULL);
}

int file_read(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size){
	mm_segment_t oldfs;
	int ret;
	oldfs = get_fs();
	set_fs(get_ds());
	ret = vfs_read(file, data, size, &offset);
	set_fs(oldfs);
	return ret;
}

int file_write(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size){
	mm_segment_t oldfs;
	int ret;
	oldfs = get_fs();
	set_fs(get_ds());
	ret = vfs_write(file, data, size, &offset);
	set_fs(oldfs);
	return ret;
}

int numSpace(const char* string, int size){
	int spaces = 0;
	int i=0;
	while(i<size){
		if(string[i]==' ') spaces++;
		i++;
	}
	return spaces;
}

int spacePos(const char* string, int size){
	int i=0;
	while(i<size){
		if(string[i]==' ') return i;
		i++;
	}
	return -1;
}

int getCode(char * output){
	if(strcmp(output, "esc")==0){
		return 1;
	}else if(strcmp(output, "f1")==0){
		return 59;
	}else if(strcmp(output, "f2")==0){
		return 60;
	}else if(strcmp(output, "f3")==0){
		return 61;
	}else if(strcmp(output, "f4")==0){
		return 62;
	}else if(strcmp(output, "f5")==0){
		return 63;
	}else if(strcmp(output, "f6")==0){
		return 64;
	}else if(strcmp(output, "f7")==0){
		return 65;
	}else if(strcmp(output, "f8")==0){
		return 66;
	}else if(strcmp(output, "f9")==0){
		return 67;
	}else if(strcmp(output, "f10")==0){
		return 68;
	}else if(strcmp(output, "f11")==0){
		return 87;
	}else if(strcmp(output, "f12")==0){
		return 88;
	}else if(strcmp(output, "pscreen")==0){
		return 99;
	}else if(strcmp(output, "delete")==0){
		return 111;
	}else if(strcmp(output, "home")==0){
		return 102;
	}else if(strcmp(output, "end")==0){
		return 107;
	}else if(strcmp(output, "pup")==0){
		return 104;
	}else if(strcmp(output, "pdown")==0){
		return 109;
	}else if(strcmp(output, "`")==0){
		return 41;
	}else if(strcmp(output, "1")==0){
		return 2;
	}else if(strcmp(output, "2")==0){
		return 3;
	}else if(strcmp(output, "3")==0){
		return 4;
	}else if(strcmp(output, "4")==0){
		return 5;
	}else if(strcmp(output, "5")==0){
		return 6;
	}else if(strcmp(output, "6")==0){
		return 7;
	}else if(strcmp(output, "7")==0){
		return 8;
	}else if(strcmp(output, "8")==0){
		return 9;
	}else if(strcmp(output, "9")==0){
		return 10;
	}else if(strcmp(output, "0")==0){
		return 11;
	}else if(strcmp(output, "-")==0){
		return 12;
	}else if(strcmp(output, "=")==0){
		return 13;
	}else if(strcmp(output, "backspace")==0){
		return 14;
	}else if(strcmp(output, "numlock")==0){
		return 69;
	}else if(strcmp(output, "n/")==0){
		return 98;
	}else if(strcmp(output, "n*")==0){
		return 55;
	}else if(strcmp(output, "n-")==0){
		return 74;
	}else if(strcmp(output, "tab")==0){
		return 15;
	}else if(strcmp(output, "q")==0){
		return 16;
	}else if(strcmp(output, "w")==0){
		return 17;
	}else if(strcmp(output, "e")==0){
		return 18;
	}else if(strcmp(output, "r")==0){
		return 19;
	}else if(strcmp(output, "t")==0){
		return 20;
	}else if(strcmp(output, "y")==0){
		return 21;
	}else if(strcmp(output, "u")==0){
		return 22;
	}else if(strcmp(output, "i")==0){
		return 23;
	}else if(strcmp(output, "o")==0){
		return 24;
	}else if(strcmp(output, "p")==0){
		return 25;
	}else if(strcmp(output, "[")==0){
		return 26;
	}else if(strcmp(output, "]")==0){
		return 27;
	}else if(strcmp(output, "\\")==0){
		return 43;
	}else if(strcmp(output, "n7")==0){
		return 71;
	}else if(strcmp(output, "n8")==0){
		return 72;
	}else if(strcmp(output, "n9")==0){
		return 73;
	}else if(strcmp(output, "n+")==0){
		return 78;
	}else if(strcmp(output, "capslock")==0){
		return 58;
	}else if(strcmp(output, "a")==0){
		return 30;
	}else if(strcmp(output, "s")==0){
		return 31;
	}else if(strcmp(output, "d")==0){
		return 32;
	}else if(strcmp(output, "f")==0){
		return 33;
	}else if(strcmp(output, "g")==0){
		return 34;
	}else if(strcmp(output, "h")==0){
		return 35;
	}else if(strcmp(output, "j")==0){
		return 36;
	}else if(strcmp(output, "k")==0){
		return 37;
	}else if(strcmp(output, "l")==0){
		return 38;
	}else if(strcmp(output, ";")==0){
		return 49;
	}else if(strcmp(output, "\'")==0){
		return 40;
	}else if(strcmp(output, "enter")==0){
		return 28;
	}else if(strcmp(output, "n4")==0){
		return 75;
	}else if(strcmp(output, "n5")==0){
		return 76;
	}else if(strcmp(output, "n6")==0){
		return 77;
	}else if(strcmp(output, "lshift")==0){
		return 42;
	}else if(strcmp(output, "z")==0){
		return 44;
	}else if(strcmp(output, "x")==0){
		return 45;
	}else if(strcmp(output, "c")==0){
		return 46;
	}else if(strcmp(output, "v")==0){
		return 47;
	}else if(strcmp(output, "b")==0){
		return 48;
	}else if(strcmp(output, "n")==0){
		return 49;
	}else if(strcmp(output, "m")==0){
		return 50;
	}else if(strcmp(output, ",")==0){
		return 51;
	}else if(strcmp(output, ".")==0){
		return 52;
	}else if(strcmp(output, "/")==0){
		return 53;
	}else if(strcmp(output, "rshift")==0){
		return 54;
	}else if(strcmp(output, "n1")==0){
		return 79;
	}else if(strcmp(output, "n2")==0){
		return 80;
	}else if(strcmp(output, "n3")==0){
		return 81;
	}else if(strcmp(output, "nenter")==0){
		return 96;
	}else if(strcmp(output, "rctrl")==0){
		return 29;
	}else if(strcmp(output, "windows")==0){
		return 125;
	}else if(strcmp(output, "ralt")==0){
		return 56;
	}else if(strcmp(output, "space")==0){
		return 57;
	}else if(strcmp(output, "lalt")==0){
		return 100;
	}else if(strcmp(output, "lctrl")==0){
		return;
	}else if(strcmp(output, "lkey")==0){
		return 105;
	}else if(strcmp(output, "ukey")==0){
		return 103;
	}else if(strcmp(output, "dkey")==0){
		return 108;
	}else if(strcmp(output, "rkey")==0){
		return 106;
	}else if(strcmp(output, "n0")==0){
		return 82;
	}else if(strcmp(output, "n.")==0){
		return 83;
	}else if(strcmp(output, "mleft")==0){
		return 272;
	}else if(strcmp(output, "mright")==0){
		return 273;
	}else if(strcmp(output, "mforward")==0){
		return 276;
	}else if(strcmp(output, "mback")==0){
		return 275;
	}else if(strcmp(output, "mmiddle")==0){
		return 274;
	}else if(strcmp(output, "sup")==0){
		return 8;
	}else if(strcmp(output, "sdown")==0){
		return 8;
	}else return -1;
}

int getType(char * output){
	if(strcmp(output, "esc")==0){
		return 1;
	}else if(strcmp(output, "f1")==0){
		return 1;
	}else if(strcmp(output, "f2")==0){
		return 1;
	}else if(strcmp(output, "f3")==0){
		return 1;
	}else if(strcmp(output, "f4")==0){
		return 1;
	}else if(strcmp(output, "f5")==0){
		return 1;
	}else if(strcmp(output, "f6")==0){
		return 1;
	}else if(strcmp(output, "f7")==0){
		return 1;
	}else if(strcmp(output, "f8")==0){
		return 1;
	}else if(strcmp(output, "f9")==0){
		return 1;
	}else if(strcmp(output, "f10")==0){
		return 1;
	}else if(strcmp(output, "f11")==0){
		return 1;
	}else if(strcmp(output, "f12")==0){
		return 1;
	}else if(strcmp(output, "pscreen")==0){
		return 1;
	}else if(strcmp(output, "delete")==0){
		return 1;
	}else if(strcmp(output, "home")==0){
		return 1;
	}else if(strcmp(output, "end")==0){
		return 1;
	}else if(strcmp(output, "pup")==0){
		return 1;
	}else if(strcmp(output, "pdown")==0){
		return 1;
	}else if(strcmp(output, "`")==0){
		return 1;
	}else if(strcmp(output, "1")==0){
		return 1;
	}else if(strcmp(output, "2")==0){
		return 1;
	}else if(strcmp(output, "3")==0){
		return 1;
	}else if(strcmp(output, "4")==0){
		return 1;
	}else if(strcmp(output, "5")==0){
		return 1;
	}else if(strcmp(output, "6")==0){
		return 1;
	}else if(strcmp(output, "7")==0){
		return 1;
	}else if(strcmp(output, "8")==0){
		return 1;
	}else if(strcmp(output, "9")==0){
		return 1;
	}else if(strcmp(output, "0")==0){
		return 1;
	}else if(strcmp(output, "-")==0){
		return 1;
	}else if(strcmp(output, "=")==0){
		return 1;
	}else if(strcmp(output, "backspace")==0){
		return 1;
	}else if(strcmp(output, "numlock")==0){
		return 1;
	}else if(strcmp(output, "n/")==0){
		return 1;
	}else if(strcmp(output, "n*")==0){
		return 1;
	}else if(strcmp(output, "n-")==0){
		return 1;
	}else if(strcmp(output, "tab")==0){
		return 1;
	}else if(strcmp(output, "q")==0){
		return 1;
	}else if(strcmp(output, "w")==0){
		return 1;
	}else if(strcmp(output, "e")==0){
		return 1;
	}else if(strcmp(output, "r")==0){
		return 1;
	}else if(strcmp(output, "t")==0){
		return 1;
	}else if(strcmp(output, "y")==0){
		return 1;
	}else if(strcmp(output, "u")==0){
		return 1;
	}else if(strcmp(output, "i")==0){
		return 1;
	}else if(strcmp(output, "o")==0){
		return 1;
	}else if(strcmp(output, "p")==0){
		return 1;
	}else if(strcmp(output, "[")==0){
		return 1;
	}else if(strcmp(output, "]")==0){
		return 1;
	}else if(strcmp(output, "\\")==0){
		return 1;
	}else if(strcmp(output, "n7")==0){
		return 1;
	}else if(strcmp(output, "n8")==0){
		return 1;
	}else if(strcmp(output, "n9")==0){
		return 1;
	}else if(strcmp(output, "n+")==0){
		return 1;
	}else if(strcmp(output, "capslock")==0){
		return 1;
	}else if(strcmp(output, "a")==0){
		return 1;
	}else if(strcmp(output, "s")==0){
		return 1;
	}else if(strcmp(output, "d")==0){
		return 1;
	}else if(strcmp(output, "f")==0){
		return 1;
	}else if(strcmp(output, "g")==0){
		return 1;
	}else if(strcmp(output, "h")==0){
		return 1;
	}else if(strcmp(output, "j")==0){
		return 1;
	}else if(strcmp(output, "k")==0){
		return 1;
	}else if(strcmp(output, "l")==0){
		return 1;
	}else if(strcmp(output, ";")==0){
		return 1;
	}else if(strcmp(output, "\'")==0){
		return 1;
	}else if(strcmp(output, "enter")==0){
		return 1;
	}else if(strcmp(output, "n4")==0){
		return 1;
	}else if(strcmp(output, "n5")==0){
		return 1;
	}else if(strcmp(output, "n6")==0){
		return 1;
	}else if(strcmp(output, "lshift")==0){
		return 1;
	}else if(strcmp(output, "z")==0){
		return 1;
	}else if(strcmp(output, "x")==0){
		return 1;
	}else if(strcmp(output, "c")==0){
		return 1;
	}else if(strcmp(output, "v")==0){
		return 1;
	}else if(strcmp(output, "b")==0){
		return 1;
	}else if(strcmp(output, "n")==0){
		return 1;
	}else if(strcmp(output, "m")==0){
		return 1;
	}else if(strcmp(output, ",")==0){
		return 1;
	}else if(strcmp(output, ".")==0){
		return 1;
	}else if(strcmp(output, "/")==0){
		return 1;
	}else if(strcmp(output, "rshift")==0){
		return 1;
	}else if(strcmp(output, "n1")==0){
		return 1;
	}else if(strcmp(output, "n2")==0){
		return 1;
	}else if(strcmp(output, "n3")==0){
		return 1;
	}else if(strcmp(output, "nenter")==0){
		return 1;
	}else if(strcmp(output, "rctrl")==0){
		return 1;
	}else if(strcmp(output, "windows")==0){
		return 1;
	}else if(strcmp(output, "ralt")==0){
		return 1;
	}else if(strcmp(output, "space")==0){
		return 1;
	}else if(strcmp(output, "lalt")==0){
		return 1;
	}else if(strcmp(output, "lctrl")==0){
		return;
	}else if(strcmp(output, "lkey")==0){
		return 1;
	}else if(strcmp(output, "ukey")==0){
		return 1;
	}else if(strcmp(output, "dkey")==0){
		return 1;
	}else if(strcmp(output, "rkey")==0){
		return 1;
	}else if(strcmp(output, "n0")==0){
		return 1;
	}else if(strcmp(output, "n.")==0){
		return 1;
	}else if(strcmp(output, "mleft")==0){
		return 1;
	}else if(strcmp(output, "mright")==0){
		return 1;
	}else if(strcmp(output, "mforward")==0){
		return 1;
	}else if(strcmp(output, "mback")==0){
		return 1;
	}else if(strcmp(output, "mmiddle")==0){
		return 1;
	}else if(strcmp(output, "sup")==0){
		return 2;
	}else if(strcmp(output, "sdown")==0){
		return 2;
	}else return -1;
}

int getValue(char * output){
	if(strcmp(output, "esc")==0){
		return 1;
	}else if(strcmp(output, "f1")==0){
		return 1;
	}else if(strcmp(output, "f2")==0){
		return 1;
	}else if(strcmp(output, "f3")==0){
		return 1;
	}else if(strcmp(output, "f4")==0){
		return 1;
	}else if(strcmp(output, "f5")==0){
		return 1;
	}else if(strcmp(output, "f6")==0){
		return 1;
	}else if(strcmp(output, "f7")==0){
		return 1;
	}else if(strcmp(output, "f8")==0){
		return 1;
	}else if(strcmp(output, "f9")==0){
		return 1;
	}else if(strcmp(output, "f10")==0){
		return 1;
	}else if(strcmp(output, "f11")==0){
		return 1;
	}else if(strcmp(output, "f12")==0){
		return 1;
	}else if(strcmp(output, "pscreen")==0){
		return 1;
	}else if(strcmp(output, "delete")==0){
		return 1;
	}else if(strcmp(output, "home")==0){
		return 1;
	}else if(strcmp(output, "end")==0){
		return 1;
	}else if(strcmp(output, "pup")==0){
		return 1;
	}else if(strcmp(output, "pdown")==0){
		return 1;
	}else if(strcmp(output, "`")==0){
		return 1;
	}else if(strcmp(output, "1")==0){
		return 1;
	}else if(strcmp(output, "2")==0){
		return 1;
	}else if(strcmp(output, "3")==0){
		return 1;
	}else if(strcmp(output, "4")==0){
		return 1;
	}else if(strcmp(output, "5")==0){
		return 1;
	}else if(strcmp(output, "6")==0){
		return 1;
	}else if(strcmp(output, "7")==0){
		return 1;
	}else if(strcmp(output, "8")==0){
		return 1;
	}else if(strcmp(output, "9")==0){
		return 1;
	}else if(strcmp(output, "0")==0){
		return 1;
	}else if(strcmp(output, "-")==0){
		return 1;
	}else if(strcmp(output, "=")==0){
		return 1;
	}else if(strcmp(output, "backspace")==0){
		return 1;
	}else if(strcmp(output, "numlock")==0){
		return 1;
	}else if(strcmp(output, "n/")==0){
		return 1;
	}else if(strcmp(output, "n*")==0){
		return 1;
	}else if(strcmp(output, "n-")==0){
		return 1;
	}else if(strcmp(output, "tab")==0){
		return 1;
	}else if(strcmp(output, "q")==0){
		return 1;
	}else if(strcmp(output, "w")==0){
		return 1;
	}else if(strcmp(output, "e")==0){
		return 1;
	}else if(strcmp(output, "r")==0){
		return 1;
	}else if(strcmp(output, "t")==0){
		return 1;
	}else if(strcmp(output, "y")==0){
		return 1;
	}else if(strcmp(output, "u")==0){
		return 1;
	}else if(strcmp(output, "i")==0){
		return 1;
	}else if(strcmp(output, "o")==0){
		return 1;
	}else if(strcmp(output, "p")==0){
		return 1;
	}else if(strcmp(output, "[")==0){
		return 1;
	}else if(strcmp(output, "]")==0){
		return 1;
	}else if(strcmp(output, "\\")==0){
		return 1;
	}else if(strcmp(output, "n7")==0){
		return 1;
	}else if(strcmp(output, "n8")==0){
		return 1;
	}else if(strcmp(output, "n9")==0){
		return 1;
	}else if(strcmp(output, "n+")==0){
		return 1;
	}else if(strcmp(output, "capslock")==0){
		return 1;
	}else if(strcmp(output, "a")==0){
		return 1;
	}else if(strcmp(output, "s")==0){
		return 1;
	}else if(strcmp(output, "d")==0){
		return 1;
	}else if(strcmp(output, "f")==0){
		return 1;
	}else if(strcmp(output, "g")==0){
		return 1;
	}else if(strcmp(output, "h")==0){
		return 1;
	}else if(strcmp(output, "j")==0){
		return 1;
	}else if(strcmp(output, "k")==0){
		return 1;
	}else if(strcmp(output, "l")==0){
		return 1;
	}else if(strcmp(output, ";")==0){
		return 1;
	}else if(strcmp(output, "\'")==0){
		return 1;
	}else if(strcmp(output, "enter")==0){
		return 1;
	}else if(strcmp(output, "n4")==0){
		return 1;
	}else if(strcmp(output, "n5")==0){
		return 1;
	}else if(strcmp(output, "n6")==0){
		return 1;
	}else if(strcmp(output, "lshift")==0){
		return 1;
	}else if(strcmp(output, "z")==0){
		return 1;
	}else if(strcmp(output, "x")==0){
		return 1;
	}else if(strcmp(output, "c")==0){
		return 1;
	}else if(strcmp(output, "v")==0){
		return 1;
	}else if(strcmp(output, "b")==0){
		return 1;
	}else if(strcmp(output, "n")==0){
		return 1;
	}else if(strcmp(output, "m")==0){
		return 1;
	}else if(strcmp(output, ",")==0){
		return 1;
	}else if(strcmp(output, ".")==0){
		return 1;
	}else if(strcmp(output, "/")==0){
		return 1;
	}else if(strcmp(output, "rshift")==0){
		return 1;
	}else if(strcmp(output, "n1")==0){
		return 1;
	}else if(strcmp(output, "n2")==0){
		return 1;
	}else if(strcmp(output, "n3")==0){
		return 1;
	}else if(strcmp(output, "nenter")==0){
		return 1;
	}else if(strcmp(output, "rctrl")==0){
		return 1;
	}else if(strcmp(output, "windows")==0){
		return 1;
	}else if(strcmp(output, "ralt")==0){
		return 1;
	}else if(strcmp(output, "space")==0){
		return 1;
	}else if(strcmp(output, "lalt")==0){
		return 1;
	}else if(strcmp(output, "lctrl")==0){
		return;
	}else if(strcmp(output, "lkey")==0){
		return 1;
	}else if(strcmp(output, "ukey")==0){
		return 1;
	}else if(strcmp(output, "dkey")==0){
		return 1;
	}else if(strcmp(output, "rkey")==0){
		return 1;
	}else if(strcmp(output, "n0")==0){
		return 1;
	}else if(strcmp(output, "n.")==0){
		return 1;
	}else if(strcmp(output, "mleft")==0){
		return 1;
	}else if(strcmp(output, "mright")==0){
		return 1;
	}else if(strcmp(output, "mforward")==0){
		return 1;
	}else if(strcmp(output, "mback")==0){
		return 1;
	}else if(strcmp(output, "mmiddle")==0){
		return 1;
	}else if(strcmp(output, "sup")==0){
		return 1;
	}else if(strcmp(output, "sdown")==0){
		return -1;
	}else return 0;
}

int setKey(struct mouse_keys * mouse, char * input, char * output){
	if(getCode(output)<0 || getType(output)<0 || getValue(output)==0) {
		printk("NOT VALID SETTING");
		return 0;
	}
	if(strcmp("left", input)==0){
		mouse->left->type = getType(output);
		mouse->left->code = getCode(output);
		mouse->left->value = getValue(output);
		printk("left set to type: %d, code: %d, value:%d", mouse->left->type, mouse->left->code, mouse->left->value);
		return 1;
	}else if(strcmp("right", input)==0){
		mouse->right->type = getType(output);
		mouse->right->code = getCode(output);
		mouse->right->value = getValue(output);
		printk("right set to type: %d, code: %d, value:%d", mouse->right->type, mouse->right->code, mouse->right->value);
		return 2;
	}else if(strcmp("middle", input)==0){
		mouse->middle->type = getType(output);
		mouse->middle->code = getCode(output);
		mouse->middle->value = getValue(output);
		printk("middle set to type: %d, code: %d, value:%d", mouse->middle->type, mouse->middle->code, mouse->middle->value);
		return 3;
	}else if(strcmp("back", input)==0){
		mouse->back->type = getType(output);
		mouse->back->code = getCode(output);
		mouse->back->value = getValue(output);
		printk("back set to type: %d, code: %d, value:%d", mouse->back->type, mouse->back->code, mouse->back->value);
		return 4;
	}else if(strcmp("forward", input)==0){
		mouse->forward->type = getType(output);
		mouse->forward->code = getCode(output);
		mouse->forward->value = getValue(output);
		printk("forward set to type: %d, code: %d, value:%d", mouse->forward->type, mouse->forward->code, mouse->forward->value);
		return 5;
	}else if(strcmp("sup", input)==0){
		mouse->scroll_up->type = getType(output);
		mouse->scroll_up->code = getCode(output);
		mouse->scroll_up->value = getValue(output);
		printk("scroll up set to type: %d, code: %d, value:%d", mouse->scroll_up->type, mouse->scroll_up->code, mouse->scroll_up->value);
		return 5;
	}else if(strcmp("sdown", input)==0){
		mouse->scroll_down->type = getType(output);
		mouse->scroll_down->code = getCode(output);
		mouse->scroll_down->value = getValue(output);
		printk("scroll down set to type: %d, code: %d, value:%d", mouse->scroll_down->type, mouse->scroll_down->code, mouse->scroll_down->value);
		return 5;
	}else{
		printk("NOT A MOUSE KEY");
		return 0;
	}
}

static int KW_IA_Mouse_Driver_open(struct inode * inode, struct file *file){
    printk(KERN_ERR "\"KW_IA_Mouse_Driver\" is open\n");
    return 0;
}


// got it working for my razer mouse because it happened to be plugged in
static ssize_t KW_IA_Mouse_Driver_read(struct file *file, char *buffer, size_t len, loff_t *offset){
		struct input_event *ie = kmalloc(sizeof(struct input_event), GFP_USER);
		mouseFile = file_open(mfName, 0, 0);
		char mouse_buff[72];
		file_read(mouseFile, 0, mouse_buff, 72);
		if(mouse_buff[42]+'0'==68 && mouse_buff[44]+'0'==49){
			printk("forward");
		}else if(mouse_buff[42]+'0'==67 && mouse_buff[44]+'0'==49){
			printk("back");
		}else if(mouse_buff[42]+'0'==66 && mouse_buff[44]+'0'==49){
			printk("middle");
		}else if(mouse_buff[42]+'0'==65 && mouse_buff[44]+'0'==49){
			printk("right");
		}else if(mouse_buff[42]+'0'==64 && mouse_buff[44]+'0'==49){
			printk("left");
		}else printk("not a mouse click");
		file_close((struct file*)mouseFile);
		return 0;
}

static ssize_t KW_IA_Mouse_Driver_write(struct file* filep, const char *buff, size_t len, loff_t *off){
	if(numSpace(buff, len)==0){
		char *tempName = kmalloc(len+1, GFP_USER);
		if(copy_from_user(tempName, buff, len+1)) return 0;
		printk("mouse file path: %s", tempName);
		if(file_open(tempName, 0, O_RDWR)) {
			printk("file opened");
			strlcpy(mfName, tempName, len+1);
			return 1;
		}
	}else if(numSpace(buff, len)==1){
		char *data = kmalloc(len+1, GFP_USER);
		copy_from_user(data, buff, len+1);
		printk("%s", data);
		int space = spacePos(buff, len);
		char input[space+1];
		strlcpy(input, buff, space+1);
		buff += space+1;
		char output[len-space];
		strlcpy(output, buff, len-space);
		printk("input: %s output: %s", input, output);
		if(setKey(mk, input, output)==0) return 0;
		return 2;
	}
	return 0;
}

static int KW_IA_Mouse_Driver_release(struct inode *inodep, struct file *filep){
    return 0;
}

static const struct file_operations KW_IA_Mouse_Driver_fops = {
    .open = KW_IA_Mouse_Driver_open,
    .read = KW_IA_Mouse_Driver_read,
	.write = KW_IA_Mouse_Driver_write,
    .release = KW_IA_Mouse_Driver_release,
};

static int __init KW_IA_Mouse_Driver_init(void){
	majorNumber = register_chrdev(0, DEVICE_NAME, &KW_IA_Mouse_Driver_fops);
    if (majorNumber<0){
        printk(KERN_ALERT "KW_IA_Mouse_Driver FAILD TO REGISTER A MAJOR NUMBER\n");
        return majorNumber;
    }
    printk(KERN_INFO "KW_IA_Mouse_Driver REGISTERED WITH MAJOR NUMBER %d\n", majorNumber);

	KIMClass = class_create(THIS_MODULE, CLASS_NAME);
    if(IS_ERR(KIMClass)){
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "FAILED TO REGISTER DEVICE CLASS\n");
        return PTR_ERR(KIMClass);
    }
    printk(KERN_INFO "KW_IA_Mouse_Driver: DEVICE CLASS REGISTERED CORRECTLY\n");

    KIMDevice = device_create(KIMClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if(IS_ERR(KIMDevice)){
        class_destroy(KIMClass);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "FAILED TO CREATE THE DEVICE");
        return PTR_ERR(KIMDevice);
    }
    printk(KERN_INFO "KW_IA_Mouse_Driver: DEVICE CLASS CREATED CORRECTLY\n");

	mk = kmalloc(sizeof(struct mouse_keys), GFP_USER);
	mk->left = kmalloc(sizeof(struct input_event), GFP_USER);
	mk->right = kmalloc(sizeof(struct input_event), GFP_USER);
	mk->middle = kmalloc(sizeof(struct input_event), GFP_USER);
	mk->forward = kmalloc(sizeof(struct input_event), GFP_USER);
	mk->back = kmalloc(sizeof(struct input_event), GFP_USER);
	mk->scroll_up = kmalloc(sizeof(struct input_event), GFP_USER);
	mk->scroll_down = kmalloc(sizeof(struct input_event), GFP_USER);

	return 0;
}

static void __exit KW_IA_Mouse_Driver_cleanup(void){
	kfree(mk);
    device_destroy(KIMClass, MKDEV(majorNumber,0));
    class_unregister(KIMClass);
    class_destroy(KIMClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);
}

module_init(KW_IA_Mouse_Driver_init);
module_exit(KW_IA_Mouse_Driver_cleanup);
