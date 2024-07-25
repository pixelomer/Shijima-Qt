#!/usr/bin/env bash

fail() {
    [[ ! -z "$@" ]] && echo "$@" >&2
    echo "Usage: $0 <program.exe> <dll_root>" >&2
    echo "  Finds the absolute paths to all DLLs required by the program"
    echo "  and prints them as a space-separated list."
    exit 1
}

if [[ $# -ne 2 ]]; then
    fail "ERROR: Invalid arguments"
fi

if [[ ! -f "$1" ]]; then
    fail "ERROR: $1 does not exist."
fi

if [[ ! -d "$2" ]]; then
    fail "ERROR: $2 is not a directory."
fi

if [[ -z "$OBJDUMP" ]]; then
    OBJDUMP=objdump
fi

dlls=("$1")

for ((i=0; i<${#dlls[@]}; i++)); do
    current="${dlls[$i]}"
    if [[ $i -gt 0 ]]; then
        current="$2/${current}"
    fi
    if [[ ! -f "${current}" ]]; then
        dlls=(${dlls[@]/${dlls[$i]}})
        ((--i))
        continue
    fi
    #echo "========= DLL: ${dlls[$i]} ==========" >&2
    dump="$(${OBJDUMP} -p "${current}" | grep -oE 'DLL Name: [^ ]+\.dll' \
        | awk '{ print $3; }')"
    #echo "${dump}" >&2
    while read dll; do
        is_new=1
        for entry in "${dlls[@]}"; do
            if [[ "$dll" = "${entry}" ]]; then
                is_new=0
                break
            fi
        done
        if [[ $is_new -eq 1 ]]; then
            dlls+=("${dll}")
        fi
    done <<<"${dump}"
done
for dll in ${dlls[@]/${dlls[0]}}; do
    echo -n "$2/${dll} ";
done