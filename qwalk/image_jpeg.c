/*
    QShed <http://www.icculus.org/qshed>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* based on stuff from darkplaces */

#include <setjmp.h>
#include <stdio.h>

#include "global.h"
#include "image.h"

/* jboolean is unsigned char instead of int on Win32 */
#ifdef WIN32
typedef unsigned char jboolean;
#else
typedef int jboolean;
#endif

#define JFALSE 0
#define JTRUE 1

/*
=================================================================

  Minimal set of definitions from the JPEG lib

  WARNING: for a matter of simplicity, several pointer types are
  casted to "void*", and most enumerated values are not included

=================================================================
*/

typedef void *j_common_ptr;
typedef struct jpeg_compress_struct *j_compress_ptr;
typedef struct jpeg_decompress_struct *j_decompress_ptr;

#define JPEG_LIB_VERSION  62  /* Version 6b */

typedef enum
{
	JCS_UNKNOWN,
	JCS_GRAYSCALE,
	JCS_RGB,
	JCS_YCbCr,
	JCS_CMYK,
	JCS_YCCK
} J_COLOR_SPACE;
typedef enum {JPEG_DUMMY1} J_DCT_METHOD;
typedef enum {JPEG_DUMMY2} J_DITHER_MODE;
typedef unsigned int JDIMENSION;

#define JPOOL_PERMANENT	0	/* lasts until master record is destroyed */
#define JPOOL_IMAGE		1	/* lasts until done with image/datastream */

#define JPEG_EOI	0xD9  /* EOI marker code */

#define JMSG_STR_PARM_MAX  80

#define DCTSIZE2 64
#define NUM_QUANT_TBLS 4
#define NUM_HUFF_TBLS 4
#define NUM_ARITH_TBLS 16
#define MAX_COMPS_IN_SCAN 4
#define C_MAX_BLOCKS_IN_MCU 10
#define D_MAX_BLOCKS_IN_MCU 10

struct jpeg_memory_mgr
{
  void* (*alloc_small) (j_common_ptr cinfo, int pool_id, size_t sizeofobject);
  void (*_reserve_space_for_alloc_large) (void *dummy, ...);
  void (*_reserve_space_for_alloc_sarray) (void *dummy, ...);
  void (*_reserve_space_for_alloc_barray) (void *dummy, ...);
  void (*_reserve_space_for_request_virt_sarray) (void *dummy, ...);
  void (*_reserve_space_for_request_virt_barray) (void *dummy, ...);
  void (*_reserve_space_for_realize_virt_arrays) (void *dummy, ...);
  void (*_reserve_space_for_access_virt_sarray) (void *dummy, ...);
  void (*_reserve_space_for_access_virt_barray) (void *dummy, ...);
  void (*_reserve_space_for_free_pool) (void *dummy, ...);
  void (*_reserve_space_for_self_destruct) (void *dummy, ...);

  long max_memory_to_use;
  long max_alloc_chunk;
};

struct jpeg_error_mgr
{
	void (*error_exit) (j_common_ptr cinfo);
	void (*emit_message) (j_common_ptr cinfo, int msg_level);
	void (*output_message) (j_common_ptr cinfo);
	void (*format_message) (j_common_ptr cinfo, char * buffer);
	void (*reset_error_mgr) (j_common_ptr cinfo);
	int msg_code;
	union {
		int i[8];
		char s[JMSG_STR_PARM_MAX];
	} msg_parm;
	int trace_level;
	long num_warnings;
	const char * const * jpeg_message_table;
	int last_jpeg_message;
	const char * const * addon_message_table;
	int first_addon_message;
	int last_addon_message;
};

struct jpeg_source_mgr
{
	const unsigned char *next_input_byte;
	size_t bytes_in_buffer;

	void (*init_source) (j_decompress_ptr cinfo);
	jboolean (*fill_input_buffer) (j_decompress_ptr cinfo);
	void (*skip_input_data) (j_decompress_ptr cinfo, long num_bytes);
	jboolean (*resync_to_restart) (j_decompress_ptr cinfo, int desired);
	void (*term_source) (j_decompress_ptr cinfo);
};

typedef struct {
  /* These values are fixed over the whole image. */
  /* For compression, they must be supplied by parameter setup; */
  /* for decompression, they are read from the SOF marker. */
  int component_id;             /* identifier for this component (0..255) */
  int component_index;          /* its index in SOF or cinfo->comp_info[] */
  int h_samp_factor;            /* horizontal sampling factor (1..4) */
  int v_samp_factor;            /* vertical sampling factor (1..4) */
  int quant_tbl_no;             /* quantization table selector (0..3) */
  /* These values may vary between scans. */
  /* For compression, they must be supplied by parameter setup; */
  /* for decompression, they are read from the SOS marker. */
  /* The decompressor output side may not use these variables. */
  int dc_tbl_no;                /* DC entropy table selector (0..3) */
  int ac_tbl_no;                /* AC entropy table selector (0..3) */
  
  /* Remaining fields should be treated as private by applications. */
  
  /* These values are computed during compression or decompression startup: */
  /* Component's size in DCT blocks.
   * Any dummy blocks added to complete an MCU are not counted; therefore
   * these values do not depend on whether a scan is interleaved or not.
   */
  JDIMENSION width_in_blocks;
  JDIMENSION height_in_blocks;
  /* Size of a DCT block in samples.  Always DCTSIZE for compression.
   * For decompression this is the size of the output from one DCT block,
   * reflecting any scaling we choose to apply during the IDCT step.
   * Values of 1,2,4,8 are likely to be supported.  Note that different
   * components may receive different IDCT scalings.
   */
  int DCT_scaled_size;
  /* The downsampled dimensions are the component's actual, unpadded number
   * of samples at the main buffer (preprocessing/compression interface), thus
   * downsampled_width = ceil(image_width * Hi/Hmax)
   * and similarly for height.  For decompression, IDCT scaling is included, so
   * downsampled_width = ceil(image_width * Hi/Hmax * DCT_scaled_size/DCTSIZE)
   */
  JDIMENSION downsampled_width;  /* actual width in samples */
  JDIMENSION downsampled_height; /* actual height in samples */
  /* This flag is used only for decompression.  In cases where some of the
   * components will be ignored (eg grayscale output from YCbCr image),
   * we can skip most computations for the unused components.
   */
  jboolean component_needed;     /* do we need the value of this component? */

  /* These values are computed before starting a scan of the component. */
  /* The decompressor output side may not use these variables. */
  int MCU_width;                /* number of blocks per MCU, horizontally */
  int MCU_height;               /* number of blocks per MCU, vertically */
  int MCU_blocks;               /* MCU_width * MCU_height */
  int MCU_sample_width;         /* MCU width in samples, MCU_width*DCT_scaled_size */
  int last_col_width;           /* # of non-dummy blocks across in last MCU */
  int last_row_height;          /* # of non-dummy blocks down in last MCU */

  /* Saved quantization table for component; NULL if none yet saved.
   * See jdinput.c comments about the need for this information.
   * This field is currently used only for decompression.
   */
  void *quant_table;

  /* Private per-component storage for DCT or IDCT subsystem. */
  void * dct_table;
} jpeg_component_info;

struct jpeg_decompress_struct
{
	struct jpeg_error_mgr *err;		/* USED */
	struct jpeg_memory_mgr *mem;	/* USED */

	void *progress;
	void *client_data;
	jboolean is_decompressor;
	int global_state;

	struct jpeg_source_mgr *src;	/* USED */
	JDIMENSION image_width;			/* USED */
	JDIMENSION image_height;		/* USED */

	int num_components;
	J_COLOR_SPACE jpeg_color_space;
	J_COLOR_SPACE out_color_space;
	unsigned int scale_num, scale_denom;
	double output_gamma;
	jboolean buffered_image;
	jboolean raw_data_out;
	J_DCT_METHOD dct_method;
	jboolean do_fancy_upsampling;
	jboolean do_block_smoothing;
	jboolean quantize_colors;
	J_DITHER_MODE dither_mode;
	jboolean two_pass_quantize;
	int desired_number_of_colors;
	jboolean enable_1pass_quant;
	jboolean enable_external_quant;
	jboolean enable_2pass_quant;
	JDIMENSION output_width;

	JDIMENSION output_height;	/* USED */

	int out_color_components;

	int output_components;		/* USED */

	int rec_outbuf_height;
	int actual_number_of_colors;
	void *colormap;

	JDIMENSION output_scanline;	/* USED */

	int input_scan_number;
	JDIMENSION input_iMCU_row;
	int output_scan_number;
	JDIMENSION output_iMCU_row;
	int (*coef_bits)[DCTSIZE2];
	void *quant_tbl_ptrs[NUM_QUANT_TBLS];
	void *dc_huff_tbl_ptrs[NUM_HUFF_TBLS];
	void *ac_huff_tbl_ptrs[NUM_HUFF_TBLS];
	int data_precision;
	jpeg_component_info *comp_info;
	jboolean progressive_mode;
	jboolean arith_code;
	unsigned char arith_dc_L[NUM_ARITH_TBLS];
	unsigned char arith_dc_U[NUM_ARITH_TBLS];
	unsigned char arith_ac_K[NUM_ARITH_TBLS];
	unsigned int restart_interval;
	jboolean saw_JFIF_marker;
	unsigned char JFIF_major_version;
	unsigned char JFIF_minor_version;
	unsigned char density_unit;
	unsigned short X_density;
	unsigned short Y_density;
	jboolean saw_Adobe_marker;
	unsigned char Adobe_transform;
	jboolean CCIR601_sampling;
	void *marker_list;
	int max_h_samp_factor;
	int max_v_samp_factor;
	int min_DCT_scaled_size;
	JDIMENSION total_iMCU_rows;
	void *sample_range_limit;
	int comps_in_scan;
	jpeg_component_info *cur_comp_info[MAX_COMPS_IN_SCAN];
	JDIMENSION MCUs_per_row;
	JDIMENSION MCU_rows_in_scan;
	int blocks_in_MCU;
	int MCU_membership[D_MAX_BLOCKS_IN_MCU];
	int Ss, Se, Ah, Al;
	int unread_marker;
	void *master;
	void *main;
	void *coef;
	void *post;
	void *inputctl;
	void *marker;
	void *entropy;
	void *idct;
	void *upsample;
	void *cconvert;
	void *cquantize;
};


struct jpeg_compress_struct
{
	struct jpeg_error_mgr *err;
	struct jpeg_memory_mgr *mem;
	void *progress;
	void *client_data;
	jboolean is_decompressor;
	int global_state;

	void *dest;
	JDIMENSION image_width;
	JDIMENSION image_height;
	int input_components;
	J_COLOR_SPACE in_color_space;
	double input_gamma;
	int data_precision;

	int num_components;
	J_COLOR_SPACE jpeg_color_space;
	jpeg_component_info *comp_info;
	void *quant_tbl_ptrs[NUM_QUANT_TBLS];
	void *dc_huff_tbl_ptrs[NUM_HUFF_TBLS];
	void *ac_huff_tbl_ptrs[NUM_HUFF_TBLS];
	unsigned char arith_dc_L[NUM_ARITH_TBLS];
	unsigned char arith_dc_U[NUM_ARITH_TBLS];
	unsigned char arith_ac_K[NUM_ARITH_TBLS];

	int num_scans;
	const void *scan_info;
	jboolean raw_data_in;
	jboolean arith_code;
	jboolean optimize_coding;
	jboolean CCIR601_sampling;
	int smoothing_factor;
	J_DCT_METHOD dct_method;

	unsigned int restart_interval;
	int restart_in_rows;

	jboolean write_JFIF_header;
	unsigned char JFIF_major_version;
	unsigned char JFIF_minor_version;
	unsigned char density_unit;
	unsigned short X_density;
	unsigned short Y_density;
	jboolean write_Adobe_marker;
	JDIMENSION next_scanline;

	jboolean progressive_mode;
	int max_h_samp_factor;
	int max_v_samp_factor;
	JDIMENSION total_iMCU_rows;
	int comps_in_scan;
	jpeg_component_info *cur_comp_info[MAX_COMPS_IN_SCAN];
	JDIMENSION MCUs_per_row;
	JDIMENSION MCU_rows_in_scan;
	int blocks_in_MCU;
	int MCU_membership[C_MAX_BLOCKS_IN_MCU];
	int Ss, Se, Ah, Al;

	void *master;
	void *main;
	void *prep;
	void *coef;
	void *marker;
	void *cconvert;
	void *downsample;
	void *fdct;
	void *entropy;
	void *script_space;
	int script_space_size;
};

struct jpeg_destination_mgr
{
	unsigned char* next_output_byte;
	size_t free_in_buffer;

	void (*init_destination) (j_compress_ptr cinfo);
	jboolean (*empty_output_buffer) (j_compress_ptr cinfo);
	void (*term_destination) (j_compress_ptr cinfo);
};


/*
=================================================================

  DarkPlaces definitions

=================================================================
*/

#define qjpeg_create_compress(cinfo) \
	qjpeg_CreateCompress((cinfo), JPEG_LIB_VERSION, (size_t)sizeof(struct jpeg_compress_struct))
#define qjpeg_create_decompress(cinfo) \
	qjpeg_CreateDecompress((cinfo), JPEG_LIB_VERSION, (size_t)sizeof(struct jpeg_decompress_struct))

static void (*qjpeg_CreateCompress) (j_compress_ptr cinfo, int version, size_t structsize);
static void (*qjpeg_CreateDecompress) (j_decompress_ptr cinfo, int version, size_t structsize);
static void (*qjpeg_destroy_compress) (j_compress_ptr cinfo);
static void (*qjpeg_destroy_decompress) (j_decompress_ptr cinfo);
static void (*qjpeg_finish_compress) (j_compress_ptr cinfo);
static jboolean (*qjpeg_finish_decompress) (j_decompress_ptr cinfo);
static jboolean (*qjpeg_resync_to_restart) (j_decompress_ptr cinfo, int desired);
static int (*qjpeg_read_header) (j_decompress_ptr cinfo, jboolean require_image);
static JDIMENSION (*qjpeg_read_scanlines) (j_decompress_ptr cinfo, unsigned char **scanlines, JDIMENSION max_lines);
static void (*qjpeg_set_defaults) (j_compress_ptr cinfo);
static void (*qjpeg_set_quality) (j_compress_ptr cinfo, int quality, jboolean force_baseline);
static jboolean (*qjpeg_start_compress) (j_compress_ptr cinfo, jboolean write_all_tables);
static jboolean (*qjpeg_start_decompress) (j_decompress_ptr cinfo);
static struct jpeg_error_mgr *(*qjpeg_std_error) (struct jpeg_error_mgr *err);
static JDIMENSION (*qjpeg_write_scanlines) (j_compress_ptr cinfo, unsigned char **scanlines, JDIMENSION num_lines);

static dllfunction_t jpegfuncs[] =
{
	{ "jpeg_CreateCompress",     (void**)&qjpeg_CreateCompress },
	{ "jpeg_CreateDecompress",   (void**)&qjpeg_CreateDecompress },
	{ "jpeg_destroy_compress",   (void**)&qjpeg_destroy_compress },
	{ "jpeg_destroy_decompress", (void**)&qjpeg_destroy_decompress },
	{ "jpeg_finish_compress",    (void**)&qjpeg_finish_compress },
	{ "jpeg_finish_decompress",  (void**)&qjpeg_finish_decompress },
	{ "jpeg_resync_to_restart",  (void**)&qjpeg_resync_to_restart },
	{ "jpeg_read_header",        (void**)&qjpeg_read_header },
	{ "jpeg_read_scanlines",     (void**)&qjpeg_read_scanlines },
	{ "jpeg_set_defaults",       (void**)&qjpeg_set_defaults },
	{ "jpeg_set_quality",        (void**)&qjpeg_set_quality },
	{ "jpeg_start_compress",     (void**)&qjpeg_start_compress },
	{ "jpeg_start_decompress",   (void**)&qjpeg_start_decompress },
	{ "jpeg_std_error",          (void**)&qjpeg_std_error },
	{ "jpeg_write_scanlines",    (void**)&qjpeg_write_scanlines },
	{ NULL, NULL }
};

static dllhandle_t jpegdll = 0;

static unsigned char jpeg_eoi_marker[2] = { 0xFF, JPEG_EOI };
static jmp_buf error_in_jpeg;
/*static bool_t jpeg_toolarge;*/

static void jpeg_closelibrary(void)
{
	unloadlibrary(&jpegdll);
	jpegdll = NULL;

	printf("Closed jpeg library.\n");
}

static bool_t jpeg_openlibrary(void)
{
	if (jpegdll)
		return true;

#if defined(WIN32)
	if (!loadlibrary("libjpeg.dll", &jpegdll, jpegfuncs))
		return false;
#elif defined(MACOSX)
	if (!loadlibrary("libjpeg.62.dylib", &jpegdll, jpegfuncs))
		return false;
#else
	if (!loadlibrary("libjpeg.so.62", &jpegdll, jpegfuncs) &&
	    !loadlibrary("libjpeg.so", &jpegdll, jpegfuncs))
		return false;
#endif

	add_atexit_event(jpeg_closelibrary);

	printf("Opened jpeg library.\n");
	return true;
}

/*
=================================================================

	JPEG decompression

=================================================================
*/

static void JPEG_Noop(j_decompress_ptr cinfo) {}

static jboolean JPEG_FillInputBuffer(j_decompress_ptr cinfo)
{
/* Insert a fake EOI marker */
	cinfo->src->next_input_byte = jpeg_eoi_marker;
	cinfo->src->bytes_in_buffer = 2;

	return JTRUE;
}

static void JPEG_SkipInputData(j_decompress_ptr cinfo, long num_bytes)
{
	if (cinfo->src->bytes_in_buffer <= (unsigned long)num_bytes)
	{
		cinfo->src->bytes_in_buffer = 0;
		return;
	}

	cinfo->src->next_input_byte += num_bytes;
	cinfo->src->bytes_in_buffer -= num_bytes;
}

static void JPEG_MemSrc(j_decompress_ptr cinfo, const unsigned char *buffer, size_t filesize)
{
	cinfo->src = (struct jpeg_source_mgr*)cinfo->mem->alloc_small((j_common_ptr)cinfo, JPOOL_PERMANENT, sizeof(struct jpeg_source_mgr));

	cinfo->src->next_input_byte = buffer;
	cinfo->src->bytes_in_buffer = filesize;

	cinfo->src->init_source = JPEG_Noop;
	cinfo->src->fill_input_buffer = JPEG_FillInputBuffer;
	cinfo->src->skip_input_data = JPEG_SkipInputData;
	cinfo->src->resync_to_restart = qjpeg_resync_to_restart; /* use the default method */
	cinfo->src->term_source = JPEG_Noop;
}

static void JPEG_ErrorExit(j_common_ptr cinfo)
{
/* FIXME - figure out how to return this instead of printing it */
	((struct jpeg_decompress_struct*)cinfo)->err->output_message(cinfo);
	longjmp(error_in_jpeg, 1);
}

image_rgba_t *image_jpg_load(mem_pool_t *pool, void *filedata, size_t filesize, char **out_error)
{
	image_rgba_t *image = NULL;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	unsigned char *scanline = NULL;
	unsigned int line;
	int width, height;

	if (!jpeg_openlibrary())
	{
		if (out_error)
			*out_error = msprintf("failed to load libjpeg.");
		return NULL;
	}

	cinfo.err = qjpeg_std_error(&jerr);
	qjpeg_create_decompress(&cinfo);
	if (setjmp(error_in_jpeg))
		goto error_caught;
	cinfo.err = qjpeg_std_error(&jerr);
	cinfo.err->error_exit = JPEG_ErrorExit;
	JPEG_MemSrc(&cinfo, filedata, filesize);
	qjpeg_read_header(&cinfo, JTRUE);
	qjpeg_start_decompress(&cinfo);

	width = cinfo.image_width;
	height = cinfo.image_height;

	if (width > 4096 || height > 4096 || width <= 0 || height <= 0)
	{
		if (out_error)
			*out_error = msprintf("jpeg: bad dimensions");
		return NULL;
	}

	image = image_alloc(pool, width, height);
	scanline = (unsigned char*)qmalloc(width * cinfo.output_components);
	if (!image || !scanline)
	{
		if (image)
			image_free(&image);
		if (scanline)
			qfree(scanline);

		if (out_error)
			*out_error = msprintf("jpeg: out of memory");
		qjpeg_finish_decompress(&cinfo);
		qjpeg_destroy_decompress(&cinfo);
		return NULL;
	}

/* decompress the image, line by line */
	line = 0;
	while (cinfo.output_scanline < cinfo.output_height)
	{
		unsigned char *buffer_ptr;
		int ind;

		qjpeg_read_scanlines(&cinfo, &scanline, 1);

	/* convert the image to RGBA */
		switch (cinfo.output_components)
		{
		/* RGB images */
		case 3:
			buffer_ptr = &image->pixels[width * line * 4];
			for (ind = 0; ind < width * 3; ind += 3, buffer_ptr += 4)
			{
				buffer_ptr[0] = scanline[ind];
				buffer_ptr[1] = scanline[ind+1];
				buffer_ptr[2] = scanline[ind+2];
				buffer_ptr[3] = 255;
			}
			break;

		/* greyscale images (default to it, just in case) */
		case 1:
		default:
			buffer_ptr = &image->pixels[width * line * 4];
			for (ind = 0; ind < width; ind++, buffer_ptr += 4)
			{
				buffer_ptr[0] = scanline[ind];
				buffer_ptr[1] = scanline[ind];
				buffer_ptr[2] = scanline[ind];
				buffer_ptr[3] = 255;
			}
			break;
		}

		line++;
	}
	qfree(scanline);

	qjpeg_finish_decompress(&cinfo);
	qjpeg_destroy_decompress(&cinfo);

	return image;

error_caught:
	if (scanline)
		qfree(scanline);
	if (image)
		image_free(&image);

	qjpeg_destroy_decompress(&cinfo);

	if (out_error)
		*out_error = msprintf("jpeg: an error occurred");
	return NULL;
}
