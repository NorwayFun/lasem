/*
 * Copyright © 2009 Emmanuel Pacaud
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 * Author:
 * 	Emmanuel Pacaud <emmanuel@gnome.org>
 */

#ifndef LSM_SVG_MATRIX_H
#define LSM_SVG_MATRIX_H

#include <lsmsvg.h>

typedef struct {
	double a, b, c, d, e, f;
} LsmSvgMatrix;

void 	lsm_svg_matrix_init 			(LsmSvgMatrix *matrix, double a, double b,
					                               double c, double d,
								       double e, double f);
void 	lsm_svg_matrix_init_identity 		(LsmSvgMatrix *matrix);
void 	lsm_svg_matrix_init_scale 		(LsmSvgMatrix *matrix, double sx, double sy);
void 	lsm_svg_matrix_scale 			(LsmSvgMatrix *matrix, double sx, double sy);
void 	lsm_svg_matrix_init_translate 		(LsmSvgMatrix *matrix, double tx, double ty);
void 	lsm_svg_matrix_translate 		(LsmSvgMatrix *matrix, double tx, double ty);
void 	lsm_svg_matrix_init_rotate 		(LsmSvgMatrix *matrix, double radians);
void 	lsm_svg_matrix_rotate 			(LsmSvgMatrix *matrix, double radians);
void 	lsm_svg_matrix_init_skew_x 		(LsmSvgMatrix *matrix, double radians);
void 	lsm_svg_matrix_skew_x 			(LsmSvgMatrix *matrix, double radians);
void 	lsm_svg_matrix_init_skew_y 		(LsmSvgMatrix *matrix, double radians);
void 	lsm_svg_matrix_skew_y 			(LsmSvgMatrix *matrix, double radians);
void 	lsm_svg_matrix_multiply 		(LsmSvgMatrix *result, const LsmSvgMatrix *a, const LsmSvgMatrix *b);
void 	lsm_svg_matrix_transform_point 		(const LsmSvgMatrix *matrix, double *x, double *y);
void 	lsm_svg_matrix_transform_bounding_box 	(const LsmSvgMatrix *matrix, double *x1, double *y1,
						                             double *x2, double *y2);

#endif
