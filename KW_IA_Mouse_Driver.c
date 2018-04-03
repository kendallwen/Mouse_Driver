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
	struct input_event *left = kmalloc(sizeof(struct input_event));
	struct input_event *right = kmalloc(sizeof(struct input_event));
	struct input_event *middle = kmalloc(sizeof(struct input_event));
	struct input_event *forward = kmalloc(sizeof(struct input_event));
	struct input_event *back = kmalloc(sizeof(struct input_event));
	struct input_event *scroll_up = kmalloc(sizeof(struct input_event));
	struct input_event *scroll_down = kmalloc(sizeof(struct input_event));
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

int getType(char * output){
	return -1;
}

int getCode(char * ouput){
	return -1;
}

int setValues(struct mouse_keys * mouse, char * input, char * output){
	if(getCode(output)<0 || getType(ouput)<0) {
		printk("NOT VALID SETTING");
		return 0;
	}
	if(strcmp("left", input)){
		mouse->left->type = getType(output);
		mouse->left->code = getCode(output);
		return 1;
	}else if(strcmp("right", input)){
		mouse->rightt->type = getType(output);
		mouse->right->code = getCode(output);
		return 2;
	}else if(strcmp("middle", input)){
		mouse->middle->type = getType(output);
		mouse->middle->code = getCode(output);
		return 3;
	}else if(strcmp("back", input)){
		mouse->back->type = getType(output);
		mouse->back->code = getCode(output);
		return 4;
	}else if(strcmp("forward", input)){
		mouse->forward->type = getType(output);
		mouse->forward->code = getCode(output);
		return 5;
	}else if(strcmp("sup", input)){
		mouse->scroll_up->type = getType(output);
		mouse->scroll_up->code = getCode(output);
		return 5;
	}else if(strcmp("sdown", input)){
		mouse->scroll_down->type = getType(output);
		mouse->scroll_down->code = getCode(output);
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
		char *tempName = kmalloc(len, GFP_USER);
		if(copy_from_user(tempName, buff, len)) return 0;
		printk("mouse file path: %s", tempName);
		if(filp_open(tempName, 0, O_RDWR)) {
			strlcpy(mfName, tempName, len+1);
			return 1;
		}
	}else if(numSpace(buff, len)==1){
		char *data = kmalloc(len, GFP_USER);
		copy_from_user(data, buff, len);
		printk("%s", data);
		int space = spacePos(buff, len);
		char input[space+1];
		strlcpy(input, buff, space+1);
		buff += space+1;
		char output[len-space];
		strlcpy(output, buff, len-space);
		printk("input: %s output: %s", input, output);
		if(setValues(mk, input, output)==0) return 0;
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
