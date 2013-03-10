#include "config.h"
#ifdef HAVE_JPEG
#include <stdio.h>
#include <X11/Intrinsic.h>

#include "mosaic.h"
#include "jpeglib.h"
#include "readJPEG.h"
#include <setjmp.h>

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */
  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;


static void
my_error_exit (j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr) cinfo->err;
#ifndef DISABLE_TRACE
	if (srcTrace) {
		fprintf(stderr,"Error reading JPEG image: ");
		(*cinfo->err->output_message) (cinfo);
	}
#endif
	longjmp(myerr->setjmp_buffer, 1);
}



unsigned char *
ReadJPEG(FILE *infile,int *width, int *height, XColor *colrs)
{
struct jpeg_decompress_struct cinfo;
struct my_error_mgr jerr;
unsigned char *retBuffer=0;	/* Output image buffer */
unsigned char *r;
JSAMPROW buffer[1];		/* row pointer array for read_scanlines */
int row_stride;		/* physical row width in output buffer */
int i;

#ifndef DISABLE_TRACE
	if (srcTrace) {
		fprintf(stderr,"ReadJPEG(): I've been called\n");
	}
#endif

	/* We set up the normal JPEG error routines,
		then override error_exit. */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	/* Establish the setjmp return context for my_error_exit to use. */

	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error. */
    		jpeg_destroy_decompress(&cinfo);
		fclose(infile);

		if (retBuffer) {
			free(retBuffer);
			}
		return 0;
		}

	jpeg_create_decompress(&cinfo);

	jpeg_stdio_src(&cinfo, infile);

	(void) jpeg_read_header(&cinfo, TRUE);

	/* We can ignore the return value from jpeg_read_header since
	*   (a) suspension is not possible with the stdio data source, and
	*   (b) we passed TRUE to reject a tables-only JPEG file as an error.
	* See libjpeg.doc for more info.
	*/

  	cinfo.quantize_colors = TRUE;
	/*cinfo.desired_number_of_colors = 50;*/
	cinfo.desired_number_of_colors = get_pref_int(eCOLORS_PER_INLINED_IMAGE);
	cinfo.two_pass_quantize = TRUE;

	jpeg_start_decompress(&cinfo);

	if (!(retBuffer = (unsigned char *) malloc(cinfo.output_width
			* cinfo.output_height * cinfo.output_components))) {
		jpeg_destroy_decompress(&cinfo);
#ifndef DISABLE_TRACE
		if (srcTrace) {
			fprintf(stderr,"Couldn't create space for JPEG read\n");
		}
#endif

		return(0);
		}
#ifndef DISABLE_TRACE
	if (srcTrace) {
		fprintf(stderr,"buffer size is width=%d x height=%d x depth=%d\n",
		       cinfo.output_width , cinfo.output_height ,
		       cinfo.output_components);
	}
#endif

	r = retBuffer;
	row_stride = cinfo.output_width * cinfo.output_components;
	while (cinfo.output_scanline < cinfo.output_height) {
		buffer[0] = r;
		(void) jpeg_read_scanlines(&cinfo, buffer, 1);
		r += row_stride;
  		}

	*width =  cinfo.output_width;
	*height =  cinfo.output_height;

	/* set up X colormap */
	if (cinfo.out_color_components  == 3) {

#ifndef DISABLE_TRACE
		if (srcTrace) {
			fprintf(stderr,"cinfo.actual_number_of_colors=%d\n",cinfo.actual_number_of_colors);
			fprintf(stderr,"colrs[0].red=%d colrs[99].red=%d\n",colrs[0].red,colrs[99].red);
			fprintf(stderr,"cinfo.colormap[0][0]=%d\n",cinfo.colormap[0][0]);
			{char dummy[80]; fprintf(stderr,"RETURN\n"); fgets(dummy, sizeof(dummy), stdin);}
		}
#endif

		for (i=0; i < cinfo.actual_number_of_colors; i++) {
			colrs[i].red = cinfo.colormap[0][i] << 8;
			colrs[i].green = cinfo.colormap[1][i] << 8;
			colrs[i].blue = cinfo.colormap[2][i] << 8;
			colrs[i].pixel = i;
			colrs[i].flags = DoRed|DoGreen|DoBlue;
			}
		}
	else {
		for (i=0; i < cinfo.actual_number_of_colors; i++) {
			colrs[i].red = colrs[i].green =
				colrs[i].blue = cinfo.colormap[0][i] << 8;
			colrs[i].pixel = i;
			colrs[i].flags = DoRed|DoGreen|DoBlue;
			}
		}


  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  return retBuffer;
}

#endif
