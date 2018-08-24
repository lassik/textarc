#!/bin/sh
set -eux
${CC:-clang} -Wall -Wextra -g -o textarc-pack textarc_pack_unix.c textarc_pack.c crc.c
