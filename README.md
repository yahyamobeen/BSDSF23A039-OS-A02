# README.md Markdown Code

```markdown
# BSDSF23A039-OS-A02
**Operating Systems Programming Assignment 02 - Custom ls Utility Implementation**

## 📋 Project Overview

This project implements a custom `ls` command from scratch in C, replicating the core functionality of the standard Unix/Linux `ls` utility. The implementation includes advanced features like colorized output, recursive directory listing, and multiple display formats.

## 🚀 Features Implemented

### ✅ Feature 1: Project Setup and Initial Build (v1.0.0)
- **GitHub Repository Setup** with proper naming convention
- **Project Structure** with `src/`, `bin/`, `obj/`, `man/` directories
- **Makefile** for automated compilation
- **Initial ls Implementation** with basic directory listing

### ✅ Feature 2: Complete Long Listing Format (v1.1.0)
- **`-l` option** for detailed file information
- **File Metadata** using `stat()`/`lstat()` system calls
- **User/Group Resolution** with `getpwuid()` and `getgrgid()`
- **Permission Formatting** in `rwxrwxrwx` format
- **Time Formatting** using `ctime()` and custom formatting

### ✅ Feature 3: Column Display - Down Then Across (v1.2.0)
- **Dynamic Column Layout** based on terminal width
- **Vertical Column Format** (down then across)
- **Terminal Width Detection** using `ioctl()` system call
- **Proper Alignment** with filename length calculation

### ✅ Feature 4: Horizontal Column Display (v1.3.0)
- **`-x` option** for horizontal (across then down) display
- **Row-Major Formatting** with automatic line wrapping
- **Display Mode Management** supporting multiple output formats
- **Option Precedence** handling (`-l` takes priority over `-x`)

### ✅ Feature 5: Alphabetical Sort (v1.4.0)
- **Automatic Sorting** of all directory entries
- **qsort Integration** with custom comparison function
- **Case-Sensitive Sorting** matching standard ls behavior
- **Universal Application** across all display modes

### ✅ Feature 6: Colorized Output (v1.5.0)
- **File Type Colors** using ANSI escape codes
- **Color Scheme**:
  - 🔵 **Blue**: Directories
  - 🟢 **Green**: Executable files
  - 🔴 **Red**: Archive files (.tar, .gz, .zip, etc.)
  - 🟣 **Magenta**: Symbolic links
  - 🔵 **Cyan**: Special files (devices, sockets)
  - 🟡 **Yellow**: Regular files
- **Permission-Based Detection** for executable files
- **Extension Recognition** for archive files

### ✅ Feature 7: Recursive Listing (v1.6.0)
- **`-R` option** for recursive directory traversal
- **Directory Headers** marking each directory level
- **Full Path Construction** for accurate recursion
- **Base Case Handling** preventing infinite loops
- **Integration** with all display modes and colors

## 🛠️ Build Instructions

### Prerequisites
- GCC compiler
- GNU Make
- Linux/Unix environment

### Compilation
```bash
# Clone the repository
git clone https://github.com/YOUR_USERNAME/BSDSF23A039-OS-A02.git
cd BSDSF23A039-OS-A02

# Build the project
make

# Clean build artifacts
make clean
```

### Installation
```bash
# Install to system (optional)
make install
```

## 📖 Usage Examples

### Basic Usage
```bash
./bin/ls                    # Default column display
./bin/ls /path/to/directory # List specific directory
./bin/ls dir1 dir2 dir3     # List multiple directories
```

### Display Options
```bash
./bin/ls -l                 # Long listing format
./bin/ls -x                 # Horizontal columns
./bin/ls -l -x              # Long format (x ignored)
```

### Advanced Features
```bash
./bin/ls -R                 # Recursive listing
./bin/ls -lR                # Recursive long format
./bin/ls -xR                # Recursive horizontal
./bin/ls -R /etc            # Recursive on system directory
```

### Color Examples
```bash
./bin/ls /bin               # Executables in green
./bin/ls /tmp               # Directories in blue
./bin/ls ~/Downloads        # Archives in red
```

## 🏗️ Project Structure

```
BSDSF23A039-OS-A02/
├── src/
│   └── ls-v1.0.0.c         # Main source code
├── bin/
│   └── ls                  # Compiled binary
├── obj/
│   └── ls-v1.0.0.o         # Object files
├── man/                    # Manual page (bonus)
├── Makefile                # Build configuration
├── README.md               # This file
└── REPORT.md               # Technical report and answers
```

## 🔧 Technical Implementation

### System Calls Used
- `stat()`, `lstat()` - File metadata
- `readdir()` - Directory reading
- `getpwuid()`, `getgrgid()` - User/group resolution
- `ioctl()` - Terminal width detection
- `qsort()` - Array sorting

### Key Data Structures
```c
typedef struct {
    char **names;           // Array of filenames
    int count;              // Number of files
    int max_name_len;       // Longest filename
} file_list_t;

typedef enum {
    DISPLAY_SIMPLE,         // Vertical columns
    DISPLAY_LONG,           // Long format
    DISPLAY_HORIZONTAL      // Horizontal columns
} display_mode_t;
```

### Color Implementation
```c
#define COLOR_RESET   "\033[0m"
#define COLOR_BLUE    "\033[1;34m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_MAGENTA "\033[1;35m"
#define COLOR_CYAN    "\033[1;36m"
#define COLOR_YELLOW  "\033[1;33m"
```

## 📊 Version History

| Version | Feature | Release Date |
|---------|---------|--------------|
| v1.0.0 | Project Setup | Initial Release |
| v1.1.0 | Long Listing Format (-l) | Feature 2 |
| v1.2.0 | Vertical Column Display | Feature 3 |
| v1.3.0 | Horizontal Column Display (-x) | Feature 4 |
| v1.4.0 | Alphabetical Sort | Feature 5 |
| v1.5.0 | Colorized Output | Feature 6 |
| v1.6.0 | Recursive Listing (-R) | Feature 7 |

## 📝 Report

See [REPORT.md](REPORT.md) for detailed answers to assignment questions and implementation insights.

## 👨‍💻 Developer

- **Name**: Yahya Mobeen
- **Roll Number**: BSDSF23A039
- **Course**: Operating Systems
- **Instructor**: Muhammad Arif Butt, PhD

## 📄 License

This project is for educational purposes as part of the Operating Systems course.

## 🤝 Contributing

This is an academic assignment. No contributions are accepted.

---
```

