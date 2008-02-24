/* gdomcharacterdata.c
 *
 * Copyright © 2007-2008  Emmanuel Pacaud
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

#include <gdomcharacterdata.h>

static GObjectClass *parent_class = NULL;

/* GDomCharacterData implementation */

char*
gdom_character_data_get_data (GDomCharacterData* self)
{
	g_return_val_if_fail (GDOM_IS_CHARACTER_DATA (self), NULL);

	return self->data;
}

void
gdom_character_data_set_data (GDomCharacterData* self, const char* value)
{
	g_return_if_fail (GDOM_IS_CHARACTER_DATA (self));
	g_return_if_fail (value != NULL);

	g_free (self->data);
	self->data = g_strdup (value);
}

static void
gdom_character_data_init (GDomCharacterData *character_data)
{
}

static void
gdom_character_data_finalize (GObject *object)
{
	GDomCharacterData *self = GDOM_CHARACTER_DATA (object);

	g_free (self->data);

	parent_class->finalize (object);
}

/* GDomCharacterData class */

static void
gdom_character_data_class_init (GDomCharacterDataClass *character_data_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (character_data_class);

	parent_class = g_type_class_peek_parent (character_data_class);

	object_class->finalize = gdom_character_data_finalize;
}

G_DEFINE_ABSTRACT_TYPE (GDomCharacterData, gdom_character_data, GDOM_TYPE_NODE)
