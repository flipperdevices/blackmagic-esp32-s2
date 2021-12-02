#!/bin/sh

cd $(dirname "$0")

FLASH_COMMAND="$(cat flash.command)"

if [ -n "$1" ]; then
    if [ -c "$1" ]; then
        PORT="$1"

        TOOL="$(echo "$FLASH_COMMAND" | sed -e 's/\s.*$//')"
        ESPTOOL=`which $TOOL`
        if [ -n "$ESPTOOL" ] && [ -x "$ESPTOOL" ]; then
            echo "$FLASH_COMMAND" | sed -e "s|(PORT)|$PORT|g" | sh -x
            exit $?
        else
            echo "Can't find executable $ESPTOOL"
            echo
        fi
    else
        echo "Invalid port $1"
        echo
    fi
fi

echo "Usage: $0 <serial_port>"
echo "Available ports:"
find /dev/tty* | grep -E "ACM|USB|\." --color=never
exit 255;
