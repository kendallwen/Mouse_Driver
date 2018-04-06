all: event_writer.c event_reader.c keyboard_reader.c
	gcc -o eWriter event_writer.c
	gcc -o eReader event_reader.c
	gcc -o kReader keyboard_reader.c
	gcc -o mReader mouse_reader.c

clean:
	rm -f *.o eWriter eReader kReader mReader
