# portmap - Smart Port Manager for Linux

A user-friendly command-line tool for managing network ports on Linux. Provides an easier interface than `lsof` and `netstat` for common port management tasks that developers face daily.

![version](https://img.shields.io/badge/version-1.0.0-blue)
![C11](https://img.shields.io/badge/C-11-blue)
![MISRA](https://img.shields.io/badge/MISRA-compliant-green)

## Features

- **List Ports**: View all active network connections with process details
- **Kill by Port**: Terminate processes using specific ports
- **Find by Process**: Search for ports used by a specific process name
- **Check Availability**: Verify if a port is free before starting services
- **Flexible Filtering**: Filter by port number, range, or protocol (TCP/UDP)
- **JSON Output**: Machine-readable output for scripting and automation

## Installation

### From Source

```bash
git clone https://github.com/sayoojkkarun/portmap.git
cd portmap
make
sudo make install
```

### From Snap

```bash
sudo snap install portmap
sudo snap connect portmap:network-observe
sudo snap connect portmap:system-observe
sudo snap connect portmap:process-control
```

## Usage

### Basic Commands

```bash
# List all active ports
portmap list

# Check if a specific port is available
portmap check 8080

# Find ports used by a process
portmap find python

# Kill process using a port
portmap kill 8080
```

### Advanced Filtering

```bash
# List only TCP ports
portmap list --tcp

# List only UDP ports
portmap list --udp

# Filter by specific port
portmap list --port 22

# Filter by port range
portmap list --range 3000-4000

# JSON output for scripting
portmap list --json
```

### Examples

**List all listening ports:**
```bash
portmap list
```

**Check if port 3000 is available before starting a dev server:**
```bash
if portmap check 3000; then
    npm start
else
    echo "Port 3000 is already in use!"
fi
```

**Find all ports used by Node.js:**
```bash
portmap find node
```

**Kill a process blocking port 8080:**
```bash
portmap kill 8080
```

## Building

### Requirements

- GCC compiler
- Linux kernel 2.6+ (uses `/proc` filesystem)
- Make

###Build Options

```bash
# Standard build
make

# Debug build with symbols
make debug

# Clean build artifacts
make clean

# Run unit tests
make test

# Install system-wide
sudo make install

# Uninstall
sudo make uninstall
```

## Unit Testing

portmap includes a comprehensive unit test suite with **57 tests** covering:

- **Utility functions** (22 tests): String manipulation, parsing, validation
- **Port scanner** (35 tests): List management, dynamic growth, search functionality

Run tests:
```bash
make test
```

All tests compile with zero warnings under strict MISRA-compliant flags and provide colored output showing pass/fail status.

## Code Style Checking

The project includes Linux kernel's `checkpatch.pl` for code style verification:

```bash
# Check all source files
make checkpatch

# Check specific file
./scripts/check_style.sh -f src/main.c
```

This ensures consistent coding style across the project.

## Code Quality

This project follows **automotive industry standards**:

- MISRA C compliant
- Zero compiler warnings with strict flags (`-Wall -Wextra -Werror -pedantic`)
- C11 standard
- Comprehensive error handling
- Well-documented code

## How It Works

`portmap` reads network connection information directly from the Linux `/proc` filesystem:

1. **Port Scanning**: Parses `/proc/net/tcp` and `/proc/net/udp` for active connections
2. **Process Matching**: Maps socket inodes from `/proc/net/*` to processes via `/proc/[pid]/fd/`
3. **Process Information**: Reads process details from `/proc/[pid]/cmdline`, `/proc/[pid]/status`

This approach is efficient and doesn't require external dependencies.

## Contributing

Contributions welcome! Please ensure:

- Code compiles with zero warnings (`make clean && make`)
- Follows MISRA C guidelines
- Updates this README if adding features

## License

MIT License - see LICENSE file for details

## Author

**Sayooj K Karun**
- Email: sayooj@aerlync.com
- Email: sayoojkkarun@gmail.com

Built with ❤️ for developers who work with network ports daily
