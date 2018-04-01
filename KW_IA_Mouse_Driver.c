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
char brightness_buff[10];
char mouse_buff[sizeof(struct input_event)];
int type, code, value;
struct file *mouseFile, *brightnessFile;
static struct class* kimClass = NULL;
static struct device* kimDevice = NULL;
int readNum = 0;

struct file* file_open(const char* path, int flags, int rights)
{
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

void file_close(struct file* file)
{
	filp_close(file, NULL);
}

int file_read(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size)
{
	mm_segment_t oldfs;
	int ret;
	oldfs = get_fs();
	set_fs(get_ds());
	ret = vfs_read(file, data, size, &offset);
	set_fs(oldfs);
	return ret;
}

int file_write(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size)
{
	mm_segment_t oldfs;
	int ret;
	oldfs = get_fs();
	set_fs(get_ds());
	ret = vfs_write(file, data, size, &offset);
	set_fs(oldfs);
	return ret;
}

static int KW_IA_Mouse_Driver_open(struct inode * inode, struct file *file){
    printk(KERN_ERR "\"KW_IA_Mouse_Driver\" is open\n");
    return 0;
}


// got it working for my razer mouse because it happened to be plugged in
static ssize_t KW_IA_Mouse_Driver_read(struct file *file, char *buffer, size_t len, loff_t *offset){
		struct input_event *ie = kmalloc(sizeof(struct input_event), GFP_USER);
		mouseFile = file_open("/dev/input/by-id/usb-Razer_Razer_DeathAdder_2013-event-mouse", 0, 0);
		char mouse_buff[72];
		file_read(mouseFile, 0, mouse_buff, 72);
		brightnessFile = file_open("/sys/class/backlight/intel_backlight/brightness", 0, 0);
		file_read((struct file*)brightnessFile, 0, brightness_buff, 2);
		int brightness = 0;
		brightness = (brightness_buff[0]-'0') * 10;
		brightness += (brightness_buff[1]-'0');
		if(mouse_buff[42]+'0'==68 && mouse_buff[44]+'0'==49){
			if(brightness<95) brightness += 5;
			printk("forward");
		}else if(mouse_buff[42]+'0'==67 && mouse_buff[44]+'0'==49){
			if(brightness>5) brightness-=5;
			printk("back");
		}else printk("not forward or back");
		brightness_buff[0] = brightness/10 + '0';
		brightness_buff[1] = brightness%10 + '0';
		printk("brightness: %d", brightness);
		file_close((struct file*) brightnessFile);
		brightnessFile = file_open("/sys/class/backlight/intel_backlight/brightness", 1, 0);
		file_write((struct file*)brightnessFile, 0, brightness_buff, 2);
		file_close((struct file*)brightnessFile);
		file_close((struct file*)mouseFile);
		return 0;
}

static ssize_t KW_IA_Mouse_Driver_write(struct file* filep, const char *buff, size_t len, loff_t *off){
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
	majorNumber = register_chrdev(45, DEVICE_NAME, &KW_IA_Mouse_Driver_fops);
    if (majorNumber<0){
        printk(KERN_ALERT "KW_IA_Mouse_Driver FAILD TO REGISTER A MAJOR NUMBER\n");
        return majorNumber;
    }
    printk(KERN_INFO "KW_IA_Mouse_Driver REGISTERED WITH MAJOR NUMBER %d\n", majorNumber);
	return 0;
}

static void __exit KW_IA_Mouse_Driver_cleanup(void){
    unregister_chrdev(majorNumber, DEVICE_NAME);
}

module_init(KW_IA_Mouse_Driver_init);
module_exit(KW_IA_Mouse_Driver_cleanup);
