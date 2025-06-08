#include "gap_buffer_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Debug prints all the information stored in the gap_buffer data structure.*/
void print_buffer_info(struct gap_buffer *buffer)
{
    printf("Buffer total size is: %d\n", buffer->size);
    printf("Buffer gap size is: %d\n", buffer->gap_size);
    printf("Buffer gap range is from %d to %d\n", buffer->gap_left, buffer->gap_right);
    printf("\n");
}

void delete_char_from_buf(struct gap_buffer *buffer)
{
    buffer->gap_left--;
    buffer->gap_size++;
    buffer->char_count--;
}

void move_gap_to_pos(struct gap_buffer *buffer, int pos)
{
    if(buffer->gap_left == pos)
    {
        return;
    }

    if(buffer->gap_left > pos)
    {
        int updated_right_index = buffer->gap_right;

        for(int i = buffer->gap_left; i > pos; i--)
        {
            buffer->buffer[updated_right_index] = buffer->buffer[i - 1];
            updated_right_index--;
        }

        buffer->gap_left = pos;
        buffer->gap_right = updated_right_index;

    } else
    {
        int updated_right_index = buffer->gap_right;

        for(int i = buffer->gap_left; i < pos; i++)
        {
            buffer->buffer[i] = buffer->buffer[updated_right_index + 1];
            updated_right_index++;
        }

        buffer->gap_left = pos;
        buffer->gap_right = updated_right_index;
    }
}

int grow_gap_for_buffer(struct gap_buffer *buffer)
{
    size_t gap_added_size = sizeof(char) * 100;
    size_t new_size;

    // Realloc the buffer 
    buffer->buffer = realloc(buffer->buffer, buffer->size + gap_added_size);
    memset(buffer->buffer + buffer->size, 0, gap_added_size);
    new_size = buffer->size + gap_added_size;

    // the gap was at the end of the buffer, so no need for moving characters
    if(buffer->gap_right == buffer->size - 1)
    {
        goto updates;
    }

    //Move characters to the right
    int new_index = new_size - 1;
    int old_index = buffer->size - 1;

    while(old_index > buffer->gap_right)
    {
        buffer->buffer[new_index] = buffer->buffer[old_index];
        new_index--;
        old_index--;
    }

updates:
    //Update sizes and gap_right
    buffer->size = new_size;
    buffer->gap_size = buffer->gap_size + gap_added_size;
    buffer->gap_right = buffer->gap_right + gap_added_size;

    return 0;
} 

int add_text_to_buffer(char *text, struct gap_buffer *buffer)
{
    // gap size can't hold the text
    if(buffer->gap_size < strlen(text))
    {
        grow_gap_for_buffer(buffer);
    }

    strncpy(buffer->buffer + buffer->gap_left, text, strlen(text));
    buffer->gap_left += strlen(text);
    buffer->gap_size -= strlen(text);
    buffer->char_count += strlen(text);

    return 0;
}

int add_char_to_buffer(char ch, struct gap_buffer *buffer)
{
    // gap size can't hold the character
    if(buffer->gap_size < 1)
    {
        grow_gap_for_buffer(buffer);
    }

    buffer->buffer[buffer->gap_left] = ch;
    buffer->gap_left++;
    buffer->gap_size--;
    buffer->char_count++;

    return 0;
}

int add_char_to_pos_buffer(struct gb_list *list, char ch, int pos_buf)
{
    struct gb_node *flag;
    flag = list->head;

    while(pos_buf > 0)
    {
        flag = flag->next;
        pos_buf--;
    }

    add_char_to_buffer(ch, flag->buffer);
    return 0;
}

/* Write the buffers from list to file. */
int buffers_to_file(struct gb_list *list, FILE *input)
{
    struct gb_node *flag;
    flag = list->head;

    while(flag != NULL)
    {
        int index = 0;

        while(index < flag->buffer->size)
        {
            if(index == flag->buffer->gap_left)
            {
                index = flag->buffer->gap_right + 1;
                continue;
            }

            fprintf(input, "%c", flag->buffer->buffer[index]);
            index++;
        }

        fprintf(input, "\n");

        flag = flag->next;
    }
}

struct gap_buffer* buffer_at_pos(struct gb_list *list, int position)
{
    struct gb_node *flag;
    flag = list->head;

    while(position > 0)
    {
        flag = flag->next;
        position--;
    }

    return flag->buffer;
}

/* Initializes a new gap_buffer node in the list at position.*/
int initialize_new_gbuffer(struct gb_list *list, int position)
{
    if(position < 0 || (position > list->length && list->head != NULL))
    {
        printf("Invalid position %d in initialize_new_gbuffer()\n", position);
        return -1;
    }

    // Initialize gap buffer with size 100 
    struct gap_buffer *newBuf;
    newBuf = malloc(sizeof(struct gap_buffer));
    newBuf->buffer = calloc(100, sizeof(char));
    newBuf->gap_size = 100;
    newBuf->gap_left = 0;
    newBuf->gap_right = 99;
    newBuf->size = 100;
    newBuf->char_count = 0;

    // Add the node in the list
    struct gb_node *newNode;
    newNode = malloc(sizeof(struct gb_node));
    newNode->buffer = newBuf;
    
    if(list->head == NULL)
    {
        newNode->next = NULL;
        list->head = newNode;
        list->length++;

        return 0;
    } else
    {
        int curr_pos = 0;
        struct gb_node *flag;
        flag = list->head;

        while(curr_pos < position - 1)
        {
            flag = flag->next;
            curr_pos++;
        }

        struct gb_node *next_neigh = flag->next;
        flag->next = newNode;
        newNode->next = next_neigh;
        list->length++;

        return 0;
    }
}

/* Initializing list with head NULL and length 0*/
int initialize_list(struct gb_list *list)
{
    list->head = NULL;
    list->length = 0;

    return 0;
}

/* Frees dynamically-allocated structures within the list, excluding the list structure itself.*/
int free_list(struct gb_list *list)
{
    struct gb_node *flag;
    flag = list->head;

    while(flag != NULL)
    {
        struct gb_node * aux;
        aux = flag->next;

        free(flag->buffer->buffer);
        free(flag->buffer);
        free(flag);

        flag = aux;
    }
}
