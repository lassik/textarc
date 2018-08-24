cd "%~dp0"
wcc386 -q -wx textarc_pack_windows.c
wcc386 -q -wx textarc_pack.c
wcc386 -q -wx crc.c
wlink op q nam textarc-pack f textarc_pack_windows f textarc_pack f crc
