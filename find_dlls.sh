#!/usr/bin/env bash

if [[ ! -f "$1" ]]; then
    echo "ERROR: $1 does not exist." >&2
    exit 1
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