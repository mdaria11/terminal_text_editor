#include <stdio.h>

struct gap_buffer
{
    char *buffer;
    int gap_size; 
    int gap_left; 
    int gap_right; 
    int size; 
    /*TODO: add letter_count variable keeping the letters in the line aka the ones visible in terminal screen*/
};

struct gb_node
{
    struct gap_buffer *buffer;
    struct gb_node *next;
};

struct gb_list
{
    struct gb_node *head;
    int length;
};

int initialize_list(struct gb_list *list); /* Initializing an empty list. */
int initialize_new_gbuffer(struct gb_list *list, int position); /* Inializing a gap buffer in the list at position.*/
void print_buffer_info(struct gap_buffer *buffer); /* Prints info relating the buffer*/
int add_char_to_pos_buffer(struct gb_list *list, char ch, int pos_buf);
int free_list(struct gb_list *list);
int buffers_to_file(struct gb_list *list, FILE *input);
