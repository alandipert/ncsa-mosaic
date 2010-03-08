#include "../config.h"
/*After much deliberation, it was decided NOT to Internationalize this file.*/

/*
  
  This module provides access routines to take an HDF file and return an
  HTML description of its contents.

  The function hdfGrokFile() will take a whole file and return stuff that
  can be seen at the "top level" (i.e. all datasets, all raster images and
  all Vgroups which are not contained within other Vgroups).

  The function hdfGrokReference() will return a description of what can
  be "seen" from a given location in the file.  Typically, this will be how
  users can navigate their way through a forest of Vgroups.
  
*/

/*
#define CHOUCK
*/

#ifdef HAVE_HDF
#define HDF

#if defined(__sgi)
#define IRIS4
#endif
#if defined(_IBMR2)
#define IBM6000
#endif
#if defined(sun)
#define SUN
#ifndef Void
#define Void void
#endif
#endif
#if defined(cray)
#define UNICOS
#endif
#if defined(__alpha)
#define DEC_ALPHA
#endif
#if defined(__hpux)
#define HP9000
#endif
/* VMS defines itself */

/* netCDF error toggle */
extern int ncopts;

/* wanker */
extern char *use_this_url_instead;

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

#include "mfhdf.h"
#include "mosaic.h"
#include "libhtmlw/HTML.h" /* for ImageInfo */
#ifdef HAVE_DTM
#include "netdata.h"
#endif
#include <X11/Xlib.h>

/* make all in-lined images smaller than hdfImageSize X hdfImageSize */
#define hdfImageSize get_pref_int(eHDF_MAX_IMAGE_DIMENSION)

/* limits on datasets and attribute counts to send into brief mode */
#define MAX_DATASET_DISPLAY   get_pref_int(eHDF_MAX_DISPLAYED_DATASETS)
#define MAX_ATTRIBUTE_DISPLAY get_pref_int(eHDF_MAX_DISPLAYED_ATTRIBUTES)

/* if is HDF Power User then drop most of the supporting text */
#define POWER_USER            get_pref_boolean(eHDF_POWER_USER)

#define SHOW_URL              FALSE

static FILE * fp;
static int    brief;
static char * my_url;

#ifdef CHOUCK
/* forward decls */
void hdfStartImage
    PROTO(());
void hdfEndImage
    PROTO(());
ImageInfo * hdfFetchImage
    PROTO((intn *bg));
void hdfXsds
    PROTO((char *name, int32 rank));
void hdfXvgroup
    PROTO((char *name, char *class, int32 count));
#endif /* CHOUCK */

/* return a string description of the number type */
char *
#ifdef PROTOTYPE
get_type(int32 nt)
#else
get_type(nt)
int32 nt;
#endif
{

    switch(nt) {

    case DFNT_CHAR   : return("8-bit characters");

    case DFNT_INT8   : return("signed 8-bit integers");
    case DFNT_UINT8  : return("unsigned 8-bit integers");
    case DFNT_INT16  : return("signed 16-bit integers");
    case DFNT_UINT16 : return("unsigned 8-bit integers");
    case DFNT_INT32  : return("signed 32-bit integers");
    case DFNT_UINT32 : return("unsigned 32-bit integers");

    case DFNT_FLOAT32  : return("32-bit floating point numbers");
    case DFNT_FLOAT64  : return("64-bit floating point numbers");
 
    default : return("unknown number type");

    }

} /* get_type */


/* return a string dump of buffer */
/* FREES the incoming buffer !!!!!!!!!!!!!!!!!! */
char *
#ifdef PROTOTYPE
buffer_to_string(char * tbuff, int32 nt, int32 count)
#else
buffer_to_string(tbuff, nt, count)
char * tbuff;
int32 nt;
int32 count;
#endif
{
    intn i;
    char * buffer;
    
    if(nt == DFNT_CHAR) {
        tbuff[count] = '\0';
        return tbuff;
    }

    buffer = (char *) HDgetspace(80 * count);
    if(buffer == NULL) return NULL;

    buffer[0] = '\0';

    switch(nt) {
    case DFNT_INT8   : 
    case DFNT_UINT8  : 
        sprintf(buffer, "%d", ((int8 *)tbuff)[0]);
        for(i = 1; i < count; i++)
            sprintf(buffer, "%s, %d", buffer, ((int8 *)tbuff)[i]);
        break;
    case DFNT_INT16   : 
    case DFNT_UINT16  : 
        sprintf(buffer, "%d", ((int16 *)tbuff)[0]);
        for(i = 1; i < count; i++)
            sprintf(buffer, "%s, %d", buffer, ((int16 *)tbuff)[i]);
        break;
    case DFNT_INT32   : 
    case DFNT_UINT32  : 
        sprintf(buffer, "%d", ((int32 *)tbuff)[0]);
        for(i = 1; i < count; i++)
            sprintf(buffer, "%s, %d", buffer, ((int32 *)tbuff)[i]);
        break;
    case DFNT_FLOAT32 : 
        sprintf(buffer, "%f", ((float32 *)tbuff)[0]);
        for(i = 1; i < count; i++)
            sprintf(buffer, "%s, %f", buffer, ((float32 *)tbuff)[i]);
        break;
    case DFNT_FLOAT64 : 
        sprintf(buffer, "%f", ((float64 *)tbuff)[0]);        
        for(i = 1; i < count; i++)
            sprintf(buffer, "%s, %f", buffer, ((float64 *)tbuff)[i]);
        break;
    }

    HDfreespace((void *)tbuff);
    return buffer;

} /* buffer_to_string */


/* return a string of the contents of an attribute */
char *
#ifdef PROTOTYPE
get_attribute(int32 id, int32 num, int32 nt, int32 count)
#else
get_attribute(id, num, nt, count)
int32 id;
int32 nt;
int32 count;
int32 num;
#endif
{

    char *tbuff;
    int32 dsize;
    int32 status;

    dsize = DFKNTsize(nt);

    if(dsize < 1) return NULL;

    tbuff = HDgetspace(dsize * (count + 1));
    if(tbuff == NULL) return NULL;

    status = SDreadattr(id, num, tbuff);
    if(status == FAIL) return NULL;

    return(buffer_to_string(tbuff, nt, count));

}


/* 

  Print out the info for data sets in the file.  This code is based on
  the multi-file interface of HDF 3.3 so it is able to decode netCDF files
  as well a strict HDF files

*/
#ifdef PROTOTYPE
do_sds(char *fname)
#else
do_sds(fname)
char *fname; 
#endif
{

    int32 fid;
    int32 sds;
    int32 dsets, nattr, nattrs, status;
    int32 i, j;
    char name[512];
    int32 nt, dimsizes[50], rank;
    intn  count;
    /* SWP */
    int k;
    char attr_name[MAX_NC_NAME];
    int32 ant;
    int32 alen;
    char attr_data[MAX_NC_NAME];
    char longname[MAX_NC_NAME];

    fid = SDstart(fname, DFACC_RDONLY);
    if(fid == FAIL) return;

    status = SDfileinfo(fid, &dsets, &nattr);
    if(status == FAIL) return;

    if(dsets + nattr < 1) return;

    fprintf(fp, "<H2>Datasets</H2>\n");

    /* see if enough to send us into brief mode */
    if(dsets > MAX_DATASET_DISPLAY)
        brief = TRUE;

    fprintf(fp, "There are %d dataset%s and %d global attribute%s in this file.<P>\n", 
           dsets, (dsets == 1 ? "" : "s"), 
           nattr, (nattr == 1 ? "" : "s"));

    if(dsets) {
        fprintf(fp, "Available datasets :\n");
        fprintf(fp, "<UL>\n");
        for(i = 0; i < dsets; i++) {
            sds = SDselect(fid, i);
            if(sds == FAIL) return;

            status = SDgetinfo(sds, name, &rank, dimsizes, &nt, &nattrs);
            if(status == FAIL) return;

	    /* SWP -- 09/06/95 */
            if (get_pref_boolean(eHDFLONGNAME)) {
		strcpy(longname,"No Label/Longname");
		for (k=0; k<nattrs; k++) {
			status = SDattrinfo(sds, k, attr_name, &ant, &alen);
			if (status == FAIL) {
			}
			else if (!strcmp(attr_name,"long_name")) {
				status = SDreadattr(sds, k, (VOIDP)(&attr_data[0]));
				if (status == FAIL) {
				}
				else {
					attr_data[alen]='\0';
					strcpy(longname,attr_data);
					break;
				}
			}
		}

		if(POWER_USER)
			fprintf(fp, "<LI> <A NAME=\"DataSet%d\"><B>%s(%s)</B></A> rank %d : [", 
				i, name, longname, rank);
		else
			fprintf(fp, "<LI> Dataset <A NAME=\"DataSet%d\"><B>%s(%s)</B></A> has rank %d with dimensions [", 
				i, name, longname, rank);
	    }
	    else {
		if(POWER_USER)
			fprintf(fp, "<LI> <A NAME=\"DataSet%d\"><B>%s</B></A> rank %d : [", 
				i, name, rank);
		else
			fprintf(fp, "<LI> Dataset <A NAME=\"DataSet%d\"><B>%s</B></A> has rank %d with dimensions [", 
				i, name, rank);
	    }

#ifdef CHOUCK
            /* put it in the picture */
            hdfXsds(name, rank);
#endif /* CHOUCK */

            for(j = 0; j < rank; j++) 
                if(j == 0)
                    fprintf(fp, "%d", dimsizes[j]);
                else
                    fprintf(fp, ", %d", dimsizes[j]);

            fprintf(fp, "]");
            
            if(POWER_USER)
                fprintf(fp, "; %s.\n", get_type(nt));
            else
                fprintf(fp, ".  The dataset is composed of %s.\n", get_type(nt));

#ifdef HAVE_DTM
        if (mo_dtm_out_active_p () && ((rank == 2) || (rank == 3)))
          fprintf (fp, "(To broadcast this dataset over DTM, click <A HREF=\"#hdfdtm;tag=%d,ref=%d\">here</A>.)\n", 
                   (int32) DFTAG_NDG, i);
#endif
            if(nattrs) {
                if(brief || nattrs > MAX_ATTRIBUTE_DISPLAY) {
                    
                    fprintf(fp, "There %s %d <A HREF=\"#hdfref;tag=%d,ref=%d\">attribute%s</A>.\n", 
                            (nattrs == 1 ? "is" : "are"), nattrs, (int32) DFTAG_NDG, i, 
                            (nattrs == 1 ? "" : "s"));

                } else {
                    
                    if(POWER_USER)
                        fprintf(fp, "Attributes :\n");
                    else
                        fprintf(fp, "It has the following attributes :\n");

                    fprintf(fp, "<UL>\n");
                    for(j = 0; j < nattrs; j++) {
                        char *valstr;
                        status = SDattrinfo(sds, j, name, &nt, &count);
                        if(status == FAIL) return;
                        
                        valstr = get_attribute(sds, j, nt, count);
                        if(valstr == NULL) continue;
                        
                        if(POWER_USER)
                            fprintf(fp, "<LI> <i>%s</i> : <B>%s</B>", name, valstr);
                        else
                            fprintf(fp, "<LI> Attribute <i>%s</i> has the value : <B>%s</B>", name, valstr);

                        HDfreespace((void *)valstr);
                        
                    }
                    fprintf(fp, "</UL>\n");
                }
            }

            SDendaccess(sds);
        }
        fprintf(fp, "</UL>\n");
    }


    if(nattr) {
        fprintf(fp, "Global attributes :\n");
        fprintf(fp, "<UL>\n");
        for(i = 0; i < nattr; i++) {

            char *valstr;

            status = SDattrinfo(fid, i, name, &nt, &count);
            if(status == FAIL) return;

            valstr = get_attribute(fid, i, nt, count);
            if(valstr == NULL) continue;

            if(POWER_USER)
                fprintf(fp, "<LI> <i>%s</i> : <B>%s</B>", name, valstr);
            else
                fprintf(fp, "<LI> Attribute <i>%s</i> has the value : <B>%s</B>", name, valstr);

            HDfreespace((void *)valstr);
 
        }
        fprintf(fp, "</UL>\n");
    }

    SDend(fid);

} /* do_sds */


/*
  
  If there are a lot of attributes we need to do them in a separate window
  
  That's what this function is for

*/
do_attributes(char *fname, int index) {
    
    char name[MAX_NC_NAME];
    int32 nattrs, nt, dims[MAX_VAR_DIMS], rank, status;
    int32 fid, sds;
    intn j, count;

    fid = SDstart(fname, DFACC_RDONLY);
    if(fid == FAIL) return;

    sds = SDselect(fid, index);
    if(sds == FAIL) return;

    status = SDgetinfo(sds, name, &rank, dims, &nt, &nattrs);
    if(status == FAIL) return;

    if(nattrs) {

        fprintf(fp, "Dataset <B>%s</B> has the following attributes :\n", name);
        fprintf(fp, "<UL>\n");
        for(j = 0; j < nattrs; j++) {
            char *valstr;
            status = SDattrinfo(sds, j, name, &nt, &count);
            if(status == FAIL) return;
            
            valstr = get_attribute(sds, j, nt, count);
            if(valstr == NULL) continue;
            
            if(POWER_USER)
                fprintf(fp, "<LI> <i>%s</i> : <B>%s</B>", name, valstr);
            else
                fprintf(fp, "<LI> Attribute <i>%s</i> has the value : <B>%s</B>", name, valstr);

            HDfreespace((void *)valstr);
            
        }
        fprintf(fp, "</UL>\n");
    }
    
    SDend(fid);

} /* do_attributes */


/* 

  Print out info about file ids and descriptions

*/
#ifdef PROTOTYPE
do_fanns(int32 fid)
#else
do_fanns(fid)
int32 fid; 
#endif
{
    char *buffer;
    int32 status;
    int32 len;

    len = DFANgetfidlen(fid, 1);
    if(len > 0) {

        buffer = HDgetspace(len + 1);
        if(buffer == NULL) return;

        status = DFANgetfid(fid, buffer, len + 1, 1);
        if(status == FAIL) return;

        fprintf(fp, "This file has the following label: <B>%s</B><P>\n", buffer);

        HDfreespace((void *)buffer);

    }

    len = DFANgetfdslen(fid, 1);
    if(len > 0) {

        buffer = HDgetspace(len + 1);
        if(buffer == NULL) return;

        status = DFANgetfds(fid, buffer, len, 1);
        if(status == FAIL) return;

        buffer[len] = '\0';
        fprintf(fp, "Here is the file description: <BLOCKQUOTE> %s </BLOCKQUOTE>\n", buffer); 

        HDfreespace((void *)buffer);

    }

} /* do_fanns */


/*

  Print out labels and descriptions for this item

*/
#ifdef PROTOTYPE
print_desc(char *fname, uint16 tag, uint16 ref, char *name)
#else
print_desc(fname, tag, ref, name)
char *fname;
uint16 tag, ref;
char *name;
#endif
{

    int32 len, status;
    char *buffer;

    len = DFANgetlablen(fname, tag, ref);
    if(len > 0) {

        buffer = HDgetspace(len + 1);
        if(buffer == NULL) return;
        
            status = DFANgetlabel(fname, tag, ref, buffer, len + 1);
        if(status == SUCCEED) {
            name[0] = tolower(name[0]);

            if(POWER_USER)
                fprintf(fp, "Label : <B>%s</B> <P> \n", name, buffer);
            else
                fprintf(fp, "This %s was given the label : <B>%s</B> <P> \n", name, buffer);
        }
        HDfreespace((void *)buffer);
    }
    
    len = DFANgetdesclen(fname, tag, ref);
    if(len > 0) {
        
        buffer = HDgetspace(len + 1);
        if(buffer == NULL) return;
        
        status = DFANgetdesc(fname, tag, ref, buffer, len + 1);
        if(status == SUCCEED) {
            buffer[len] = '\0';
            name[0] = toupper(name[0]);
            fprintf(fp, "%s description : <BLOCKQUOTE> %s </BLOCKQUOTE>\n", name, buffer);
        }
        HDfreespace((void *)buffer);
    }
    
} /* print_desc */


/* 

  print out the info for RIGSs in the file 

*/
#ifdef PROTOTYPE
do_rigs(char *fname)
#else
do_rigs(fname)
char *fname; 
#endif
{
    int32 count, i, ref, len;
    int32 status, w, h;
    intn  ip;

    count = DFR8nimages(fname);

    if(count < 1) return;

    fprintf(fp, "<H2>Images</H2>\n");

    if(count == 1)
        fprintf(fp, "There is 1 image in this file :\n");
    else
        fprintf(fp, "There are %d images in this file :\n", count);
    
    fprintf(fp, "<UL>\n");

    for(i = 0; i < count; i++) {
        status = DFR8getdims(fname, &w, &h, &ip);
        if(status == FAIL) return;
        
        ref = DFR8lastref();
        if(ref == FAIL) return;

        if(POWER_USER) {
            fprintf(fp, "<LI> Image : <IMG SRC=\"#hdfref;tag=%d,ref=%d\"> [%d by %d]\n", 
                    (int32) DFTAG_RIG, ref, w, h);
            
            if(w > hdfImageSize || h > hdfImageSize)
                fprintf(fp, "  (subsampled)");
            
            if(ip) fprintf(fp, " has a palette\n");

            fprintf(fp, ".  ");

        } else {
            fprintf(fp, "<LI> This image : <IMG SRC=\"#hdfref;tag=%d,ref=%d\"> has dimensions %d by %d\n", 
                    (int32) DFTAG_RIG, ref, w, h);
            
            if(w > hdfImageSize || h > hdfImageSize)
                fprintf(fp, "  (the image has been subsampled for display)");
            fprintf(fp, ".  ");
            
            if(ip) fprintf(fp, "There is also a palette associated with this image.\n");
        }

#ifdef HAVE_DTM
        if (mo_dtm_out_active_p ())
          fprintf (fp, "(To broadcast this image over DTM, click <A HREF=\"#hdfdtm;tag=%d,ref=%d\">here</A>.)\n", (int32) DFTAG_RIG, ref);
#endif

        print_desc(fname, DFTAG_RIG, ref, strdup ("image"));

    }

    fprintf(fp, "</UL>\n");

} /* do_rigs */


/* 

  print out the info for Palettes in the file 

*/
#ifdef PROTOTYPE
do_pals(char *fname)
#else
do_pals(fname)
char *fname; 
#endif
{
    int32 count, i, ref, len;
    int32 status, w, h, ip;
    char pal[768];

    count = DFPnpals(fname);

    if(count < 1) return;

    fprintf(fp, "<H2>Palettes</H2>\n");

    if(count == 1)
        fprintf(fp, "There is 1 palette in this file :\n");
    else
        fprintf(fp, "There are %d palettes in this file :\n", count);
    
    fprintf(fp, "<UL>\n");

    for(i = 0; i < count; i++) {
        status = DFPgetpal(fname, pal);
        if(status == FAIL) return;
        
        ref = DFPlastref();
        if(ref < 1) return;

        fprintf(fp, "<LI> Here's what the palette looks like : <IMG SRC=\"#hdfref;tag=%d,ref=%d\">\n", (int32) DFTAG_IP8, ref);

#ifdef HAVE_DTM
        if (mo_dtm_out_active_p ())
          fprintf (fp, "(To broadcast this palette over DTM, click <A HREF=\"#hdfdtm;tag=%d,ref=%d\">here</A>.)\n", (int32) DFTAG_IP8, ref);
#endif

        print_desc(fname, DFTAG_IP8, ref, strdup ("palette"));

    }

    fprintf(fp, "</UL>\n");

} /* do_pals */


/* ------------------------------ dump_vdata ------------------------------ */
#ifdef PROTOTYPE
dump_vdata(int32 fid, int32 ref)
#else
dump_vdata(fid, ref)
int32 fid;
int32 ref;
#endif
{
    int32  vd;
    char   name[VSNAMELENMAX + 1], class[VSNAMELENMAX + 1];
    char   fields[(FIELDNAMELENMAX + 1) * VSFIELDMAX];
    char   padded_fields[(FIELDNAMELENMAX + 2) * VSFIELDMAX];
    int32  intr, sz, cnt;
    int32  nfields, single;
    char * p, * q;
    
    vd = VSattach(fid, ref, "r");
    if(vd == FAIL) return;
    
    VSinquire(vd, &cnt, &intr, fields, &sz, name);
    VSgetclass(vd, class);
    
    nfields = VFnfields(vd);
    single = FALSE;
    if((cnt == 1) && (nfields == 1))
        single = TRUE;

    if(name[0]  == '\0') sprintf(name, "[no name value]");
    
    fprintf(fp, "<LI> Vdata <B>%s</B>", name);
    if(class[0])
        fprintf(fp, " of class <B>%s</B>", class);
    if(!single)
        fprintf(fp," contains %d record%s", cnt, (cnt == 1 ? "" : "s"));
    fprintf(fp,".  \n");
    
    for(p = fields, q = padded_fields; *p; p++, q++) {
        *q = *p;
        if(*q == ',') *(++q) = ' ';
    }
    *q = '\0';
    
    fprintf(fp, "This Vdata contains the field%s <B>%s</B>\n",
            (nfields == 1 ? "" : "s"), padded_fields);
    
    /* if there is a single field with one value print out its values */
    if(single) {
        
        uint8 *tbuff;
        
        /* Attempted bugfix -- added 1 -- marca, 10:52pm sep 23. */
        tbuff = (uint8 *)HDgetspace(VFfieldisize(vd, 0) + 1);
        if(tbuff == NULL)
            return;
        
        VSsetfields(vd, fields);
        VSread(vd, tbuff, 1, 0);
        tbuff = buffer_to_string(tbuff, VFfieldtype(vd, 0), VFfieldorder(vd, 0));

        if(tbuff) {
            fprintf(fp, ": %s", tbuff);
            HDfreespace((void *)tbuff);
        }
            
    }

    fprintf(fp, ".  \n");

    VSdetach(vd);

} /* dump_vdata */


/* ------------------------------- dump_vg -------------------------------- */
/*

  Print out some HTML for the given Vgroup.

  If X is TRUE then call the functions to register the Vgroup in the
  hacked up image

*/
#ifdef PROTOTYPE
dump_vg(int32 fid, int32 ref, int32 X)
#else
dump_vg(fid, ref, X)
int32 fid;
int32 ref;
int32 X;
#endif
{
    int32 vg, count;
    char name[VSNAMELENMAX + 1], class[VSNAMELENMAX + 1];

    vg = Vattach(fid, ref, "r");
    if(vg == FAIL) return;
    
    Vgetname(vg, name);
    Vgetclass(vg, class);
    count = Vntagrefs(vg);
    
    if(name[0]  == '\0') sprintf(name, "[no name value]");
    if(class[0] == '\0') sprintf(class, "[no class value]");
    
    if(count) 
        fprintf(fp, "<LI> Vgroup <A HREF=\"#hdfref;tag=%d,ref=%d\">%s</A> of class <B>%s</B> has %d element%s.\n",
                DFTAG_VG, ref, name, class, count, (count == 1 ? "" : "s"));
                
    else
        fprintf(fp, "<LI> Vgroup %s of class <B>%s</B> is empty.\n",
                name, class);
    
#ifdef CHOUCK
    if(X)
        hdfXvgroup(name, class, count);
#endif /* CHOUCK */

    Vdetach(vg);

} /* dump_vg */


/* 
  print out the info for Vgroups in the file

  If where is -1 print out info for the Vgroups we can see at the top level
  (i.e. Vgroups which are not contained within other Vgroups)

  If where is a positive number assume that is the ref of a Vgroup and report
  everything that is contained within that Vgroup.

  Because of poor code design, a lot of functionality will be duplicated when
  reporting on the contents of a Vgroup.  Oh well.  Will give someone something
  to do later....

  I guess we know why this used to be called spank.c
*/
#ifdef PROTOTYPE
do_vgs(int32 fid, char *fname, int32 where)
#else
do_vgs(fid, fname, where)
int32  fid;
char * fname;
int32  where;
#endif
{

    int32 curr_ref, i, count, cnt;
    int32 curr_vg, vg, vg1, vd;
    int32 tag, ref, myref, intr, sz;
    int any;
    char name[VSNAMELENMAX], class[VSNAMELENMAX];
    
    if(where == -1) {
        
        if(Hnumber(fid, DFTAG_VG) < 1) return;
        
        any = FALSE;

        /* looking for top level VGroups */
        curr_ref = -1;
        while((curr_ref = Vgetid(fid, curr_ref)) != FAIL) {
            int top_level = TRUE;

            /* look through all other VGroups */
            myref = -1;
            while(top_level && ((myref = Vgetid(fid, myref)) != FAIL)) {
                if(myref == curr_ref) continue;
                vg = Vattach(fid, myref, "r");
                if(vg == FAIL) continue;

                count = Vntagrefs(vg);
                for(i = 0; i < count; i++) {
                    Vgettagref(vg, i, &tag, &ref);
                    if(tag == (int32) DFTAG_VG && ref == curr_ref) top_level = FALSE;

                }

                Vdetach(vg);
            }

            if(top_level) {

                if(any == FALSE) {
                    fprintf(fp, "<H2>Vgroups</H2>\n");
                    fprintf(fp, "The following Vgroups are visible at this level of the file.\n");
                    fprintf(fp, "<UL>\n");
                    any = TRUE;
                }

                dump_vg(fid, curr_ref, TRUE);

            }

        }

        if(any) fprintf(fp, "</UL>\n");

    } else {
        /* looking for stuff inside a given VGroup */
        
        vg = Vattach(fid, where, "r");
        if(vg == FAIL) return;

        Vgetname(vg, name);
        Vgetclass(vg, class);
        count = Vntagrefs(vg);
        
        /* set up title and stuff */
        fprintf(fp, "<TITLE>Vgroup: %s</TITLE>\n", name);
        fprintf(fp, "<H1>Vgroup %s</H1>\n", name);
        
        fprintf(fp, "This Vgroup is named %s and is of class <B>%s</B>.\n", name, class);

        if(count == 0) {
            Vdetach(vg);
            return;
        }

        if(count == 1)
          fprintf(fp, "There is %d element in Vgroup %s : \n", count, name);
        else
          fprintf(fp, "There are %d elements in Vgroup %s : \n", count, name);
        
        fprintf(fp, "<UL>\n");

        for(i = 0; i < count; i++) {
            Vgettagref(vg, i, &tag, &ref);
            switch((uint16) tag) {

            case DFTAG_VG :
                dump_vg(fid, ref, FALSE);
                break;
            case DFTAG_VH :
                dump_vdata(fid, ref);
                break;
            case DFTAG_RIG :
            case DFTAG_RI8 :
                {
                    int32 count, i, len;
                    int32 status, w, h;
                    intn  ip;
                    
                    DFR8readref(fname, (uint16) ref);

                    status = DFR8getdims(fname, &w, &h, &ip);
                    if(status == FAIL) return;
        
                    if(POWER_USER) {
                        fprintf(fp, "<LI> Image : <IMG SRC=\"#hdfref;tag=%d,ref=%d\"> [%d by %d]\n", 
                                (int32) DFTAG_RIG, ref, w, h);
                        
                        if(w > hdfImageSize || h > hdfImageSize)
                            fprintf(fp, "  (subsampled)");
                        
                        if(ip) fprintf(fp, " has a palette\n");
                        
                        fprintf(fp, ".  ");
                        
                    } else {
                        fprintf(fp, "<LI> Nested Raster Image : <IMG SRC=\"#hdfref;tag=%d,ref=%d\"> has dimensions %d by %d\n", 
                                (int32) DFTAG_RIG, ref, w, h);
                        
                        if(w > hdfImageSize || h > hdfImageSize)
                            fprintf(fp, "  (the image has been subsampled for display)");
                        fprintf(fp, ".  ");
                        
                        if(ip) fprintf(fp, "There is also a palette associated with this image.\n");
                    }
                    
#ifdef HAVE_DTM
                    if (mo_dtm_out_active_p ())
                        fprintf (fp, "(To broadcast this image over DTM, click <A HREF=\"#hdfdtm;tag=%d,ref=%d\">here</A>.)\n", (int32) DFTAG_RIG, ref);
#endif
                    
                    print_desc(fname, DFTAG_RIG, ref, strdup ("image"));
                    
                }
            break;
            
            default:
                if(HDgettagsname((uint16)tag))
                    fprintf(fp, "<LI> HDF object : %s (Ref = %d)\n", HDgettagsname((uint16)tag), ref);
                else
                    fprintf(fp, "<LI> HDF object : Tag = %d; Ref = %d\n", tag, ref);
                break;
            }
        }

        fprintf(fp, "</UL>\n");
        Vdetach(vg);

    }

} /* do_vgs */


/*
  
  Process Vdatas that are visible from the top level 
    (i.e. they are not inside any Vgroups)

*/
#ifdef PROTOTYPE
do_lone_vds(int32 fid)
#else
do_lone_vds(fid)
int32 fid;
#endif
{

    int32 count, vd;
    int32 i, *ids, status;

    count = VSlone(fid, NULL, 0);
    if(count < 1) return;
    
    ids = (int32 *) HDgetspace(sizeof(int32) * count);
    if(ids == NULL) return;

    status = VSlone(fid, ids, count);
    if(status != count) return;

    fprintf(fp, "<H2>Vdatas</H2>\n");
    fprintf(fp, "There are %d Vdatas visible at this level of the file.\n", count);
    fprintf(fp, "<UL>\n");
    
    for(i = 0; i < count; i++) 
        dump_vdata(fid, ids[i]);
    
    fprintf(fp, "<UL>\n");
    HDfreespace((void *)ids);

} /* do_lone_vds */


char *
#ifdef PROTOTYPE
hdfGrokFile(char *fname, char *realname)
#else
hdfGrokFile(fname, realname)
char *fname;
char *realname;
#endif
{
    char *tmp = (char *)mo_tmpnam((char *)0);
    char *data;
    int32 len, fid;

    /* make sure we don't crash on invalid open */
    ncopts = 0;

    fp = fopen(tmp, "w");
    if(!fp) return(NULL);

    fprintf (fp, "<TITLE>Scientific Data Brows-o-rama</TITLE>\n");
    fprintf (fp, "<H1>Scientific Data Brows-o-rama</H1>\n");

    if(SHOW_URL)
        fprintf (fp, "This file was accessed as URL <code>%s</code> <p>\n", realname);

    if(my_url)
        free(my_url);

    my_url = strdup(realname);

    /* start doing the full description */
    brief = FALSE;

#ifdef CHOUCK
    fprintf(fp, "<P> File dump : <A HREF=\"#hdfref;fileImageHit\"> <IMG SRC=\"#hdfref;fileImage\" ISMAP></A><P>\n");

    hdfStartImage();
#endif

    fid = Hopen(fname, DFACC_RDONLY, 0);

    if(fid != FAIL) {

        Vstart(fid);
        
        /* Do file annotations */
        do_fanns(fid);

    }

    /* Do SDSs */
    do_sds(fname);

    if(fid != FAIL) {

        /* Do RIGs */
        do_rigs(fname);
        
        /* Do Pals */
        do_pals(fname);
        
        /* Do Vgroups */
        do_vgs(fid, fname, -1);
        
        /* Do lone Vdatas */
        do_lone_vds(fid);
        
        /* close the file */
        Vend(fid);
        Hclose(fid);
        
    }

#ifdef CHOUCK
    hdfEndImage();
#endif

    fclose(fp);

    fp = fopen(tmp, "r");
    if (fp != NULL) {

        /*
         * Find the length of the file the really cheesy way!
         */
        fseek(fp, 0L, 0);
        fseek(fp, 0L, 2);
        len = ftell(fp);
        fseek(fp, 0L, 0);
        data = HDgetspace((len + 1) * sizeof(unsigned char));
        if (data == NULL)
            return(NULL);

        len = fread(data, sizeof(char), len, fp);
        fclose(fp);
        remove(tmp);
        data[len] = '\0';
        return(data);

    }

    return NULL;
    
} /* hdfGrokFile */


char *
#ifdef PROTOTYPE
hdfGrokReference(char *fname, char *ref, char *realname)
#else
hdfGrokReference(fname, ref, realname)
char *fname;
char *ref;
char *realname;
#endif
{
    int32 t, r;
    int32 x, y;

    char *tmp = (char *)mo_tmpnam((char *) 0);
    char *data;
    int32 len, fid;

    fp = fopen(tmp, "w");
    if(!fp) return(NULL);

    fprintf (fp, "<TITLE>HDF Brows-o-rama Internal</TITLE>\n");
    fprintf (fp, "<H1>HDF Brows-o-rama Internal</H1>\n");

    if(SHOW_URL)
        fprintf (fp, "This file was accessed as URL <code>%s</code> <p>\n", realname);

    if(sscanf(ref, "tag=%d,ref=%d", &t, &r) == 2) {
        
        switch((uint16) t) {
        case DFTAG_VG  : 
            fid = Hopen(fname, DFACC_RDONLY, 0);
            if(fid == FAIL) return(NULL);
            
            Vstart(fid);

            do_vgs(fid, fname, r);

            Vend(fid);
            Hclose(fid);
            break;
        case DFTAG_NDG :
            do_attributes(fname, r);
           break;
        default:
            fprintf (fp, "<H2>Sorry, Bad Tag</H2> We're sorry, but objects of type %s are not currently recognized.<P>\n",
                     HDgettagsname((uint16)t));
            break;
        }

    } else {
        if(sscanf(ref, "fileImageHit?%d,%d", &x, &y) == 2) {
            char *buf;
            fprintf(fp, "<H2>File Image Hit</H2>\n");
            fprintf(fp, "We're sorry, but we really have no clue what is at location %d %d\n", x, y); 

            buf = malloc(100);
            sprintf(buf, "#DataSet%d", 2);

            use_this_url_instead = buf;

        } else {
            fprintf(fp, "<H2>Sorry, Bad Reference</H2>\n");
            fprintf(fp, "We're sorry, but reference <I>%s</I> is bad and we can't figure out what to do about it.<P>\n", 
                    ref);
        }
    }        
    
    fclose(fp);

    fp = fopen(tmp, "r");
    if (fp != NULL) {

        /*
         * Find the length of the file the really cheesy way!
         */
        fseek(fp, 0L, 0);
        fseek(fp, 0L, 2);
        len = ftell(fp);
        fseek(fp, 0L, 0);
        data = HDgetspace((len + 1) * sizeof(unsigned char));
        if (data == NULL)
            return(NULL);

        len = fread(data, sizeof(char), len, fp);
        fclose(fp);
        remove(tmp);
        data[len] = '\0';
        return(data);

    }
    
    return NULL;
    
} /* hdfGrokReference */


/*

ImageInfo *hdfGetImage(char *filename, char *reference, int subsample)
  returns an ImageInfo struct corresponding to the named reference,
  as in hdfGrokReference.  If subsample == TRUE then subsample the 
  image to fit within a hdfImageSize X hdfImageSize box

Return the backgroud pixel's index in bg only if the file Image

  return NULL on failure

*/

ImageInfo *
#ifdef PROTOTYPE
hdfGetImage(char *filename, char *reference, intn subsample, intn *bg)
#else
hdfGetImage(filename, reference, subsample, bg)
char *filename;
char *reference;
intn subsample;
intn *bg;
#endif
{

    ImageInfo *Image;
    int32 tag, ref, i, j;
    int32 status, w, h;
    intn  isp;
    char pal[768];
    
    Image = NULL;

#ifdef CHOUCK
    if(!strcmp(reference, "fileImage")) {
        use_this_url_instead = NULL;
        return ((ImageInfo *) hdfFetchImage(bg));
    }
#endif

    if(sscanf(reference, "tag=%d,ref=%d", &tag, &ref) == 2) {

#ifndef DISABLE_TRACE
	if (srcTrace) {
		fprintf (stderr, "[hdfGrokImage] Got tag %d, ref %d\n", tag, ref);
	}
#endif

        if((uint16)tag == DFTAG_RIG || (uint16)tag == DFTAG_RI8) {
            
            status = DFR8readref(filename, (uint16) ref);
            if(status == FAIL) return NULL;

            status = DFR8getdims(filename, &w, &h, &isp);
            if(status == FAIL) return NULL;

            /* allocate the basic structure */
            Image = (ImageInfo *) HDgetspace(sizeof(ImageInfo));
            if(Image == NULL) return NULL;

            /* allocate space for the data */
            Image->image_data = (unsigned char *) HDgetspace(w * h * sizeof(unsigned char));
            if(Image->image_data == NULL) return NULL;

            /* allocate the palette space */
            Image->reds = (int *) HDgetspace(256 * sizeof(int));
            if(Image->reds == NULL) return NULL;
            Image->greens = (int *) HDgetspace(256 * sizeof(int));
            if(Image->greens == NULL) return NULL;
            Image->blues = (int *) HDgetspace(256 * sizeof(int));
            if(Image->blues == NULL) return NULL;

            /* fill in the static fields */
            Image->ismap = FALSE;
            Image->width = w;
            Image->height = h;
            Image->num_colors = 256;
            Image->image = NULL;

            /* read the image */
            status = DFR8getimage(filename, Image->image_data, w, h, pal);
            if(status == FAIL) return NULL;

            /* set the palette */
            if(isp) {
                /* move the palette over into the fields */
                for(i = 0; i < 256; i++) {
                    Image->reds  [i] = (int)pal[i * 3]     << 8;
                    Image->greens[i] = (int)pal[i * 3 + 1] << 8;
                    Image->blues [i] = (int)pal[i * 3 + 2] << 8;
                }
            } else {
                /* create a fake palette */
                for(i = 0; i < 256; i++) {
                    Image->reds  [i] = i << 8;
                    Image->greens[i] = i << 8;
                    Image->blues [i] = i << 8;
                }
            }

            if(subsample) {
                int max   = (h > w ? h : w);
                int skip;

#ifndef DISABLE_TRACE
		if (srcTrace) {
			fprintf (stderr, "[***] max '%d'\n", max);
			fprintf (stderr, "[***] hdfImageSize '%d'\n", hdfImageSize);
		}
#endif

                skip = max / hdfImageSize;

                if(skip) {
                    
                    /* allocate space for the data */
                    int i, j;
                    int cnt = 0;
                    unsigned char * newSpace = 
                        (unsigned char *) HDgetspace(max * max * sizeof(unsigned char));
                    if(newSpace == NULL) return NULL;

                    skip++;
                    for(j = 0; j < h; j += skip)
                        for(i = 0; i < w; i += skip) 
                            newSpace[cnt++] = Image->image_data[i + j * w];
                            
                    HDfreespace((void *)(Image->image_data));
                    Image->image_data = newSpace;
                    Image->height = h / skip;
                    Image->width  = w / skip;
                    if(w % skip) Image->width++;

                }

            }

            return Image;

        }

        if((uint16)tag == DFTAG_IP8) {

            status = DFPreadref(filename, (uint16) ref);
            if(status == FAIL) return NULL;

             /* allocate the basic structure */
            Image = (ImageInfo *) HDgetspace(sizeof(ImageInfo));
            if(Image == NULL) return NULL;

            /* allocate space for the data */
            h = 30;
            Image->image_data = (unsigned char *) HDgetspace(768 * h * sizeof(unsigned char));
            if(Image->image_data == NULL) return NULL;

            /* allocate the palette space */
            Image->reds = (int *) HDgetspace(256 * sizeof(int));
            if(Image->reds == NULL) return NULL;
            Image->greens = (int *) HDgetspace(256 * sizeof(int));
            if(Image->greens == NULL) return NULL;
            Image->blues = (int *) HDgetspace(256 * sizeof(int));
            if(Image->blues == NULL) return NULL;

            /* fill in the static fields */
            Image->ismap = FALSE;
            Image->width = 256;
            Image->height = h;
            Image->num_colors = 256;
            Image->image = NULL;

            /* read the image */
            status = DFPgetpal(filename, pal);
            if(status == FAIL) return NULL;

            /* move the palette over into the fields */
            for(i = 0; i < 256; i++) {
                Image->reds  [i] = (int)pal[i * 3]     << 8;
                Image->greens[i] = (int)pal[i * 3 + 1] << 8;
                Image->blues [i] = (int)pal[i * 3 + 2] << 8;
                for(j = 0; j < h; j++) 
                    Image->image_data[i + j * 256] = i;
            }
                
        }

        return Image;

    }

    
} /* hdfGetImage */


/*

ImageInfo *hdfGrokImage(char *filename, char *reference)
  returns an ImageInfo struct corresponding to the named reference,
  as in hdfGrokReference.

  return NULL on failure

*/
ImageInfo *
#ifdef PROTOTYPE
hdfGrokImage(char *filename, char *reference, int *bg)
#else
hdfGrokImage(filename, reference, bg)
char *filename;
char *reference;
int  *bg;
#endif
{

    return (hdfGetImage(filename, reference, TRUE, bg));

} /* hdfGrokImage */

  
#ifdef HAVE_DTM
/* Send the referenced image over DTM.  This is bad architecture.
   But what else is new? */
void
#ifdef PROTOTYPE
hdfDtmThang(char *filename, char *reference)
#else
hdfDtmThang(filename, reference)
char *filename;
char *reference;
#endif
{
    ImageInfo *img;
    int32 tag, ref;
    intn  foo;

    /* Check for presence of DTM output port. */
    if (!mo_dtm_out_active_p ())
        return;
    
    if(sscanf(reference, "tag=%d,ref=%d", &tag, &ref) == 2) {

        if((uint16)tag == DFTAG_RIG) {
            /* Using a cached image is not an option since we quantized. */
            img = hdfGetImage(filename, reference, FALSE, &foo);
            if (img) {
                mo_dtm_send_image ((void *)img);
                /* We specify copy_internally in args to NetSendRaster8Group,
                   so we can free everything here now. */
                free (img->image_data);
                free (img->reds);
                free (img->greens);
                free (img->blues);
                free (img);
            }
            return;
        }
        
        if((uint16)tag == DFTAG_IP8) {

            img = hdfGrokImage(filename, reference, NULL);
            if(img) {
                mo_dtm_send_palette ((void *)img);
                /* We specify copy_internally in args to NetSendRaster8Group,
                   so we can free everything here now. */
                free (img->image_data);
                free (img->reds);
                free (img->greens);
                free (img->blues);
                free (img);

            }

        }

        if((uint16)tag == DFTAG_NDG) {
            int32 start[10], end[10];
            int32 fid, sds, nt, nattr, size, i, tmp;
            char name[512];
            Data *d;

            fid = SDstart(filename, DFACC_RDONLY);
            if(fid == FAIL) return;

            sds = SDselect(fid, (int32) ref);
            if(sds == FAIL) return;
            
            /* davet what the hell is this about ???? */
            if (!(d = DataNew())) 
                return;
            d->entity = ENT_Internal;
            d->dot = DOT_Array;
            
            /* get all basic meta-data */
            sprintf(name, "(no name)");
            SDgetinfo(sds, name, &(d->rank), d->dim, &nt, &nattr);

            if((d->rank > 3) || (d->rank < 2)) return;
            
            switch(nt) {
            case DFNT_INT8:
            case DFNT_UINT8:
                d->dost = DOST_Char;
                break;
            case DFNT_INT16:
            case DFNT_UINT16:
                d->dost = DOST_Int16;
                break;
            case DFNT_INT32:
            case DFNT_UINT32:
                d->dost = DOST_Int32;
                break;
            case DFNT_FLOAT32:
                d->dost = DOST_Float;
                break;
            case DFNT_FLOAT64:
                d->dost = DOST_Double;
                break;
            }
       
            /* set up the region we want to read */
            for(i = 0; i < d->rank; i++) {
                start[i] = 0;
                end[i]  = d->dim[i];
            }
            
            /* figger out how much space */
            for(i = 0, size = 1; i < d->rank; i++)
                size *= d->dim[i];
            
            /* allocate storage to store the raw numbers */
            if (!(d->data = (VOIDP) HDgetspace(size * DFKNTsize(nt))))
                return;

            /* read that crazy data */
            SDreaddata(sds, start, NULL, end, d->data);
            
            /* set the name */
            if (d->label = (char *) MALLOC(strlen(name)+1)){
                strcpy(d->label,name);
            }

            SDendaccess(sds);
            SDend(fid);
            
            /* swap numbers because Collage was written by a bunch of idiots */
            for(i = 0; i < ( d->rank / 2 ); i++) {
                tmp = d->dim[i];
                d->dim[i] = d->dim[d->rank - i - 1];
                d->dim[d->rank - i - 1] = tmp;
            }
            
            mo_dtm_send_dataset(d);

            /* free it */
            HDfreespace((void *)(d->label));
            HDfreespace((void *)(d->data));
            HDfreespace((void *)d);

        }

    }

    return;

}
#endif

#endif /* HAVE_HDF */

#ifdef CHOUCK

#define ImSize 512

Display *dsp;
Pixmap scrPix;
GC scrGC;
ImageInfo *img;
XFontStruct *myFont;

extern mo_window *current_win;
Widget view;

int32 sds, images, vgroups, vdatas;

void
hdfStartImage()
{
    int ret;
    Cardinal argcnt;
    Arg arg[5];
    XmFontList font_list = (XmFontList)NULL;
    XmFontContext font_context;
    XmStringCharSet charset;

    /*
     * Keep track of the window Widget
     */
    view = current_win->scrolled_win;

    /* 
     * Remember the display 
     */
    dsp = XtDisplay(view);

    /*
     * Create the Graphics Context
     */
    scrGC = XCreateGC(dsp, XtWindow(view), 0, NULL);
    
    /*
     * Create a ImSizexImSize pixmap to draw in, and clear it to the background
     */
    scrPix = XCreatePixmap(dsp, XtWindow(view), ImSize, ImSize,
                           DefaultDepth(dsp, DefaultScreen(dsp)));
    XSetForeground(dsp, scrGC, WhitePixel(dsp, DefaultScreen(dsp)));
    XFillRectangle(dsp, scrPix, scrGC, 0, 0, ImSize, ImSize);
    
    /*
     * Prepare to draw our stuff in Black on White
     */
    XSetForeground(dsp, scrGC, BlackPixel(dsp, DefaultScreen(dsp)));
    XSetBackground(dsp, scrGC, WhitePixel(dsp, DefaultScreen(dsp)));

    sds = images = vgroups = vdatas = 0;

#ifdef FONTS_SUCK
    /*
     *  Get a FontStruct so everything will look pretty
     */
    argcnt = 0;
    XtSetArg(arg[argcnt], XmNfontList, &font_list); argcnt++;
    XtGetValues(view, arg, argcnt);
    
    if (font_list == (XmFontList)NULL) 
        return;
    
    ret = XmFontListInitFontContext(&font_context, font_list);
    if(ret == False)
        return;

    ret = XmFontListGetNextFont(font_context, &charset, &myFont);
    if (ret == False) {
        return;
    } else {
        XmFontListFreeFontContext(font_context);
        free((char *)charset);
    }

#else

    myFont = XLoadQueryFont(dsp, "fixed");

#endif

    
} /* hdfStartImage */


    /*
     * Place drawing calls here.
     * XDrawPoint, XDrawLine, XDrawArc, etc.
     */
 
#define sdsDepth      5
#define sdsWidth     35
#define sdsSep       20
#define sdsY         50
#define vgroupY     200
#define vgroupWidth  50

/* ------------------------------- hdfXsds -------------------------------- */
/*
 * Draw an SDS into our image
 */
void
hdfXsds(char *name, int32 rank)
{
    char   rankString[10];
    char   nameString[512];
    XPoint Points[10];
    int x, y;
    int strX, strY;
    int height, width, dir, asc, des;
    int sz;
    XCharStruct overall;

    y = sdsY;   /* all SDSs on same line */
    x = sdsSep + (sdsWidth + sdsSep) * sds;

    /* 
     * Make sure our box is big enough by seeing how big the rank
     * string is going to be
     */
    sprintf(rankString, "%d", rank);
    XTextExtents(myFont, rankString, strlen(rankString), &dir, &asc, &des, &overall);
    height = asc + des;
    width = overall.width;

    /* see how big we need to make it */
    sz = sdsWidth;
    if(height > sz) sz = height;
    if(width  > sz) sz = width;

    if(x + sz + sdsDepth > ImSize) return;

    /* draw the cube */
    Points[0].x = (short) x;
    Points[0].y = (short) y;
    Points[1].x = (short) x + sdsDepth;
    Points[1].y = (short) y - sdsDepth;
    Points[2].x = (short) x + sdsDepth + sz;
    Points[2].y = (short) y - sdsDepth;
    Points[3].x = (short) x + sz;
    Points[3].y = (short) y;
    Points[4].x = (short) x + sdsDepth + sz;
    Points[4].y = (short) y - sdsDepth;
    Points[5].x = (short) x + sdsDepth + sz;
    Points[5].y = (short) y - sdsDepth + sz ;
    Points[6].x = (short) x + sz;
    Points[6].y = (short) y + sz;
    
    XDrawLines(dsp, scrPix, scrGC, Points, 7, CoordModeOrigin);
    XDrawRectangle(dsp, scrPix, scrGC, x, y, sz, sz);

    /* draw the rank now centered in the box */
    strY = (sz + height) / 2 + y;
    strX = (sz - width) / 2 + x;
    XDrawString(dsp, scrPix, scrGC, strX, strY, rankString, strlen(rankString));

    /* draw the name */
    sprintf(nameString, "%s", name);
    if(strlen(nameString) > 4)
        nameString[4] = '\0';
    XTextExtents(myFont, nameString, strlen(nameString), &dir, &asc, &des, &overall);
    strY = sz + height + y;
    strX = (sz - overall.width) / 2 + x;
    XDrawString(dsp, scrPix, scrGC, strX, strY, nameString, strlen(nameString));

    /* remember where we put it for future reference */


    /* increment counter */
    sds++;

} /* hdfXsds */


/* ------------------------------ hdfXvgroup ------------------------------ */
/*

  Draw a Vgroup object into our image
 
*/
void
hdfXvgroup(char *name, char *class, int32 count) 
{
    char   classString[512];
    char   nameString[512];
    XPoint Points[10];
    int x, y;
    int strX, strY;
    int height, width, dir, asc, des;
    int sz;
    XCharStruct overall;

    y = vgroupY;   /* all Vgroups on same line */
    x = sdsSep + (vgroupWidth + sdsSep) * vgroups;

    /* 
     * Make sure our box is big enough by seeing how big the name
     * string is going to be
     */
    sprintf(nameString, "%s", name);
    if(strlen(nameString) > 7)
        nameString[7] = '\0';
       
    XTextExtents(myFont, nameString, strlen(nameString), &dir, &asc, &des, &overall);
    height = asc + des;
    width = overall.width;

    /* see how big we need to make it */
    sz = vgroupWidth;
    if(height > sz) sz = height;
    if(width / 2 > sz) sz = width / 2;

    if(x + sz > ImSize) return;

    /* draw a circle */
    XDrawArc(dsp, scrPix, scrGC, x, y, 2 * sz, sz, 0, 360 * 64);

    /* put the name in */
    strY = sz / 2 + y;
    strX = (sz * 2 - overall.width) / 2 + x;
    XDrawString(dsp, scrPix, scrGC, strX, strY, nameString, strlen(nameString));

    /* write the count out */
    sprintf(nameString, "(%d)", count);
    XTextExtents(myFont, nameString, strlen(nameString), &dir, &asc, &des, &overall);
    strY = sz / 2 + height + y;
    strX = (sz * 2 - overall.width) / 2 + x;
    XDrawString(dsp, scrPix, scrGC, strX, strY, nameString, strlen(nameString));


    /* added another one to the image */
    vgroups++;

} /* hdfXvgroup */


/*
 *  All done building the image
 *  Build the pixmap and clean up
 */
void
hdfEndImage()
{

    int x, y;
    unsigned long pix;
    XImage *scrImage;
    unsigned char *dptr;


/*     XtVaGetValues (view, XtNforeground, &fg_pixel, XtNbackground, &bg_pixel, NULL); */
    

    
    scrImage = XGetImage(dsp, scrPix, 0, 0, ImSize, ImSize,
                         AllPlanes, ZPixmap);
    XFreePixmap(dsp, scrPix);
    img = (ImageInfo *)malloc(sizeof(ImageInfo));
    img->width = ImSize;
    img->height = ImSize;
    img->image_data = (unsigned char *)malloc(ImSize * ImSize);
    img->image = NULL;
    dptr = img->image_data;
    for (y=0; y<ImSize; y++)
	{
            for (x=0; x<ImSize; x++)
		{
                    pix = XGetPixel(scrImage, x, y);
                    if (pix == BlackPixel(dsp, DefaultScreen(dsp)))
			{
                            *dptr = (unsigned char)1;
			}
                    else
			{
                            *dptr = (unsigned char)0;
			}
                    dptr++;
		}
	}
    img->num_colors = 256;
    img->reds = (int *)malloc(sizeof(int) * 256);
    img->greens = (int *)malloc(sizeof(int) * 256);
    img->blues = (int *)malloc(sizeof(int) * 256);
    img->reds[0] = 65535;
    img->greens[0] = 65535;
    img->blues[0] = 65535;
    {
      int i;
      for (i = 1; i < 256; i++)
        {
          img->reds[i] = 0;
          img->greens[i] = 0;
          img->blues[i] = 0;
        }
    }
    XDestroyImage(scrImage);

} /* hdfEndImage */


ImageInfo *
hdfFetchImage(intn *bg) 
{
    if(bg) *bg = 0;
    return (img);
} /* hdfFetchImage */

#endif
