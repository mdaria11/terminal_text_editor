#include <stdio.h>

struct gap_buffer
{
    char *buffer;     // buffer holding the text
    int gap_size;     // gap size
    int gap_left;     // left index of the gap (first empty space in the gap)
    int gap_right;    // right index of the gap (last empty space in the gap)
    int size;         // total size of the buffer including the gap
    int char_count;   // character count in the buffer that are visible in terminal
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
int add_char_to_pos_buffer(struct gb_list *list, char ch, int pos_buf); /* Add one character to the buffer at position pos in list. */
int free_list(struct gb_list *list); /* Free resources used in list except the list structure itself. */
int buffers_to_file(struct gb_list *list, FILE *input); /* Writes all characters from the list in the file. */
struct gap_buffer* buffer_at_pos(struct gb_list *list, int position); /* Returns the buffer at position pos from list. */
