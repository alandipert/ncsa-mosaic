/****************************************************************************
 * NCSA Mosaic for the X Window System                                      *
 * Software Development Group                                               *
 * National Center for Supercomputing Applications                          *
 * University of Illinois at Urbana-Champaign                               *
 * 605 E. Springfield, Champaign IL 61820                                   *
 * mosaic@ncsa.uiuc.edu                                                     *
 *                                                                          *
 * Copyright 1993-1995, Board of Trustees of the University of Illinois     *
 *                                                                          *
 * NCSA Mosaic software, both binary and source (hereafter, Software) is    *
 * copyrighted by The Board of Trustees of the University of Illinois       *
 * (UI), and ownership remains with the UI.                                 *
 *                                                                          *
 * The UI grants you (hereafter, Licensee) a license to use the Software    *
 * for academic, research and internal business purposes only, without a    *
 * fee.  Licensee may distribute the binary and source code (if released)   *
 * to third parties provided that the copyright notice and this statement   *
 * appears on all copies and that no charge is associated with such         *
 * copies.                                                                  *
 *                                                                          *
 * Licensee may make derivative works.  However, if Licensee distributes    *
 * any derivative work based on or derived from the Software, then          *
 * Licensee will (1) notify NCSA regarding its distribution of the          *
 * derivative work, and (2) clearly notify users that such derivative       *
 * work is a modified version and not the original NCSA Mosaic              *
 * distributed by the UI.                                                   *
 *                                                                          *
 * Any Licensee wishing to make commercial use of the Software should       *
 * contact the UI, c/o NCSA, to negotiate an appropriate license for such   *
 * commercial use.  Commercial use includes (1) integration of all or       *
 * part of the source code into a product for sale or license by or on      *
 * behalf of Licensee to third parties, or (2) distribution of the binary   *
 * code or source code to third parties that need it to utilize a           *
 * commercial product sold or licensed by or on behalf of Licensee.         *
 *                                                                          *
 * UI MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR   *
 * ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED          *
 * WARRANTY.  THE UI SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY THE    *
 * USERS OF THIS SOFTWARE.                                                  *
 *                                                                          *
 * By using or copying this Software, Licensee agrees to abide by the       *
 * copyright law and all other applicable laws of the U.S. including, but   *
 * not limited to, export control laws, and the terms of this license.      *
 * UI shall have the right to terminate this license immediately by         *
 * written notice upon Licensee's breach of, or non-compliance with, any    *
 * of its terms.  Licensee may be held legally responsible for any          *
 * copyright infringement that is caused or encouraged by Licensee's        *
 * failure to abide by the terms of this license.                           *
 *                                                                          *
 * Comments and questions are welcome and can be sent to                    *
 * mosaic-x@ncsa.uiuc.edu.                                                  *
 ****************************************************************************/
#define FONTNAME "-adobe-courier-medium-r-normal-*-17-*-*-*-*-*-iso10646-1"

#define TRANS_HTTP 0
#define TRANS_CCI 1

#define PROXY 0
#define PROXY_DOMAIN 1

struct ProxyDomain {
	struct ProxyDomain *next;
	struct ProxyDomain *prev;
	char *domain;
};


struct Proxy {
	struct Proxy *next;
	struct Proxy *prev;
	char *scheme;
	char *address;
	char *port;
	char *transport;
	int trans_val;
	int alive;
	struct ProxyDomain *list;
};

/* added function prototypes - DXP */

void AddProxyToList(), ShowProxyList(), EditProxyInfo(), CommitProxyInfo(),
        DismissProxy(), ClearProxyText(), FillProxyText(),  WriteProxies(),
        RemoveProxyInfo(), EditProxyDomainInfo(), DisplayErrorMessage(),
        ShowProxyDomainList(), CommitProxyDomainInfo(),
        CallEdit(), CallAdd(), CallEditDomain(), CallAddDomain(),
        CallRemoveProxy(), DestroyDialog(), PopProxyDialog(), DeleteProxy(),
        EditNoProxyInfo(), CenterDialog(), ProxyHelpWindow(), HelpWindow();

struct Proxy *ReadProxies(char *filename);

struct Proxy *ReadNoProxies(char *filename);

struct ProxyDomain *AddProxyDomain(char *sbDomain, struct ProxyDomain **pdList);

void DeleteProxyDomain(struct ProxyDomain *p);
