/* Lasem
 *
 * Copyright © 2004 Caleb Moore
 * Copyright © 2012 Emmanuel Pacaud
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors:
 *	Caleb Moore <c.moore@student.unsw.edu.au>
 * 	Emmanuel Pacaud <emmanuel@gnome.org>
 */

#include <lsmsvgfiltersurface.h>
#include <lsmsvgenums.h>
#include <lsmutils.h>
#include <math.h>
#include <string.h>

struct _LsmSvgFilterSurface {
	char *name;
	cairo_surface_t *surface;
	LsmBox subregion;

	gint ref_count;
};

LsmSvgFilterSurface *
lsm_svg_filter_surface_new (const char *name, unsigned int width, unsigned int height, const LsmBox *subregion)
{
	LsmSvgFilterSurface *filter_surface;
	cairo_surface_t *surface;

	surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);

	filter_surface = lsm_svg_filter_surface_new_with_content (name, surface, subregion);

	cairo_surface_destroy (surface);

	return filter_surface;
}

LsmSvgFilterSurface *
lsm_svg_filter_surface_new_with_content (const char *name, cairo_surface_t *surface, const LsmBox *subregion)
{
	LsmSvgFilterSurface *filter_surface;
	LsmBox null_subregion = {0, 0, 0, 0};

	if (surface == NULL ||
	    cairo_surface_get_type (surface) != CAIRO_SURFACE_TYPE_IMAGE ||
	    cairo_image_surface_get_format (surface) != CAIRO_FORMAT_ARGB32) {
		surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 0, 0);
		subregion = &null_subregion;
	} else if (subregion == NULL)
		subregion = &null_subregion;

	cairo_surface_reference (surface);

	filter_surface = g_new (LsmSvgFilterSurface, 1);
	filter_surface->name = g_strdup (name);
	filter_surface->subregion = *subregion;
	filter_surface->surface  = surface;
	filter_surface->ref_count = 1;

	return filter_surface;
}

LsmSvgFilterSurface *
lsm_svg_filter_surface_new_similar (const char *name, LsmSvgFilterSurface *model, const LsmBox *subregion)
{
	if (model == NULL)
		return lsm_svg_filter_surface_new (name, 0, 0, subregion);

	return lsm_svg_filter_surface_new (name,
				       cairo_image_surface_get_width (model->surface),
				       cairo_image_surface_get_height (model->surface),
				       subregion != NULL ? subregion : &model->subregion);
}

const char *
lsm_svg_filter_surface_get_name (LsmSvgFilterSurface *surface)
{
	g_return_val_if_fail (surface != NULL, NULL);

	return surface->name;
}

cairo_surface_t *
lsm_svg_filter_surface_get_cairo_surface (LsmSvgFilterSurface *surface)
{
	g_return_val_if_fail (surface != NULL, NULL);

	return surface->surface;
}

const LsmBox *
lsm_svg_filter_surface_get_subregion (LsmSvgFilterSurface *surface)
{
	static const LsmBox null_subregion = {0, 0, 0, 0};

	if (surface == NULL)
		return &null_subregion;

	return &surface->subregion;
}

LsmSvgFilterSurface *
lsm_svg_filter_surface_ref (LsmSvgFilterSurface *filter_surface)
{
	g_return_val_if_fail (filter_surface != NULL, NULL);

	g_atomic_int_inc (&filter_surface->ref_count);

	return filter_surface;
}

void
lsm_svg_filter_surface_unref (LsmSvgFilterSurface *filter_surface)
{
	g_return_if_fail (filter_surface != NULL);
	g_return_if_fail (filter_surface->ref_count > 0);

	if (g_atomic_int_dec_and_test (&filter_surface->ref_count)) {
		cairo_surface_destroy (filter_surface->surface);
		g_free (filter_surface->name);
		g_free (filter_surface);
	}
}

G_DEFINE_BOXED_TYPE (LsmSvgFilterSurface, lsm_svg_filter_surface, lsm_svg_filter_surface_ref, lsm_svg_filter_surface_unref)

static void
box_blur (unsigned char *input_pixels,
	  unsigned char *output_pixels,
	  guchar * intermediate,
	  gint kw, gint kh,
	  int x1,
	  int y1,
	  int x2,
	  int y2,
	  int rowstride)
{
	gint ch;
	gint x, y;
	gint sum;

	if (kw > x2 - x1)
		kw = x2 - x1;

	if (kh > y2 - y1)
		kh = y2 - y1;


	if (kw >= 1) {
		for (ch = 0; ch < 4; ch++) {
			for (y = y1; y < y2; y++) {
				sum = 0;
				for (x = x1; x < x1 + kw; x++) {
					sum += (intermediate[x % kw] = input_pixels[4 * x + y * rowstride + ch]);

					if (x - kw / 2 >= 0 && x - kw / 2 < x2)
						output_pixels[4 * (x - kw / 2) + y * rowstride + ch] = sum / kw;
				}
				for (x = x1 + kw; x < x2; x++) {
					sum -= intermediate[x % kw];
					sum += (intermediate[x % kw] = input_pixels[4 * x + y * rowstride + ch]);
					output_pixels[4 * (x - kw / 2) + y * rowstride + ch] = sum / kw;
				}
				for (x = x2; x < x2 + kw; x++) {
					sum -= intermediate[x % kw];

					if (x - kw / 2 >= 0 && x - kw / 2 < x2)
						output_pixels[4 * (x - kw / 2) + y * rowstride + ch] = sum / kw;
				}
			}
		}
		input_pixels = output_pixels;
	}

	if (kh >= 1) {
		for (ch = 0; ch < 4; ch++) {
			for (x = x1; x < x2; x++) {
				sum = 0;

				for (y = y1; y < y1 + kh; y++) {
					sum += (intermediate[y % kh] = input_pixels[4 * x + y * rowstride + ch]);

					if (y - kh / 2 >= 0 && y - kh / 2 < y2)
						output_pixels[4 * x + (y - kh / 2) * rowstride + ch] = sum / kh;
				}
				for (; y < y2; y++) {
					sum -= intermediate[y % kh];
					sum += (intermediate[y % kh] = input_pixels[4 * x + y * rowstride + ch]);
					output_pixels[4 * x + (y - kh / 2) * rowstride + ch] = sum / kh;
				}
				for (; y < y2 + kh; y++) {
					sum -= intermediate[y % kh];

					if (y - kh / 2 >= 0 && y - kh / 2 < y2)
						output_pixels[4 * x + (y - kh / 2) * rowstride + ch] = sum / kh;
				}
			}
		}
	}
}

void
lsm_svg_filter_surface_fast_blur (LsmSvgFilterSurface *input,
				  LsmSvgFilterSurface *output,
				  double sx, double sy)
{
	int kx, ky;
	unsigned char *intermediate;
	int rowstride;
	int x1, y1, x2, y2;
	unsigned char *input_pixels;
	unsigned char *output_pixels;
	int width, height;
	cairo_surface_t *blur_surface;
	gboolean do_clip = FALSE;

	g_return_if_fail (input != NULL);
	g_return_if_fail (output != NULL);

	cairo_surface_flush (input->surface);

	/* Original intermediate surface size calculation was:
	 *
	 * kx = floor (sx * 3 * sqrt (2 * M_PI) / 4 + 0.5);
	 * ky = floor (sy * 3 * sqrt (2 * M_PI) / 4 + 0.5);
	 *
	 * Which has the drawback to give a surface with even dimensions, leading
	 * to a blur with a one pixel offset.
	 * The new dimensions are now always odd.
	 *
	 * 0.94 = 3.0 * sqrt(2*M_PI) / 8.0
	 */

	kx = round (sx * 0.94 - 0.5) * 2.0 + 1.5;
	ky = round (sy * 0.94 - 0.5) * 2.0 + 1.5;

	if (kx < 1 && ky < 1)
		return;

	rowstride = cairo_image_surface_get_stride (input->surface);

	width = cairo_image_surface_get_width (input->surface);
	height = cairo_image_surface_get_height (input->surface);

	if (input->subregion.x < output->subregion.x ||
	    input->subregion.y < output->subregion.y ||
	    input->subregion.width > output->subregion.width ||
	    input->subregion.height > output->subregion.height) {
		do_clip = TRUE;	
		blur_surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
	} else
		blur_surface = output->surface;

	if (width != cairo_image_surface_get_width (output->surface) ||
	    height != cairo_image_surface_get_height (output->surface))
		return;

	x1 = output->subregion.x - kx;
	y1 = output->subregion.y - ky;
	x2 = output->subregion.width + output->subregion.x + kx;
	y2 = output->subregion.height + output->subregion.y + ky;
	x1 = CLAMP (x1, 0, width);
	y1 = CLAMP (y1, 0, height);
	x2 = CLAMP (x2, x1, width);
	y2 = CLAMP (y2, y1, height);

	intermediate = g_new (guchar, MAX (kx, ky));

	input_pixels = cairo_image_surface_get_data (input->surface);
	output_pixels = cairo_image_surface_get_data (blur_surface);

	box_blur (input_pixels, output_pixels, intermediate, kx, ky, x1, y1, x2, y2, rowstride);
	box_blur (output_pixels, output_pixels, intermediate, kx, ky, x1, y1, x2, y2, rowstride);
	box_blur (output_pixels, output_pixels, intermediate, kx, ky, x1, y1, x2, y2, rowstride);

	g_free (intermediate);

	cairo_surface_mark_dirty (blur_surface);

	if (do_clip) {
		cairo_t *cairo;

		cairo = cairo_create (output->surface);
		cairo_rectangle (cairo,
				 output->subregion.x, output->subregion.y,
				 output->subregion.width, output->subregion.height);
		cairo_clip (cairo);
		cairo_set_source_surface (cairo, blur_surface, 0, 0);
		cairo_paint (cairo);
		cairo_surface_destroy (blur_surface);
		cairo_destroy (cairo);
	}
}

void
lsm_svg_filter_surface_flood (LsmSvgFilterSurface *surface,
			      double red,
			      double green,
			      double blue,
			      double opacity)
{
	cairo_t *cairo;

	g_return_if_fail (surface != NULL);

	cairo = cairo_create (surface->surface);
	cairo_rectangle (cairo, surface->subregion.x, surface->subregion.y, surface->subregion.width, surface->subregion.height);
	cairo_clip (cairo);
	cairo_set_source_rgba (cairo, red, green, blue, opacity);
	cairo_paint (cairo);
}

void
lsm_svg_filter_surface_blend (LsmSvgFilterSurface *input_1,
			      LsmSvgFilterSurface *input_2,
			      LsmSvgFilterSurface *output,
			      int blending_mode)
{
	cairo_t *cairo;
	cairo_operator_t op;

	g_return_if_fail (input_1 != NULL);
	g_return_if_fail (input_2 != NULL);
	g_return_if_fail (output != NULL);

	switch (blending_mode) {
		case LSM_SVG_BLENDING_MODE_MULTIPLY:
			op = CAIRO_OPERATOR_MULTIPLY;
			break;
		case LSM_SVG_BLENDING_MODE_SCREEN:
			op = CAIRO_OPERATOR_SCREEN;
			break;
		case LSM_SVG_BLENDING_MODE_DARKEN:
			op = CAIRO_OPERATOR_DARKEN;
			break;
		case LSM_SVG_BLENDING_MODE_LIGHTEN:
			op = CAIRO_OPERATOR_LIGHTEN;
			break;
		case LSM_SVG_BLENDING_MODE_IN:
			op = CAIRO_OPERATOR_IN;
			break;
		case LSM_SVG_BLENDING_MODE_OUT:
			op = CAIRO_OPERATOR_OUT;
			break;
		case LSM_SVG_BLENDING_MODE_ATOP:
			op = CAIRO_OPERATOR_ATOP;
			break;
		case LSM_SVG_BLENDING_MODE_XOR:
			op = CAIRO_OPERATOR_XOR;
			break;
		default:
			op = CAIRO_OPERATOR_OVER;
			break;
	}

	cairo = cairo_create (output->surface);
	cairo_rectangle (cairo, output->subregion.x, output->subregion.y, output->subregion.width, output->subregion.height);
	cairo_clip (cairo);
	cairo_set_source_surface (cairo, input_2->surface, 0, 0);
	cairo_paint (cairo);
	cairo_set_source_surface (cairo, input_1->surface, 0, 0);
	cairo_set_operator (cairo, op);
	cairo_paint (cairo);
	cairo_destroy (cairo);
}

void
lsm_svg_filter_surface_offset (LsmSvgFilterSurface *input,
			       LsmSvgFilterSurface *output,
			       int dx, int dy)
{
	cairo_t *cairo;

	g_return_if_fail (input != NULL);
	g_return_if_fail (output != NULL);

	cairo = cairo_create (output->surface);
	cairo_rectangle (cairo, output->subregion.x, output->subregion.y, output->subregion.width, output->subregion.height);
	cairo_clip (cairo);
	cairo_set_source_surface (cairo, input->surface, dx, dy);
	cairo_paint (cairo);
	cairo_destroy (cairo);
}

void
lsm_svg_filter_surface_merge (LsmSvgFilterSurface *input,
			      LsmSvgFilterSurface *output)
{
	cairo_t *cairo;

	g_return_if_fail (input != NULL);
	g_return_if_fail (output != NULL);

	cairo = cairo_create (output->surface);
	cairo_rectangle (cairo, output->subregion.x, output->subregion.y, output->subregion.width, output->subregion.height);
	cairo_clip (cairo);
	cairo_set_source_surface (cairo, input->surface, 0, 0);
	cairo_paint (cairo);
	cairo_destroy (cairo);
}

void
lsm_svg_filter_surface_tile (LsmSvgFilterSurface *input, LsmSvgFilterSurface *output)
{
	cairo_t *cairo;
	cairo_surface_t *surface;
	cairo_pattern_t *pattern;

	g_return_if_fail (input != NULL);
	g_return_if_fail (output != NULL);

	surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, input->subregion.width, input->subregion.height);
	cairo = cairo_create (surface);
	cairo_set_source_surface (cairo, input->surface, input->subregion.x, input->subregion.y);
	cairo_paint (cairo);
	cairo_destroy (cairo);

	cairo = cairo_create (output->surface);
	cairo_rectangle (cairo, output->subregion.x, output->subregion.y, output->subregion.width, output->subregion.height);
	cairo_clip (cairo);
	cairo_set_source_surface (cairo, surface, 0, 0);
	pattern = cairo_get_source (cairo);
	cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REPEAT);
	cairo_paint (cairo);
	cairo_destroy (cairo);
	cairo_surface_destroy (surface);
}

void
lsm_svg_filter_surface_alpha (LsmSvgFilterSurface *input, LsmSvgFilterSurface *output)
{
	cairo_t *cairo;

	g_return_if_fail (input != NULL);
	g_return_if_fail (output != NULL);

	cairo = cairo_create (output->surface);
	cairo_set_source_rgb (cairo, 0, 0, 0);
	cairo_mask_surface (cairo, input->surface, 0, 0);

	cairo_destroy (cairo);
}