# Mouse_Driver

Linux mouse driver for any generic 5-button mouse. The mouse driver will allow for remapping of the mouse buttons to any other mouse button as well as any keyboard key.

## How to use

To initialize the device
```
make
sudo insmod KW_IA_Mouse_Driver.ko
sudo ./device
```
The user will be prompted for the path to the mouse input file. This can be found by looking in the /dev/input/by-id/ folder. The file to be used has the name usb-"device name"-event-mouse.

## Debugging
```
dmesg
```
