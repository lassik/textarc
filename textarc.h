struct textarc_entry {
        const char *filename;
        const char *type;
        char *link;
        char *uname;
        char *gname;

	unsigned long gid;
	unsigned long uid;
	unsigned long mode;

	unsigned int year;
	unsigned int month;
	unsigned int day;
	unsigned int hour;
	unsigned int minute;
	unsigned int second;
};

void write_entry(struct textarc_entry *e);
