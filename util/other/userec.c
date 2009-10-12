#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "/usr/src/svn/branches/webdev/include/bbs.h"

// Old userec structure.
struct userec32 {
	char userid[IDLEN+2];
	int firstlogin;
	char lasthost[16];
	unsigned int numlogins;
	unsigned int numposts;
	int nummedals;
	int money;
	int bet;
	int dateforbet;
	char flags[2];
#ifdef ENCPASSLEN
	char passwd[ENCPASSLEN];
#else
	char passwd[PASSLEN];
#endif
	char username[NAMELEN];
	char ident[NAMELEN];
	char termtype[16];
	char reginfo[STRLEN-16];
	unsigned int userlevel;
	int lastlogin;
	int lastlogout;
	int stay;
	char realname[NAMELEN];
	char address[STRLEN];
	char email[STRLEN-12];
	unsigned int nummails;
	int lastjustify;
	char gender;
	unsigned char birthyear;
	unsigned char birthmonth;
	unsigned char birthday;
	int signature;
	unsigned int userdefine;
	int notedate;
	int noteline;
};

// New userec structure.
struct userec64 {
	unsigned int uid;
	unsigned int userlevel;
	unsigned int numlogins;
	unsigned int numposts;
	unsigned int stay;
	int nummedals;
	int money;
	int bet;
	char flags[2];
	char passwd[PASSLEN];
	unsigned int nummails;
	char gender;
	unsigned char birthyear;
	unsigned char birthmonth;
	unsigned char birthday;
	int signature;
	unsigned int userdefine;
	unsigned int prefs;
	int noteline;
	int64_t firstlogin;
	int64_t lastlogin;
	int64_t lastlogout;
	int64_t dateforbet;
	int64_t notedate;
	char userid[EXT_IDLEN];
	char lasthost[IP_LEN];
	char username[NAMELEN];
	char email[EMAIL_LEN];
	char reserved[8];
};

/**
 * Convert userec file (.PASSWDS) to adapt to 64-bit archs.
 */
int main(int argc, char **argv)
{
	// Check args.
	if (argc != 3) {
		printf("Usage: %s [old_passwd_file] [new_passwd_file]\n", argv[0]);
		return EXIT_FAILURE;
	}
	
	// Open files.
	FILE *from = fopen(argv[1], "rb");
	if (from == NULL) {
		printf("Failed to open file: %s", argv[1]);
		return EXIT_FAILURE;
	}
	FILE *to = fopen(argv[2], "wb");
	if (to == NULL) {
		printf("Failed to open file: %s", argv[2]);
		return EXIT_FAILURE;
	}

	struct userec32 u32;
	struct userec64 u64;
	while (fread(&u32, sizeof(u32), 1, from) == 1) {
		memset(&u64, 0,	sizeof(u64));
		// uid is a new field
		u64.userlevel = u32.userlevel;
		u64.numlogins = u32.numlogins;
		u64.numposts = u32.numposts;
		u64.stay = u32.stay;
		u64.nummedals = u32.nummedals;
		u64.money = u32.money;
		u64.bet = u32.bet;
		u64.flags[0] = u32.flags[0];
		u64.flags[1] = u32.flags[1];
		memcpy(u64.passwd, u32.passwd, sizeof(u64.passwd));
		u64.nummails = u32.nummails;
		u64.gender = u32.gender;
		u64.birthyear = u32.birthyear;
		u64.birthmonth = u32.birthmonth;
		u64.birthday = u32.birthday;
		u64.signature = u32.signature;
		u64.userdefine = u32.userdefine;
		// perfs is a new field
		u64.noteline = u32.noteline;
		u64.firstlogin = u32.firstlogin;
		u64.lastlogin = u32.lastlogin;
		u64.lastlogout = u32.lastlogout;
		u64.dateforbet = u32.dateforbet;
		u64.notedate = u32.notedate;
		// userid extended
		memcpy(u64.userid, u32.userid, sizeof(u32.userid));
		// lasthost extended
		memcpy(u64.lasthost, u32.lasthost, sizeof(u32.lasthost));
		memcpy(u64.username, u32.username, sizeof(u64.username) - 1);
		u64.username[sizeof(u64.username) - 1] = '\0';
		// email truncated from 68 to 40 bytes.
		memcpy(u64.email, u32.email, sizeof(u64.email) - 1);
		u64.email[sizeof(u64.email) - 1] = '\0';
		// u32 fields: lastjustify, ident, termtype are removed.

		fwrite(&u64, sizeof(u64), 1, to);

		// Write these u32 fields to register file.
		// reginfo, realname, address.
		char buf[256];
		snprintf(buf, sizeof(buf), "/home/bbs/home/%c/%s/register",
				u32.userid[0], u32.userid);
		FILE *reg = fopen(buf, "a");
		if (reg != NULL) {
			fprintf(reg, "\nreginfo:%s\nrealname:%s\naddress:%s\n",
					u32.reginfo, u32.realname, u32.address);
			fclose(reg);
		}
	}
	fclose(from);
	fclose(to);
	return EXIT_SUCCESS;
}

