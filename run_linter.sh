#!/usr/bin/env bash
# run_linter.sh - Runs formatting and static analysis on all C++ files.

set -e

# Find all C++ source and header files in the project
FILES=$(find src include tests -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \))

echo "=== Running clang-format ==="
for file in $FILES; do
    echo "Formatting: $file"
    clang-format -i "$file"
done

echo ""
echo "=== Running clang-tidy ==="
if [ ! -f compile_commands.json ]; then
    echo "Warning: compile_commands.json not found! Generating with xmake..."
    xmake project -k compile_commands
fi

# Run clang-tidy on each file using the compile_commands.json database
for file in $FILES; do
    echo "Tidying: $file"
    # We use || true so that warnings don't abort the entire script run
    clang-tidy -p . "$file" || true
done

echo ""
echo "Linting and formatting complete!"
