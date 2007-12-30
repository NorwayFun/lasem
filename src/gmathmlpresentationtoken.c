/* gmathmlpresentationtoken.c
 *
 * Copyright (C) 2007  Emmanuel Pacaud
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

#include <gmathmlpresentationtoken.h>
#include <gdomtext.h>

static gboolean
gmathml_presentation_token_can_append_child (GDomNode *self, GDomNode *child)
{
	return (GDOM_IS_TEXT (child) /*||
		GMATHML_IS_GLYPH_ELEMENT (child) ||
		GMATHML_IS_ALIGN_MARK_ELEMENT (child)*/);
}

static void
gmathml_presentation_token_init (GMathmlPresentationToken *token)
{
}

static void
gmathml_presentation_token_class_init (GMathmlPresentationTokenClass *klass)
{
	GDomNodeClass *node_class = GDOM_NODE_CLASS (klass);

	node_class->can_append_child = gmathml_presentation_token_can_append_child;
}

G_DEFINE_ABSTRACT_TYPE (GMathmlPresentationToken, gmathml_presentation_token, GMATHML_TYPE_ELEMENT)