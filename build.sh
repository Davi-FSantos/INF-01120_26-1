#!/usr/bin/env bash
# Build script for the project.

# Function to build the program.
function run_build() {
    case "$2" in
        "release")
            echo "xmake f -m release"
            echo "xmake"
            ;;

        "debug")
            echo "xmake f -m debug"
            echo "xmake"
            ;;

        *)
            echo "error: unknown argument -> $2"
            ;;
    esac
}

if [ "$#" -eq 0 ]; then
    echo "error: no argument provided."
    exit 1
elif [ "$1" = "build" ]; then
    run_build "$1" "$2"
else
    echo "error: unknown command."
    exit 1
fi
