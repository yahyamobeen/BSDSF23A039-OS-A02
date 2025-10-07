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

