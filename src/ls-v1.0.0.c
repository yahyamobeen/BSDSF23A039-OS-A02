/*
* Programming Assignment 02: ls v1.1.0
* Complete Long Listing Format
* Usage:
*       $ ./bin/ls 
*       $ ./bin/ls -l
*       $ ./bin/ls -l /home
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <getopt.h>
#include <termios.h>
#include <sys/ioctl.h>

// Display modes
typedef enum {
    DISPLAY_SIMPLE,     // Default column display
    DISPLAY_LONG,       // Long listing format (-l)
    DISPLAY_HORIZONTAL  // Horizontal display (-x)
} display_mode_t;

// Global variables
display_mode_t display_mode = DISPLAY_SIMPLE;
int terminal_width = 80;


// File list structure
typedef struct {
    char **names;
    int count;
    int max_name_len;
} file_list_t;

/**
 * Create a new file list
 */
file_list_t *file_list_create() {
    file_list_t *list = malloc(sizeof(file_list_t));
    list->names = NULL;
    list->count = 0;
    list->max_name_len = 0;
    return list;
}




/**
 * Print files in horizontal columns (across then down)
 */
void print_horizontal_columns(file_list_t *list) {
    if (list->count == 0) {
        return;
    }
    
    int col_width = list->max_name_len + 2; // Name + spacing
    int max_cols = terminal_width / col_width;
    if (max_cols == 0) max_cols = 1;
    
    int current_col = 0;
    int current_width = 0;
    
    for (int i = 0; i < list->count; i++) {
        int name_len = strlen(list->names[i]);
        int needed_width = name_len + 2; // Name plus spacing
        
        // Check if we need to wrap to next line
        if (current_col > 0 && (current_width + needed_width) > terminal_width) {
            printf("\n");
            current_col = 0;
            current_width = 0;
        }
        
        // Print the filename with padding
        printf("%-*s", col_width, list->names[i]);
        
        current_col++;
        current_width += col_width;
    }
    
    // Add final newline if we printed anything
    if (list->count > 0) {
        printf("\n");
    }
}




/**
 * Add filename to list
 */
void file_list_add(file_list_t *list, const char *name) {
    list->names = realloc(list->names, (list->count + 1) * sizeof(char *));
    list->names[list->count] = strdup(name);
    
    int len = strlen(name);
    if (len > list->max_name_len) {
        list->max_name_len = len;
    }
    
    list->count++;
}

/**
 * Free file list memory
 */
void file_list_free(file_list_t *list) {
    for (int i = 0; i < list->count; i++) {
        free(list->names[i]);
    }
    free(list->names);
    free(list);
}






// Global variables for display
int terminal_width = 80;

/**
 * Get terminal width using termios (alternative to ioctl)
 */
int get_terminal_width() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_col;
    }
    return 80; // Fallback to 80 columns
}
extern int errno;

// Global flag for long format
int long_format = 0;

// Function prototypes
void do_ls(const char *dir);
void get_permissions(mode_t mode, char *str);
void print_long_format(const char *dirname, const char *filename);



/**
 * Calculate column layout
 */
void calculate_column_layout(file_list_t *list, int *cols, int *rows) {
    if (list->count == 0) {
        *cols = *rows = 0;
        return;
    }
    
    int col_width = list->max_name_len + 2; // Name + spacing
    
    // Calculate maximum possible columns
    *cols = terminal_width / col_width;
    if (*cols == 0) *cols = 1; // At least 1 column
    
    // Calculate rows needed
    *rows = (list->count + *cols - 1) / *cols; // Ceiling division
}

/**
 * Print files in vertical columns (down then across)
 */
void print_vertical_columns(file_list_t *list) {
    int cols, rows;
    calculate_column_layout(list, &cols, &rows);
    
    int col_width = list->max_name_len + 2;
    
    // Print row by row, column by column
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int index = r + c * rows;
            if (index < list->count) {
                printf("%-*s", col_width, list->names[index]);
            }
        }
        printf("\n");
    }
}




/**
 * Convert file mode to permission string (e.g., "-rwxr-xr-x")
 */
void get_permissions(mode_t mode, char *str) {
    // File type
    if (S_ISDIR(mode))       str[0] = 'd';
    else if (S_ISLNK(mode))  str[0] = 'l';
    else if (S_ISCHR(mode))  str[0] = 'c';
    else if (S_ISBLK(mode))  str[0] = 'b';
    else if (S_ISFIFO(mode)) str[0] = 'p';
    else if (S_ISSOCK(mode)) str[0] = 's';
    else                     str[0] = '-';
    
    // Owner permissions
    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    str[3] = (mode & S_IXUSR) ? 'x' : '-';
    
    // Group permissions
    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    str[6] = (mode & S_IXGRP) ? 'x' : '-';
    
    // Others permissions
    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    str[9] = (mode & S_IXOTH) ? 'x' : '-';
    
    str[10] = '\0';
}

/**
 * Print file information in long listing format
 */
void print_long_format(const char *dirname, const char *filename) {
    struct stat file_stat;
    char path[1024];
    char perms[11];
    char time_str[80];
    
    // Build full path
    snprintf(path, sizeof(path), "%s/%s", dirname, filename);
    
    // Get file stats using lstat (doesn't follow symlinks)
    if (lstat(path, &file_stat) == -1) {
        perror("lstat");
        return;
    }
    
    // Get permission string
    get_permissions(file_stat.st_mode, perms);
    
    // Get username and group name
    struct passwd *pwd = getpwuid(file_stat.st_uid);
    struct group *grp = getgrgid(file_stat.st_gid);
    
    // Format time (last modification time)
    struct tm *tm_info = localtime(&file_stat.st_mtime);
    strftime(time_str, sizeof(time_str), "%b %d %H:%M", tm_info);
    
    // Print in ls -l format with proper alignment
    printf("%s %3ld %-8s %-8s %8ld %s %s\n",
           perms,
           (long)file_stat.st_nlink,
           pwd ? pwd->pw_name : "unknown",
           grp ? grp->gr_name : "unknown",
           (long)file_stat.st_size,
           time_str,
           filename);
}




/**
 * List directory contents
 */
void do_ls(const char *dir) {
    struct dirent *entry;
    DIR *dp = opendir(dir);
    
    if (dp == NULL) {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return;
    }
    
    file_list_t *file_list = file_list_create();
    errno = 0;
    
    // Read all directory entries
    while ((entry = readdir(dp)) != NULL) {
        // Skip hidden files
        if (entry->d_name[0] == '.')
            continue;
        
        if (display_mode == DISPLAY_LONG) {
            // Print in long format immediately
            print_long_format(dir, entry->d_name);
        } else {
            // Add to list for column display
            file_list_add(file_list, entry->d_name);
        }
    }
    
    // Handle different display modes for non-long format
    if (display_mode != DISPLAY_LONG && file_list->count > 0) {
        switch (display_mode) {
            case DISPLAY_SIMPLE:
                print_vertical_columns(file_list);
                break;
            case DISPLAY_HORIZONTAL:
                print_horizontal_columns(file_list);
                break;
            default:
                print_vertical_columns(file_list); // Fallback
                break;
        }
    }
    
    file_list_free(file_list);
    
    if (errno != 0) {
        perror("readdir failed");
    }
    
    closedir(dp);
}






/**
 * Main function
 */
int main(int argc, char *argv[]) {
    int opt;
    
    // Initialize terminal width
    terminal_width = get_terminal_width();
    

    // Parse command-line options
    while ((opt = getopt(argc, argv, "lx")) != -1) {
        switch (opt) {
            case 'l':
                display_mode = DISPLAY_LONG;
                break;
            case 'x':
                display_mode = DISPLAY_HORIZONTAL;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-x] [directory...]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    
    // Note: -l takes precedence over -x if both are specified
    // This matches standard ls behavior
    



    
    // Rest of main function remains the same...
    // Process directories
    if (optind == argc) {
        // No directory specified, use current directory
        do_ls(".");
    } else {
        // Process each specified directory
        for (int i = optind; i < argc; i++) {
            if (argc - optind > 1) {
                printf("%s:\n", argv[i]);
            }
            do_ls(argv[i]);
            if (i < argc - 1) {
                printf("\n");
            }
        }
    }
    
    return 0;
}
