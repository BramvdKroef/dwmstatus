
all:
	gcc `xml2-config --cflags` \
	`curl-config --cflags` \
	`xml2-config --libs` \
	`curl-config --libs`  \
	dwmstatus-bram.c -o dwmstatus-bram -O2 -s -lX11
