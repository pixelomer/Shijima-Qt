#!/usr/bin/env bash

# Adapted from https://github.com/pixelomer/BadApple

# Fail on error
set -e

if [ -z "$1" ]; then
    echo "Usage: $0 <files...>" >&2
    exit 1
fi

file_size="$(wc -c < "$1" | awk '{print $1}')"

echo "#include \"DefaultMascot.hpp\""
echo

echo "const std::map<std::string, std::pair<const char *, size_t>> defaultMascot = {"
for file in "$@"; do
    length="$(wc -c < "${file}")"
    name="$(basename "${file}")"
    echo -ne "\t"
    echo "{ \"${name}\", { "
    (cat "$file"; echo -ne '\x00') | hexdump -v -e '16/1 "_x%02X" "\n"' | \
        sed 's/_/\\/g; s/\\x  //g; s/.*/    "&"/'
    echo -e "\t, ${length} } },"
    ((offset+=length))
done
echo -e "};"
