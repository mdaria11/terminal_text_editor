#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h> 
#include <errno.h> 
#include <sys/wait.h>

#include "./include/gap_buffer_list.h"

FILE *input; // edited file
struct gb_list file_text;

static void finish(int sig)
{
    endwin();
    buffers_to_file(&file_text, input);
    free_list(&file_text);
    fclose(input);
    exit(0);
}

void print_usage()
{
    printf("Usage: ./text_editor [file_name]\n");
    printf("Without file_name, a new file will be created.\n");
}

void initialize_terminal()
{
    //Curses Terminal Initialisations
    signal(SIGINT, finish);      /* arrange interrupts to terminate */
    initscr();      /* initialize the curses library */
    start_color();  /* have colors */
    keypad(stdscr, TRUE);  /* enable keyboard mapping for special characters*/
    cbreak();       /* take input chars one at a time, no wait for \n */
    noecho();         /* no printing in terminal by itself */

    // font colors
    init_pair(1, COLOR_BLACK, COLOR_CYAN); // filename/title
    init_pair(2, COLOR_WHITE, COLOR_BLACK); // default

    // take filename from the FILE structure
    char proclnk[0xFFF];
    char filename[0xFFF];
    int fno = fileno(input);
    sprintf(proclnk, "/proc/self/fd/%d", fno);

    ssize_t r = readlink(proclnk, filename, 0xFFF);
    if (r < 0)
    {
        printf("failed to readlink\n");
        exit(1);    
    }
    filename[r] = '\0';
    // show filename on terminal
    attron(COLOR_PAIR(1));
    addstr(filename);
    addstr("\n\n");
    refresh();
}

void move_cursor_terminal(int offset_x, int offset_y)
{
    int x, y;
    getyx(stdscr, y, x);  // Get current cursor position

    // Can't move more than the length of the line.
    if(x + offset_x > buffer_at_pos(&file_text, y - 2)->char_count)
    {
        return;
    }

    // Text lines start at line 2 in terminal space.
    if(y + offset_y < 2)
    {
        return;
    }

    // Moving down
    if(offset_y == 1)
    {
        if(file_text.length > y - 1) //line exists
        {
            if(x > buffer_at_pos(&file_text, y - 1)->char_count)
            {
                move(y + offset_y, buffer_at_pos(&file_text, y - 1)->char_count);  // Move to next line
            } else
            {
                move(y + offset_y, x);  // Move to next line at the same index
                //TODO: update gap in the new buffer
            }
        }
        
        return;

    } else if(offset_y == -1) //Moving up
    {
        if(x > buffer_at_pos(&file_text, y - 3)->char_count)
        {
            move(y + offset_y, buffer_at_pos(&file_text, y - 3)->char_count);  // Move to previous line
        }else
        {
            move(y + offset_y, x);  // Move to previous line at the same index
            //TODO: update gap in the new buffer
        }
        return;
    }

    // Move left/right on the line
    //Move the gap
    move_gap_to_pos(buffer_at_pos(&file_text, y - 2), x + offset_x);
    move(y, x + offset_x);  // Move to next position
    refresh();
}

void start_terminal()
{
    initialize_terminal();
    attron(COLOR_PAIR(2));

    for (;;)
    {
        int c = getch();     /* refresh, accept single keystroke of input */
        if(c >= 0402 && c <= 0405) // cursor movement key
        {
            switch (c) {
                case KEY_DOWN:
                    move_cursor_terminal(0, 1);
                    break;
                case KEY_UP:
                    move_cursor_terminal(0, -1);
                    break;
                case KEY_LEFT:
                    move_cursor_terminal(-1, 0);
                    break;
                case KEY_RIGHT:
                    move_cursor_terminal(1, 0);
                    break;
            }
        } else if(c == KEY_BACKSPACE)
        {
            int x, y;
            getyx(stdscr, y, x);
            
            //delete character at cursor position (aka at gap )
            struct gap_buffer *current_line = buffer_at_pos(&file_text, y-2);
            delete_char_from_buf(current_line);

            //update terminal
            move_cursor_terminal(-1, 0);
            delch(); //delete character
            refresh();
        } else if (c == '\n')
        {
            int x, y;
            getyx(stdscr, y, x);
            initialize_new_gbuffer(&file_text, y - 1);
            addch(c);
            refresh();
        } else
        {
            int x, y;
            getyx(stdscr, y, x); // y-line x-column (text starts at line 2 - offset from file name)

            //update terminal screen
            if(x < buffer_at_pos(&file_text, y-2)->char_count) //the cursor is inserting
            {
                int saved_x = x;
                int saved_y = y;
                //move characters one place to the right
                for(int i = buffer_at_pos(&file_text, y-2)->char_count - 1; i >= x; i--)
                {
                    char c_aux = mvwinch(stdscr, y, i) & A_CHARTEXT;
                    move(y, i + 1);
                    addch(c_aux);
                }
                move(saved_y, saved_x); //reset the cursor back to its place
            }

            addch(c);
            refresh();

            //add the character in the text structure
            add_char_to_pos_buffer(&file_text, (char)c, y - 2);
        }
    }
}


int main(int argc, char** argv)
{
    if(argc > 2)
    {
        print_usage();
        return 1;
    } else if(argc == 1) // create new file
    {
        pid_t child_pid;

        child_pid = fork();

        if(child_pid == 0) // child process
        {
            char * argv_list[] = {"touch","file.txt",NULL};
            execv("/bin/touch",argv_list);
            exit(0);

        } else // parent process
        {
            int status;

            if (waitpid(child_pid, &status, 0) > 0) // wait for file to be created
            {
                if (WIFEXITED(status) && WEXITSTATUS(status))
                {
                    if (WEXITSTATUS(status) == 127) {
     
                        printf("Creating a file failed\n");
                        exit(1);
                    }
                }
            }

            input = fopen("file.txt", "w"); // open the file
        }

    } else // existing file
    {

    }

    /*Initialize the text list and add a gap buffer for line 0.*/
    initialize_list(&file_text);
    initialize_new_gbuffer(&file_text, 0);

    start_terminal();
}