# OS Programming Assignment 02 - Report
## Student: BSDSF23A039

---

## Feature-2: Long Listing Format (v1.1.0)

### Q1: What is the crucial difference between stat() and lstat() system calls?

**stat()** follows symbolic links and returns information about the file that the link **points to**.

**lstat()** does **NOT** follow symbolic links and returns information about the **link itself**.

#### In the context of ls command:

**lstat() is more appropriate** because:

1. **Preserves link information**: When listing a symbolic link, we want to show that it IS a link, not hide this fact by showing the target file's properties.

2. **Shows link properties**: With lstat(), we can display the link's own:
   - Permissions (usually lrwxrwxrwx)
   - Size (size of the link path itself)
   - Timestamp (when the link was created)

3. **Security**: Prevents following potentially dangerous links:
   - Broken links (targets that don't exist)
   - Links to unauthorized locations
   - Circular link references

4. **Standard behavior**: The real `ls` command uses lstat() to properly display:
lrwxrwxrwx 1 user group 11 Jan 15 10:30 mylink -> /etc/passwd
#### Example:

Consider a symbolic link: `mylink -> /etc/passwd`

**Using stat(mylink):**
- Returns information about `/etc/passwd`
- Shows as regular file: `-rw-r--r--`
- Shows passwd file's size, owner, etc.
- **Hides** that it's actually a link

**Using lstat(mylink):**
- Returns information about `mylink` itself
- Shows as symbolic link: `lrwxrwxrwx`
- Shows link's own properties
- **Reveals** that it's a symbolic link

---

### Q2: Extracting file type and permissions from st_mode field

The `st_mode` field in `struct stat` is a 16-bit integer that encodes:
- **File type** (high-order bits)
- **Permission bits** (low-order 9 bits)
- **Special bits** (setuid, setgid, sticky bit)

#### Structure of st_mode:
Bits:  15 14 13 12 | 11 10 9 | 8 7 6 | 5 4 3 | 2 1 0
File Type   | Special | User  | Group | Other
---

#### Extracting File Type:

Use predefined macros that check specific bit patterns:
```c
if (S_ISREG(st_mode))   // Regular file
if (S_ISDIR(st_mode))   // Directory
if (S_ISLNK(st_mode))   // Symbolic link
if (S_ISCHR(st_mode))   // Character device
if (S_ISBLK(st_mode))   // Block device
if (S_ISFIFO(st_mode))  // Named pipe (FIFO)
if (S_ISSOCK(st_mode))  // Socket

How these macros work internally:

## Feature-3: Column Display (v1.2.0)

### Q1: Explain the general logic for printing items in "down then across" format

The "down then across" (vertical) column format requires a two-dimensional printing approach:

**Logic:**
1. **Read All Items First**: We must know all filenames and the longest filename length before printing
2. **Calculate Layout**: 
   - Get terminal width
   - Calculate column width: `max_filename_length + spacing`
   - Calculate columns: `terminal_width / column_width`
   - Calculate rows: `ceil(total_files / columns)`
3. **Print by Rows**: 
   - Iterate through rows (0 to rows-1)
   - For each row, iterate through columns (0 to columns-1)
   - Calculate index: `index = current_row + current_column * total_rows`
   - Print item at calculated index with proper padding

**Why simple loop doesn't work:**
A simple loop would print:
file1 file2 file3
file4 file5 file6

But we need:

file1 file4 file7
file2 file5 file8
file3 file6 file9


### Q2: Purpose of terminal width detection and limitations of fixed width

**Purpose:**
- **Dynamic Adaptation**: Adjust column layout based on actual terminal size
- **Optimal Space Usage**: Maximize information display without wrapping
- **User Experience**: Provide clean, readable output regardless of window size

**Limitations of Fixed Width (80 columns):**
1. **Wasted Space**: On wide terminals, only uses partial screen width
2. **Poor Readability**: On narrow terminals, columns may get truncated
3. **Inflexible**: Doesn't adapt to different display environments
4. **Modern Incompatibility**: Many terminals are much wider than 80 columns today

**Our Implementation:**
We use `ioctl(TIOCGWINSZ)` to get terminal dimensions, falling back to 80 columns if unavailable.

## Feature-4: Horizontal Column Display (v1.3.0)

### Q1: Compare implementation complexity of vertical vs horizontal printing

**Vertical (Down Then Across) Printing:**
- **More Complex**: Requires significant pre-calculation
- **Two-pass process**: First read all files, then calculate layout, then print
- **Index calculation**: Need complex index math: `index = row + column * total_rows`
- **Memory intensive**: Must store all filenames in memory
- **Layout dependency**: Entire layout depends on longest filename and terminal width

**Horizontal (Across Then Down) Printing:**
- **Simpler**: Single-pass, stream-oriented approach
- **Real-time decisions**: Can print as we go, wrapping when needed
- **Straightforward logic**: Simple counter for current position
- **Less memory**: Could theoretically print without storing all names
- **Dynamic wrapping**: Natural text wrapping behavior

**Why Vertical Needs More Pre-calculation:**
Vertical layout requires knowing ALL filenames and the LONGEST filename before starting to print, because the entire column structure depends on these values.

### Q2: Strategy for managing different display modes

**Our Implementation Strategy:**

1. **Enum for Display Modes:**
   ```c
   typedef enum {
       DISPLAY_SIMPLE,     // Default vertical
       DISPLAY_LONG,       // -l option  
       DISPLAY_HORIZONTAL  // -x option
   } display_mode_t;


Global State Variable:
c

display_mode_t display_mode = DISPLAY_SIMPLE;

Option Parsing Logic:

    -l sets DISPLAY_LONG

    -x sets DISPLAY_HORIZONTAL

    Default remains DISPLAY_SIMPLE

    -l takes precedence (matches standard ls)

Branching in do_ls():
c

if (display_mode == DISPLAY_LONG) {
    // Print immediately in long format
} else {
    // Collect files, then choose display function
    switch (display_mode) {
        case DISPLAY_SIMPLE: print_vertical_columns(); break;
        case DISPLAY_HORIZONTAL: print_horizontal_columns(); break;
    }
}

Decision Flow:

    Parse command-line options → Set display_mode

    Read directory entries

    If LONG mode → Print each file immediately with metadata

    Otherwise → Collect all names, then call appropriate column function

text



## Feature-5: Alphabetical Sort (v1.4.0)

### Q1: Why read all directory entries into memory before sorting?

**Necessity of Reading All Entries First:**

1. **Sorting Requirement**: Sorting algorithms need access to all elements to determine their relative order
2. **Directory Stream Limitation**: `readdir()` provides entries in filesystem order (not alphabetical)
3. **Display Consistency**: All display modes (vertical, horizontal, long) need the same sorted order
4. **Performance**: Single pass through directory, then efficient sorting

**Potential Drawbacks for Large Directories:**

1. **Memory Usage**: Storing millions of filenames can consume significant RAM
2. **Startup Delay**: Users must wait for entire directory to be read before seeing any output
3. **Scalability Issues**: Very large directories might exceed available memory

**Mitigation Strategies:**
- **Lazy Loading**: Could implement pagination for huge directories
- **External Sorting**: For extremely large directories, use disk-based sorting
- **Progress Indication**: Show progress while reading large directories

### Q2: qsort() Comparison Function

**Purpose:**
The comparison function tells `qsort()` how to compare two elements in the array to determine their sort order.

**Function Signature:**
```c
int compare_strings(const void *a, const void *b);

How It Works:

    *void Parameters**: qsort() passes generic pointers since it doesn't know the data type

    Type Casting: We cast to const char** because our array contains char* pointers

    String Comparison: Use strcmp() to compare the actual strings

    Return Values:

        Negative: a comes before b

        Zero: a equals b

        Positive: a comes after b

Why const void Arguments:*

    Generic Design: qsort() works with any data type (ints, structs, strings, etc.)

    Type Safety: Forces explicit casting, making type handling deliberate

    Standard Compliance: Follows C library standards for callback functions

Example Implementation:
c

int compare_strings(const void *a, const void *b) {
    const char *str1 = *(const char **)a;  // Cast and dereference
    const char *str2 = *(const char **)b;  // Cast and dereference  
    return strcmp(str1, str2);             // Compare strings
}
