/*  Ben Fried deserves credit for writing the code that upon which
 *  this source is based. 				ADC 
 */
#include "../config.h"    

#if defined(KRB4) || defined(KRB5)

#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pwd.h>

#include "HTAAUtil.h"	/* for HTAA_KERBEROS_Vx defines */

#ifdef KRB4
#include <krb.h>
#define MAX_KDATA_LEN MAX_KTXT_LEN
static des_cblock session;        /* Our session key */
static des_key_schedule schedule; /* Schedule for our session key */
int k4checksum;
#endif

#ifdef KRB5
#include <krb5.h>
#include <krb_err.h>
#ifndef MAX_KDATA_LEN
#define MAX_KDATA_LEN 1250
#endif
#define KRB5_DEFAULT_LIFE 60*60*8	/* 8 hours */
krb5_auth_context *k5auth_context;
krb5_context	  k5context = 0;
krb5_ccache	  k5ccache;
#endif

/* is all of this necessary?  ADC */

char *getenv(), *getlogin(); /*  *index(), *malloc(), *realloc(); */
struct passwd *getpwnam(), *getpwuid();
static char *envariables[] = { "USER", "LOGNAME" };

extern int useAFS;		/* perhaps change name */

int doing_kerb_auth = 0;
char phost[1024];
char Hostname[1024];

#define NIL 0
#define T 1

/* Table for converting binary values to and from hexadecimal */
static char hex[] = "0123456789abcdef";
static char dec[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /*   0 -  15 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /*  16 -  37 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* ' ' - '/' */
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,   /* '0' - '?' */
    0,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* '@' - 'O' */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* 'P' - '_' */
    0,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* '`' - 'o' */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* 'p' - DEL */
};

/****************************************************************************
 * scheme_login -- lets user login for Kerberos TGT
 *
 * Returns 0 on success
 * Returns 1 on failure (after reporting error to user)
 ***************************************************************************/
int scheme_login(scheme)
    int scheme;
{
    char *username, *password, buf[BUFSIZ], erbuf[BUFSIZ];
    int code;

    username = (char *) prompt_for_string("Kerberos Username:");

    if (!username || !*username) {
	application_user_info_wait("You did not enter a Username.\nCannot get Kerberos ticket-granting ticket\n");
	return 1;
    }

    sprintf(buf, "Password for %s:", username);
    password = (char *) prompt_for_password(buf);

    if (!password || !*password) {
	application_user_info_wait("You did not enter a Password.\nCannot obtain ticket-granting ticket\n");
	return 1;
    }

    if (0) {	/* just to get things started */
    }
#ifdef KRB4
    else if (scheme == HTAA_KERBEROS_V4) {
	if (useAFS) {
	    code = AFSgetTGT(username, password, buf);
	}
        else {
	    code = k4getTGT(username, password, buf);
	}
    }
#endif
#ifdef KRB5
    else if (scheme == HTAA_KERBEROS_V5) {
	code = k5getTGT(username, password, buf);
    }
#endif

    memset(password, 0, sizeof(password));

    if (code) {
	sprintf(erbuf,"Kerberos login error:\n%s",buf);
	application_user_info_wait(erbuf);
	return 1;
    }

    return 0;
}

/****************************************************************************
 * AFSgetTGT() -- Uses klog to get the K4 TGT
 *
 * Returns 1 if pipe open fails
 * Returns 0 otherwise (even if klog fails!)
 ***************************************************************************/
int AFSgetTGT(username, password, err_string) 
    char *err_string, *username, *password;
{
    char reason[256];
    register int code;

    FILE *fp;
    char lngbuf[BUFSIZ*2],buf[BUFSIZ];
    char stop=0;
    int n;

    sprintf(buf,"klog -tmp -pr %s -pa %s 2>&1",username,password);
    if (!(fp=popen(buf,"r"))) {
	application_user_info_wait("Error: Could not startup external klog command.\n");
	return(1);
    }

    strcpy(buf," ");
    strcpy(lngbuf," ");
    n=1;
    while (n>0) {
	n=fread(buf,sizeof(char),BUFSIZ-1,fp);
	if (n>0) {
	    if (!stop && (n+strlen(lngbuf))<BUFSIZ*2) {
		buf[n]='\0';
		strcat(lngbuf,buf);
	    }
	    else {
		stop=1;
	    }
        }
    }

    pclose(fp);

    if (strlen(lngbuf)>1) {
	application_user_info_wait(lngbuf);
    }

    return(0);
}

#ifdef KRB4
/****************************************************************************
 * passwd_to_key -- convert users password to numeric key
 *
 * (cribbed from MIT's krb_get_in_tkt.c)
 * this can probably be augmented to support Transarc's string-to-key
 ***************************************************************************/
static int passwd_to_key(user,instance,realm,passwd,key)
    char *user, *instance, *realm, *passwd;
    C_Block key;
{
    string_to_key(passwd, key);
    return (0);
}

/****************************************************************************
 * k4getTGT() -- calls K4 libraries to get TGT   (non-AFS)
 *
 * Returns 0 on success (err_string = "")
 * Returns 1 on failure (err_string = something meaningful)
 ***************************************************************************/
int k4getTGT(username, password, err_string)
    char *err_string, *username, *password;
{
    char instance[INST_SZ], *sinstance;
    char realm[REALM_SZ];
    char pname[MAX_K_NAME_SZ];
    int  lifetime = DEFAULT_TKT_LIFE;
    int  code;

    *instance = '\0';   /* assume client principal "user@realm" (no instance */

    krb_get_lrealm(realm, 1);           /* get local realm */

    strcpy(pname, username);
    strcat(pname, "@");
    strncat(pname, realm, REALM_SZ);

    sinstance = realm;  /* assume tgt is for krbtgt.realm@realm */

    in_tkt(pname, instance);    /* to initialize ticket store  */

    code = krb_get_in_tkt(username, instance, realm, "krbtgt", sinstance,
                          lifetime, passwd_to_key , NULL, password);

    if (code == INTK_BADPW) {
        strcpy(err_string, "Wrong password");
        return 1;
   }
    else if (code != INTK_OK) {
        strcpy(err_string, krb_err_txt[code]);
        return 1;
    }
    return 0;
}
#endif /* KRB4 */
#ifdef KRB5
/****************************************************************************
 * k5getTGT() -- calls K5 libraries to get TGT   (non-AFS)  
 *		 most of this was copied from the Krb5 kinit.c
 *
 * Returns 0 on success (err_string = "")
 * Returns 1 on failure (err_string = something meaningful)
 ***************************************************************************/
int k5getTGT(username, password, err_string)
    char *username, *password, *err_string;
{
    int code, options = 0; 	/* KRB5_DEFAULT_OPTIONS = 0 */
    krb5_creds my_creds;
    krb5_timestamp now;
    krb5_principal me, server;
    krb5_preauthtype *preauth = NULL;
/* SWP -- For CodeCenter --
    krb5_data tgtname = {
        0,
        KRB5_TGS_NAME_SIZE,
        KRB5_TGS_NAME
    };
*/
    krb5_data tgtname;

    tgtname.magic = 0;
    tgtname.length = KRB5_TGS_NAME_SIZE;
    tgtname.data = KRB5_TGS_NAME;

    if (code = krb5_timeofday(k5context, &now)) {
        sprintf(err_string,"Wouldn't tell you the time of day");
        return 1;
    }

    if (code = krb5_parse_name(k5context, username, &me)) {
        sprintf(err_string,"Couldn't find client principal name");
        return 1;
    }

    if (code = krb5_cc_initialize (k5context, k5ccache, me)) {
	sprintf(err_string,"Couldn't initialize credentials cache");
	return 1;
    }

    memset((char *)&my_creds, 0, sizeof(my_creds));
    my_creds.client = me;

    if (code = krb5_build_principal_ext(k5context, &server,
                                        krb5_princ_realm(k5context, me)->length,
                                        krb5_princ_realm(k5context, me)->data,
                                        tgtname.length, tgtname.data,
                                        krb5_princ_realm(kcontext, me)->length,
                                        krb5_princ_realm(kcontext, me)->data,
                                        0)) {
        sprintf(err_string,"Couldn't build server principal name");
        return 1;
    }

    my_creds.server = server;
    my_creds.times.starttime = 0;       
    my_creds.times.endtime = 0;		/* now + KRB5_DEFAULT_LIFE; */
    my_creds.times.renew_till = 0;

    code = krb5_get_in_tkt_with_password(k5context, options, 0,
                              NULL, preauth, password, k5ccache, &my_creds, 0);

    /* eytpyes = NULL means use default etype for decoding tgt */
    /* addrs = 0 means use default local (client machine) address  */

    if (code) {
	sprintf(err_string,"krb5_get_in_tkt error: %s", error_message(code));
	return 1;
    } 
    else {
	return 0; 
    }
}
#endif

/*************************************************************************
 * kdata_to_str -- convert 8-bit char array to ascii string
 *
 * Accepts:  input array and length
 * Returns:  a pointer to the result, or null pointer on malloc failure
 *           The caller is responsible for freeing the returned value.
 *
 * Changed to accomodate general strings with length, due to conflict between
 *    KTEXT and krb5_data types  ( 6/28/95 ADC)
 ************************************************************************/
static char *kdata_to_str(in_data, length)
    char *in_data;                      /* char FAR ?? */
    int length;
{
    char *result, *p;
    int i;

    p = result = malloc(length*2+1);
    if (!result) return (char *) NULL;

    for (i=0; i < length; i++) {
        *p++ = hex[(in_data[i]>>4)&0xf];
        *p++ = hex[(in_data[i])&0xf];
    }
    *p++ = '\0';
    return result;
}


/*************************************************************************
 * str_to_kdata -- Converts ascii string to a (binary) char array
 *
 * Accepts: string to convert
 *          pointer to output array
 * Returns: length of output array, NIL on failure
 ************************************************************************/
int str_to_kdata(in_str, out_str)
    char *in_str;
    char *out_str;
{
    int inlen, outlen;

    inlen = strlen(in_str);
    if (inlen & 1) return NIL;  /* must be even number, in this scheme */
    inlen /= 2;
    if (inlen > MAX_KDATA_LEN) return NIL;

    for (outlen=0; *in_str; outlen++, in_str += 2) {
        out_str[outlen] = (dec[in_str[0]]<<4) + dec[in_str[1]];
    }
    return outlen;
}


/****************************************************************************
 * compose_kerberos_auth_string 
 *
 * Accepts: scheme (one of the HTAA_KERBEROS values)
 * 	    hostname
 * Returns: Authorization string (NULL, upon failure)
 ***************************************************************************/
char *compose_kerberos_auth_string(scheme, hostname)
    HTAAScheme scheme;
    char *hostname;
{
    struct hostent *host_name;
    char user[BUFSIZ], *inst, *pass, *tmp = 0;
    int code, retval, firsttime = 1;
    char buf[BUFSIZ], krb_err_str[BUFSIZ];
#ifdef KRB4
    CREDENTIALS k4cr, k4c;
    KTEXT_ST k4authent;
    Key_schedule k4key_s;
    char *krb_get_phost();
    static char *better_err_str[] = { "Server principal unrecognized by KDC",
				      "System clocks out of sync" };
#endif
#ifdef KRB5
    krb5_data k5ap_req;
    krb5_principal k5clientp, k5serverp;
    krb5_creds k5in_creds, *k5out_creds;
    krb5_timestamp now;
#endif

    while (code || firsttime) {

#ifdef KRB4
	if (scheme == HTAA_KERBEROS_V4) {
   	    k4checksum = time(0) ^ getpid();
	    strcpy(phost, krb_get_phost(hostname));
    	    code = krb_mk_req(&k4authent, "khttp", phost,
		      	      krb_realmofhost(hostname), k4checksum);

    	    if (!code) {  		/* check for ticket expired */

		code = krb_get_cred("khttp",phost,krb_realmofhost(hostname),&k4c);

		if (!code) {
		    k4c.issue_date += ((unsigned char) k4c.lifetime) * 5 * 60;
		    if (time(0) >= k4c.issue_date) {
			code=26;
		    }
		}
	    }

	    if (!code) {
		strcpy(user,k4c.pname);
		pass = kdata_to_str(k4authent.dat, k4authent.length);
	    }
	    else if (code == 1) {   /* normally "Principal Expired" */
		strcpy(krb_err_str, better_err_str[0]);
	    }
	    else if (code == RD_AP_TIME) {
		strcpy(krb_err_str, better_err_str[1]);
	    }
	    else {
		strcpy(krb_err_str, krb_err_txt[code]);
	    }
	}
#endif
#ifdef KRB5
	if (scheme == HTAA_KERBEROS_V5) {
	    krb_err_str[0] = '\0';

	    if (!k5context) {
		krb5_init_context(&k5context);
                if (code) {
                    sprintf(krb_err_str,"Error initializing Kerb5 context: %s\n",error_message(code)); 
                    application_user_info_wait(krb_err_str);
                    return (char *) NULL;
                }

		krb5_init_ets(k5context);

		code = krb5_cc_default(k5context, &k5ccache);
		if (code) {
		    sprintf(krb_err_str,"Error initializing Credentials Cache: %s\n",error_message(code));
		    application_user_info_wait(krb_err_str);
		    return (char *) NULL;
		}
	    }

	
	    code = krb5_mk_req(k5context, &k5auth_context, AP_OPTS_USE_SESSION_KEY,
			       "khttp", hostname, NULL, k5ccache, &k5ap_req);

    	    if (!code) { 	

		/* get username from credentials cache */

	        code = krb5_cc_get_principal(k5context, k5ccache, &k5clientp);
	        if (code) {
		    sprintf(krb_err_str,"Error getting client principal: %s\n",error_message(code));
		    application_user_info_wait(krb_err_str);
		    return (char *) NULL;
	        }

	        strcpy(user, k5clientp->data->data);

		/* get server credentials to check for expiration */

		code = krb5_timeofday(k5context, &now);
		if (code) {
                    sprintf(krb_err_str,"Couldn't give ya the time of day: %s\n",error_message(code));
                    application_user_info_wait(krb_err_str);
                    return (char *) NULL;
                }

		krb5_sname_to_principal(k5context, hostname, "khttp", 
				        KRB5_NT_SRV_HST, &k5serverp);

		memset((char *)&k5in_creds, 0, sizeof(k5in_creds));

		k5in_creds.server = k5serverp;
		k5in_creds.client = k5clientp;
		k5in_creds.times.endtime = now + KRB5_DEFAULT_LIFE;
		k5in_creds.keyblock.keytype = 0;
		k5in_creds.authdata = NULL;

		code = krb5_get_credentials(k5context,KRB5_GC_CACHED,k5ccache,&k5in_creds,&k5out_creds);

		if ((code == KRB5_CC_NOTFOUND) || (now >= k5out_creds->times.endtime)) {    
			/* replace "Matching creds not found" */
			sprintf(krb_err_str,"Kerberos ticket expired\n");
			code = 666;
		}

		krb5_free_cred_contents(k5context, &k5in_creds);
		krb5_free_cred_contents(k5context, &k5out_creds);
		krb5_free_principal(k5context, k5clientp);
		krb5_free_principal(k5context, k5serverp);
	    }

            if (code) {
	   	if (!krb_err_str[0]) {
		    sprintf(krb_err_str,"krb5_mk_req: %s\n",error_message(code));
		}
            }
	    else { 
                pass = kdata_to_str(k5ap_req.data, k5ap_req.length);
	    }
	}
#endif

	if (code) {
	    sprintf(buf,"Error: %s\n\nWould you like to attempt to login\nto obtain a ticket-granting ticket?\n",krb_err_str);

	    if (!prompt_for_yes_or_no(buf)) {
		return (char *) NULL;
	    }
	    else {
		if (scheme_login(scheme))
		    return (char *) NULL;
	    }
	} 		/* if (code)    */
	firsttime = 0;
    } 			/* while (code || firsttime) */


    if (!pass) {
	sprintf(buf,"Error: Couldn't convert kdata to string (out of memory)\nAborting...\n");
	application_user_info_wait(buf);
	return (char *) NULL;
    }

    if (!tmp)
	tmp = malloc(strlen(pass)+strlen(user)+40);
    else
	tmp = realloc(tmp, strlen(pass)+strlen(user)+40);

    if (!tmp) {
	/* XXX out of memory */
	fprintf(stderr,"out of memory!!\n");
	fflush(stderr);
	exit(1);
    }

    doing_kerb_auth = 1;
    strcpy(Hostname, hostname);
    sprintf(tmp, "%s %s", user, pass);
    free(pass);

    return tmp;
}

/*************************************************************************
 * validate_kerberos_server_auth
 * Accepts: scheme (one of the HTAA_KERBEROS values)
 *          the Authorization line from the request
 * Returns: NIL on success, T on failure 
	    (currently return value not used)
 ************************************************************************/
int validate_kerberos_server_auth(scheme, str)
    HTAAScheme  scheme;
    char *str;
{
    int retval;
    char buf[256], *tmp;


    if (!doing_kerb_auth) {
        /* sprintf(buf, "Received kerberos credentials from server %s, but I'm not doing kerberos authentication!\n", Hostname);
        application_user_info_wait(buf);
	*/
        return 1;
    }

    if (*str != '[') {
	fprintf(stderr,"\n\nleft bracket not found: [%s]\n",str);
	goto krb_server_validate_getout;
    }

    if (tmp = index(str, ' ')) *tmp = NULL;
    tmp = str + strlen(str) - 1;
    if (*tmp != ']') {
	fprintf(stderr,"\n\nright bracket not found\n\n");
	goto krb_server_validate_getout;
    }
    *tmp = 0;	/* end string where right bracket was */
    str++;	/* get rid of left bracket */


    if (0) {  /* just to get things started */
    }
#ifdef KRB4
    else if (scheme == HTAA_KERBEROS_V4) {
	retval = k4validate_kerberos_server_auth(str);
    }
#endif 
#ifdef KRB5
    else if (scheme == HTAA_KERBEROS_V5) {
	retval = k5validate_kerberos_server_auth(str);
    }
#endif
    else {
	retval = 1;
    }

    /* Reset stupid global state variables for the next auth we do.  */

krb_server_validate_getout:

    if (retval) {
        sprintf(buf, "                  Warning:\nAuthentication of server %s failed", Hostname);
        application_user_info_wait(buf);
    }

/*
    bzero(Hostname, sizeof(Hostname));
    bzero(phost, sizeof(phost));
*/
    memset(Hostname,0,sizeof(Hostname));
    memset(phost,0,sizeof(phost));

    doing_kerb_auth = 0;

    return retval;
}
/************************************************************************/
#ifdef KRB4
int k4validate_kerberos_server_auth(str)
    char *str;
{
    KTEXT_ST k4authent;
    CREDENTIALS k4cr;
    Key_schedule k4key_s;
    char buf[256];

    k4authent.length = str_to_kdata(str, k4authent.dat);

    if (k4authent.length == 0 || k4authent.length != 8) {
	fprintf(stderr,"\n\nbad length\n\n");
	return 1;
    }

    if (krb_get_cred("khttp", phost, krb_realmofhost(Hostname),&k4cr)) {
	fprintf(stderr,"\n\ncouldn't get credentials");
	return 1;
    }

    des_key_sched(k4cr.session, k4key_s);
    des_ecb_encrypt(k4authent.dat, k4authent.dat, k4key_s, 0);

    if (ntohl(*(long *)k4authent.dat) != k4checksum + 1) {
	fprintf(stderr,"\n\nchecksum just doesn't check out\n\n");
	return 1; 
    }
 
    return 0;
}
#endif
/************************************************************************/
#ifdef KRB5
int k5validate_kerberos_server_auth(instr)
    char *instr;
{
    int code;
    char buf[256];
    char tmpstr[MAX_KDATA_LEN];
    krb5_data k5ap_rep;
    krb5_ap_rep_enc_part *k5ap_rep_result;

    k5ap_rep.length = str_to_kdata(instr, tmpstr);

    if (k5ap_rep.length == 0) 
 	return 1;

    k5ap_rep.data = tmpstr;

    code = krb5_rd_rep(k5context, k5auth_context, &k5ap_rep, &k5ap_rep_result);

    krb5_free_ap_rep_enc_part(k5context, k5ap_rep_result);

    if (code) 
 	return 1;

    return 0;
}
#endif /* KRB5 */
/************************************************************************/
#endif /* KRB4 or KRB5 */

