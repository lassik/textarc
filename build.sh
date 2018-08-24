#!/bin/sh
set -eux
${CC:-clang} -Wall -Wextra -g -o textarc-pack textarc_pack.c crc.c
