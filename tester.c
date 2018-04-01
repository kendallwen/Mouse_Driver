#include <stdio.h>
#include <string.h>
#include <linux/fs.h>
#include <fcntl.h>

int main()
{
	char buffer[24];
	int fp = open("/dev/KW_IA_Mouse_Driver", O_RDWR);
	while (1)
	{
		read(fp, buffer, 1);
	}
	return 0;
}
