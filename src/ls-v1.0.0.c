/*
* Programming Assignment 02: ls v1.5.0
* Complete Long Listing Format with Column Display and Colors
* Usage:
*       $ ./bin/ls 
*       $ ./bin/ls -l
*       $ ./bin/ls -x
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
#include <sys/ioctl.h>

extern int errno;

// ANSI color codes for file types
#define COLOR_RESET   "\033[0m"
#define COLOR_BLUE    "\033[1;34m"    // Directories
#define COLOR_GREEN   "\033[1;32m"    // Executables  
#define COLOR_RED     "\033[1;31m"    // Archives
#define COLOR_MAGENTA "\033[1;35m"    // Symbolic links
#define COLOR_CYAN    "\033[1;36m"    // Special files
#define COLOR_YELLOW  "\033[1;33m"    // Regular files (default)

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

// Function prototypes
void do_ls(const char *dir);
void get_permissions(mode_t mode, char *str);
void print_long_format(const char *dirname, const char *filename);
file_list_t *file_list_create();
void file_list_add(file_list_t *list, const char *name);
void file_list_free(file_list_t *list);
int get_terminal_width();
void calculate_column_layout(file_list_t *list, int *cols, int *rows);
void print_vertical_columns(file_list_t *list, const char *dir);
void print_horizontal_columns(file_list_t *list, const char *dir);
int compare_strings(const void *a, const void *b);
const char *get_file_color(const char *filename, mode_t mode);



/**
 * Recursive directory listing function
 */
void do_ls_recursive(const char *dir) {
    struct dirent *entry;
    DIR *dp = opendir(dir);
    
    if (dp == NULL) {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return;
    }
    
    // Print directory header (standard ls -R behavior)
    printf("\n%s:\n", dir);
    
    file_list_t *file_list = file_list_create();
    file_list_t *dir_list = file_list_create();  // Separate list for subdirectories
    errno = 0;
    
    // Read all directory entries
    while ((entry = readdir(dp)) != NULL) {
        // Skip hidden files and . / .. entries for recursion
        if (entry->d_name[0] == '.')
            continue;
        
        // Add to main file list for display
        file_list_add(file_list, entry->d_name);
        
        // Check if it's a directory for recursion
        if (recursive_flag) {
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", dir, entry->d_name);
            
            struct stat st;
            if (lstat(full_path, &st) == 0 && S_ISDIR(st.st_mode)) {
                file_list_add(dir_list, entry->d_name);
            }
        }
    }
    
    // Sort the file list alphabetically
    if (file_list->count > 0) {
        qsort(file_list->names, file_list->count, sizeof(char *), compare_strings);
    }
    
    // Handle different display modes
    if (display_mode == DISPLAY_LONG) {
        // Print each file in long format from sorted list
        for (int i = 0; i < file_list->count; i++) {
            print_long_format(dir, file_list->names[i]);
        }
    } else if (file_list->count > 0) {
        // Use column display for non-long formats
        switch (display_mode) {
            case DISPLAY_SIMPLE:
                print_vertical_columns(file_list, dir);
                break;
            case DISPLAY_HORIZONTAL:
                print_horizontal_columns(file_list, dir);
                break;
            default:
                print_vertical_columns(file_list, dir); // Fallback
                break;
        }
    }
    
    // Recursively process subdirectories
    if (recursive_flag && dir_list->count > 0) {
        // Sort directory list
        qsort(dir_list->names, dir_list->count, sizeof(char *), compare_strings);
        
        for (int i = 0; i < dir_list->count; i++) {
            char subdir_path[1024];
            snprintf(subdir_path, sizeof(subdir_path), "%s/%s", dir, dir_list->names[i]);
            do_ls_recursive(subdir_path);
        }
    }
    
    file_list_free(file_list);
    file_list_free(dir_list);
    
    if (errno != 0) {
        perror("readdir failed");
    }
    
    closedir(dp);
}



/**
 * Get terminal width using termios
 */
int get_terminal_width() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_col;
    }
    
    // Fallback: environment variable
    char *columns = getenv("COLUMNS");
    if (columns != NULL) {
        int width = atoi(columns);
        if (width > 0) return width;
    }
    
    // Final fallback
    return 80;
}

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
 * Add filename to list with error checking
 */
void file_list_add(file_list_t *list, const char *name) {
    // Reallocate memory for names array
    char **new_names = realloc(list->names, (list->count + 1) * sizeof(char *));
    if (new_names == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    list->names = new_names;
    
    // Duplicate the string
    list->names[list->count] = strdup(name);
    if (list->names[list->count] == NULL) {
        fprintf(stderr, "Memory allocation failed for filename\n");
        return;
    }
    
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

/**
 * Comparison function for qsort (alphabetical order)
 */
int compare_strings(const void *a, const void *b) {
    const char *str1 = *(const char **)a;
    const char *str2 = *(const char **)b;
    return strcmp(str1, str2);
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
 * Determine color based on file type and permissions
 */
const char *get_file_color(const char *filename, mode_t mode) {
    // Check if it's a directory
    if (S_ISDIR(mode)) {
        return COLOR_BLUE;
    }
    
    // Check if it's a symbolic link
    if (S_ISLNK(mode)) {
        return COLOR_MAGENTA;
    }
    
    // Check if it's a character/block device, FIFO, or socket
    if (S_ISCHR(mode) || S_ISBLK(mode) || S_ISFIFO(mode) || S_ISSOCK(mode)) {
        return COLOR_CYAN;
    }
    
    // Check if it's executable (any execute permission bit set)
    if (mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
        return COLOR_GREEN;
    }
    
    // Check for archive files by extension
    const char *ext = strrchr(filename, '.');
    if (ext != NULL) {
        if (strcmp(ext, ".tar") == 0 || strcmp(ext, ".gz") == 0 || 
            strcmp(ext, ".zip") == 0 || strcmp(ext, ".bz2") == 0 ||
            strcmp(ext, ".xz") == 0 || strcmp(ext, ".tgz") == 0 ||
            strcmp(ext, ".deb") == 0 || strcmp(ext, ".rpm") == 0) {
            return COLOR_RED;
        }
    }
    
    // Default color for regular files
    return COLOR_YELLOW;
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
 * Print files in vertical columns (down then across) with colors
 */
void print_vertical_columns(file_list_t *list, const char *dir) {
    int cols, rows;
    calculate_column_layout(list, &cols, &rows);
    
    int col_width = list->max_name_len + 2;
    
    // Print row by row, column by column
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int index = r + c * rows;
            if (index < list->count) {
                // Get file info for color
                struct stat st;
                char path[1024];
                snprintf(path, sizeof(path), "%s/%s", dir, list->names[index]);
                
                if (lstat(path, &st) == 0) {
                    const char *color = get_file_color(list->names[index], st.st_mode);
                    printf("%s%-*s%s", color, col_width, list->names[index], COLOR_RESET);
                } else {
                    printf("%-*s", col_width, list->names[index]);
                }
            }
        }
        printf("\n");
    }
}

/**
 * Print files in horizontal columns (across then down) with colors
 */
void print_horizontal_columns(file_list_t *list, const char *dir) {
    if (list->count == 0) {
        return;
    }
    
    int col_width = list->max_name_len + 2;
    int max_cols = terminal_width / col_width;
    if (max_cols == 0) max_cols = 1;
    
    int current_col = 0;
    int current_width = 0;
    
    for (int i = 0; i < list->count; i++) {
        // Get file info for color
        struct stat st;
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir, list->names[i]);
        
        const char *color = COLOR_YELLOW; // Default
        const char *reset = COLOR_RESET;
        
        if (lstat(path, &st) == 0) {
            color = get_file_color(list->names[i], st.st_mode);
        }
        
        int name_len = strlen(list->names[i]);
        int needed_width = name_len + 2;
        
        // Check if we need to wrap to next line
        if (current_col > 0 && (current_width + needed_width) > terminal_width) {
            printf("\n");
            current_col = 0;
            current_width = 0;
        }
        
        // Print the filename with color and padding
        printf("%s%-*s%s", color, col_width, list->names[i], reset);
        
        current_col++;
        current_width += col_width;
    }
    
    // Add final newline if we printed anything
    if (list->count > 0) {
        printf("\n");
    }
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
        
        // Always add to list (we'll handle display mode later)
        file_list_add(file_list, entry->d_name);
    }
    
    // Sort the file list alphabetically
    if (file_list->count > 0) {
        qsort(file_list->names, file_list->count, sizeof(char *), compare_strings);
    }
    
    // Handle different display modes
    if (display_mode == DISPLAY_LONG) {
        // Print each file in long format from sorted list
        for (int i = 0; i < file_list->count; i++) {
            print_long_format(dir, file_list->names[i]);
        }
    } else if (file_list->count > 0) {
        // Use column display for non-long formats
        switch (display_mode) {
            case DISPLAY_SIMPLE:
                print_vertical_columns(file_list, dir);
                break;
            case DISPLAY_HORIZONTAL:
                print_horizontal_columns(file_list, dir);
                break;
            default:
                print_vertical_columns(file_list, dir); // Fallback
                break;
       }
    }
    
    file_list_free(file_list);
    
    if (errno != 0) {
        perror("readdir failed");
    }
    
    closedir(dp);
}




// Update the display_mode enum
typedef enum {
    DISPLAY_SIMPLE,     // Default column display
    DISPLAY_LONG,       // Long listing format (-l)
    DISPLAY_HORIZONTAL  // Horizontal display (-x)
} display_mode_t;

// Global variables
display_mode_t display_mode = DISPLAY_SIMPLE;
int terminal_width = 80;
int recursive_flag = 0;  // Add recursive flag

// Update main function option parsing

/**
 * Main function
 */
int main(int argc, char *argv[]) {
    int opt;
    
    // Initialize terminal width
    terminal_width = get_terminal_width();
    
    // Parse command-line options
    while ((opt = getopt(argc, argv, "lxR")) != -1) {
        switch (opt) {
            case 'l':
                display_mode = DISPLAY_LONG;
                break;
            case 'x':
                display_mode = DISPLAY_HORIZONTAL;
                break;
            case 'R':
                recursive_flag = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-x] [-R] [directory...]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    
    // Process directories
    if (optind == argc) {
        // No directory specified, use current directory
        if (recursive_flag) {
            do_ls_recursive(".");
        } else {
            do_ls(".");
        }
    } else {
        // Process each specified directory
        for (int i = optind; i < argc; i++) {
            if (recursive_flag) {
                do_ls_recursive(argv[i]);
            } else {
                if (argc - optind > 1) {
                    printf("%s:\n", argv[i]);
                }
                do_ls(argv[i]);
                if (i < argc - 1) {
                    printf("\n");
                }
            }
        }
    }
    
    return 0;
}






/*
main function 



int main(int argc, char *argv[]) {
    int opt;
    
    // Initialize terminal width
    terminal_width = get_terminal_width();
    
    // Parse command-line options - ADD 'R' to getopt string
    while ((opt = getopt(argc, argv, "lxR")) != -1) {
        switch (opt) {
            case 'l':
                display_mode = DISPLAY_LONG;
                break;
            case 'x':
                display_mode = DISPLAY_HORIZONTAL;
                break;
            case 'R':
                recursive_flag = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-x] [-R] [directory...]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    
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
*/
