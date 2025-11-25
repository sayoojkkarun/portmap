#!/bin/bash
# checkpatch wrapper for portmap project
# Runs Linux kernel checkpatch.pl with appropriate settings

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CHECKPATCH="$SCRIPT_DIR/checkpatch.pl"

if [ ! -f "$CHECKPATCH" ]; then
    echo "Error: checkpatch.pl not found in scripts/"
    echo "Download it with: wget https://raw.githubusercontent.com/torvalds/linux/master/scripts/checkpatch.pl -O scripts/checkpatch.pl"
    exit 1
fi

# Default options for this project
OPTIONS=(
    "--no-tree"           # Don't check for kernel tree
    "--max-line-length=100"  # Allow up to 100 chars per line
    "--ignore=SPDX_LICENSE_TAG"  # We use SPDX headers
    "--ignore=FILE_PATH_CHANGES"  # Ignore file path warnings
)

# If no files specified, check all source files
if [ $# -eq 0 ]; then
    echo "Checking all C source files..."
    FILES=$(find src include -name "*.c" -o -name "*.h")
    
    ERRORS=0
    for file in $FILES; do
        echo ""
        echo "Checking: $file"
        perl "$CHECKPATCH" "${OPTIONS[@]}" -f "$file" || ERRORS=$((ERRORS + 1))
    done
    
    echo ""
    if [ $ERRORS -eq 0 ]; then
        echo "All files passed checkpatch!"
    else
        echo "Some files have checkpatch warnings/errors"
        exit 1
    fi
else
    # Check specified files
    perl "$CHECKPATCH" "${OPTIONS[@]}" "$@"
fi
