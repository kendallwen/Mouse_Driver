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

### Setting Mouse Buttons

To set the mouse keys to a new input, type in two arguments, the mouse button that you wish to change and the resulting input key. For example:
```
left a
```
will change the left mouse button to the a key on the keyboard.

List of inputs:
```
left = left mouse button
right = right mouse button
middle = scroll wheel click
forward = mouse front side button
back = mouse back side button
sup = scroll up
sdown = scroll down
```

Outputs only align with the base case of keys (ie. unable to output upper case characters or other symbols obtained by shift + a key). Outputs regarding a letter, number, or symbol can simply be set by pressing the key. Other outputs require a specific string as listed:

List of outputs:
```
space key = space
escape key = esc
f1 key = f1
f2 key = f2
f3 key = f3
f4 key = f4
f5 key = f5
f6 key = f6
f7 key = f7
f8 key = f8
f9 key = f9
f10 key = f10
f11 key = f11
f12 key = f12
backspace key = backspace
enter key = enter
right shift key = rshift
right control key = rctrl
right alt key = ralt
tab key = tab
caps lock key = capslock
left shift key = lshift
left control key = lctrl
left alt key = lalt
left arrow key = lkey
up arrow key = ukey
down arrow key = dkey
right arrow key = rkey
print screen key = pscreen
delete key = delete
home key = home
end key = end
page up key = pup
page down key = pdown
num lock key = numlicj
numpad / key = n/
numpad * key = n*
numpad - key = n-
numpad + key = n+
numpad enter key = nenter
numpad 0 key = n0
numpad 1 key = n1
numpad 2 key = n2
numpad 3 key = n3
numpad 4 key = n4
numpad 5 key = n5
numpad 6 key = n6
numpad 7 key = n7
numpad 8 key = n8
numpad 9 key = n9
left mouse button = mleft
right mouse button = mright
scroll wheel click = mmiddle
mouse side forward button = mforward
mouse side back button = mback
scroll up = sup
scroll down = sdown
```

## Debugging

View problems with the device driver with the command:
```
dmesg
```

## Notes

The driver will not suppress the initial mouse click
