
/* MODULE							HTPasswd.c
**		PASSWORD FILE ROUTINES
**
** AUTHORS:
**	AL	Ari Luotonen	luotonen@dxcern.cern.ch
**
** HISTORY:
**
**
** BUGS:
**
**
*/

#include "../config.h"
#include <string.h>

#include "HTUtils.h"
#include "HTAAUtil.h"	/* Common parts of AA	*/
#include "HTAAFile.h"	/* File routines	*/
#include "HTPasswd.h"	/* Implemented here	*/
#include "tcp.h"	/* FROMASCII()		*/

extern char *crypt();

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

PRIVATE char salt_chars [65] =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

/* PUBLIC						HTAA_encryptPasswd()
**		ENCRYPT PASSWORD TO THE FORM THAT IT IS SAVED
**		IN THE PASSWORD FILE.
** ON ENTRY:
**	password	is a string of arbitrary lenght.
**
** ON EXIT:
**	returns		password in one-way encrypted form.
**
** NOTE:
**	Uses currently the C library function crypt(), which
**	only accepts at most 8 characters long strings and produces
**	always 13 characters long strings. This function is
**	called repeatedly so that longer strings can be encrypted.
**	This is of course not as safe as encrypting the entire
**	string at once, but then again, we are not that paranoid
**	about the security inside the machine.
**
*/
PUBLIC char *HTAA_encryptPasswd ARGS1(WWW_CONST char *, password)
{
    char salt[3];
    char chunk[9];
    char *result;
    char *tmp;
    WWW_CONST char *cur = password;
    int len = strlen(password);
    extern time_t theTime;
    int random = (int)theTime;	/* This is random enough */

    if (!(result = (char*)malloc(13*((strlen(password)+7)/8) + 1)))
	outofmem(__FILE__, "HTAA_encryptPasswd");

    *result = (char)0;
    while (len > 0) {
	salt[0] = salt_chars[random%64];
	salt[1] = salt_chars[(random/64)%64];
	salt[2] = (char)0;

	strncpy(chunk, cur, 8);
	chunk[8] = (char)0;

	tmp = crypt((char*)password, salt);  /*crypt() doesn't change its args*/
	strcat(result, tmp);
	free(tmp);

	cur += 8;
	len -= 8;
    } /* while */

    return result;
}



/* PUBLIC						HTAA_passwdMatch()
**		VERIFY THE CORRECTNESS OF A GIVEN PASSWORD
**		AGAINST A ONE-WAY ENCRYPTED FORM OF PASSWORD.
** ON ENTRY:
**	password	is cleartext password.
**	encrypted	is one-way encrypted password, as returned
**			by function HTAA_encryptPasswd().
**			This is typically read from the password
**			file.
**
** ON EXIT:
**	returns		YES, if password matches the encrypted one.
**			NO, if not, or if either parameter is NULL.
*/
PUBLIC BOOL HTAA_passwdMatch ARGS2(WWW_CONST char *, password,
				   WWW_CONST char *, encrypted)
{
    char *result;
    int len;
    int status;

    if (!password || !encrypted ||
	13*((strlen(password)+7)/8) != strlen(encrypted))
	return NO;

    len = strlen(encrypted);

    if (!(result = (char*)malloc(len + 1)))
	outofmem(__FILE__, "HTAA_encryptPasswd");

    *result = (char)0;
    while (len > 0) {
	char salt[3];
	char chunk[9];
	WWW_CONST char *cur1 = password;
	WWW_CONST char *cur2 = encrypted;
	char *tmp;

	salt[0] = *cur2;
	salt[1] = *(cur2+1);
	salt[2] = (char)0;

	strncpy(chunk, cur1, 8);
	chunk[8] = (char)0;

	tmp = crypt((char*)password, salt);
	strcat(result, tmp);
	free(tmp);

	cur1 += 8;
	cur2 += 13;
	len -= 13;
    } /* while */

    status = strcmp(result, encrypted);

#ifndef DISABLE_TRACE
    if (www2Trace)
	fprintf(stderr,
		"%s `%s' (encrypted: `%s') with: `%s' => %s\n",
		"HTAA_passwdMatch: Matching password:",
		password, result, encrypted,
		(status==0 ? "OK" : "INCORRECT"));
#endif

    free(result);

    if (status==0)
	return YES;
    else
	return NO;
}


/* PUBLIC						HTAAFile_readPasswdRec()
**			READ A RECORD FROM THE PASSWORD FILE
** ON ENTRY:
**	fp		open password file
**	out_username	buffer to put the read username, must be at
**			least MAX_USERNAME_LEN+1 characters long.
**	out_passwd	buffer to put the read password, must be at
**			least MAX_PASSWORD_LEN+1 characters long.
** ON EXIT:
**	returns		EOF on end of file,
**			otherwise the number of read fields
**			(i.e. in a correct case returns 2).
**	out_username	contains the null-terminated read username.
**	out_password	contains the null-terminated read password.
**
** FORMAT OF PASSWORD FILE:
**	username:password:maybe real name or other stuff
**				(may include even colons)
**
**	There may be whitespace (blanks or tabs) in the beginning and
**	the end of each field. They are ignored.
*/
PUBLIC int HTAAFile_readPasswdRec ARGS3(FILE *, fp,
					char *, out_username,
					char *, out_password)
{
    char terminator;
    
    terminator = HTAAFile_readField(fp, out_username, MAX_USERNAME_LEN);

    if (terminator == EOF) {				/* End of file */
	return EOF;
    }
    else if (terminator == CR  ||  terminator == LF) {	/* End of line */
	HTAAFile_nextRec(fp);
	return 1;
    }
    else {
	HTAAFile_readField(fp, out_password, MAX_PASSWORD_LEN);
	HTAAFile_nextRec(fp);
	return 2;
    }
}



/* PUBLIC						HTAA_checkPassword()
**		CHECK A USERNAME-PASSWORD PAIR
** ON ENTRY:
**	username	is a null-terminated string containing
**			the client's username.
**	password	is a null-terminated string containing
**			the client's corresponding password.
**	filename	is a null-terminated absolute filename
**			for password file.
**			If NULL or empty, the value of
**			PASSWD_FILE is used.
** ON EXIT:
**	returns		YES, if the username-password pair was correct.
**			NO, otherwise; also, if open fails.
*/
PUBLIC BOOL HTAA_checkPassword ARGS3(WWW_CONST char *, username,
				     WWW_CONST char *, password,
				     WWW_CONST char *, filename)
{
    FILE *fp = NULL;
    char user[MAX_USERNAME_LEN+1];
    char pw[MAX_PASSWORD_LEN+1];
    int status;
    
    if (filename && *filename)  fp = fopen(filename,"r");
    else			fp = fopen(PASSWD_FILE,"r");

    if (!fp) {
#ifndef DISABLE_TRACE
	if (www2Trace) fprintf(stderr, "%s `%s'\n",
			   "HTAA_checkPassword: Unable to open password file",
			   (filename && *filename ? filename : PASSWD_FILE));
#endif
	return NO;
    }
    do {
	if (2 == (status = HTAAFile_readPasswdRec(fp,user,pw))) {
#ifndef DISABLE_TRACE
	    if (www2Trace)
		fprintf(stderr,
			"HTAAFile_validateUser: %s \"%s\" %s \"%s:%s\"\n",
			"Matching username:", username,
			"against passwd record:", user, pw);
#endif
	    if (username  &&  user  &&  !strcmp(username,user)) {
		/* User's record found */
		if (pw) { /* So password is required for this user */
		    if (!password ||
			!HTAA_passwdMatch(password,pw)) /* Check the password */
			status = EOF;	/* If wrong, indicate it with EOF */
		}
		break;  /* exit loop */
	    }  /* if username found */
	}  /* if record is ok */
    } while (status != EOF);

    fclose(fp);
    
#ifndef DISABLE_TRACE
    if (www2Trace) fprintf(stderr, "HTAAFile_checkPassword: (%s,%s) %scorrect\n",
		       username, password, ((status != EOF) ? "" : "in"));
#endif

    if (status == EOF)  return NO;  /* We traversed to the end without luck */
    else                return YES; /* The user was found */
}

