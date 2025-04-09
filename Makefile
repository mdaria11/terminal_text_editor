CFLAGS = -lncurses

text_editor: text_editor.c include/gap_buffer_list.c
	gcc -o text_editor text_editor.c ./include/gap_buffer_list.c $(CFLAGS)

clean:
	rm text_editor
	rm file.txt
