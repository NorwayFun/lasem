/* Lasem
 *
 * Copyright © 2007-2014 Emmanuel Pacaud
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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1335, USA.
 *
 * Author:
 * 	Emmanuel Pacaud <emmanuel@gnome.org>
 */

#include <lsmdebug.h>
#include <lsmdomimplementation.h>
#include <lsmdomnode.h>
#include <lsmdomentities.h>
#include <lsmsvgtextelement.h>
#include <lsmstr.h>
#include <libxml/parser.h>
#include <gio/gio.h>
#include <string.h>
#include <../itex2mml/itex2MML.h>

typedef enum {
	STATE
} LsmDomSaxParserStateEnum;

typedef struct {
	LsmDomSaxParserStateEnum state;

	LsmDomDocument *document;
	LsmDomNode *current_node;

	gboolean is_error;

	int error_depth;

	GHashTable *entities;
} LsmDomSaxParserState;

static void
_free_entity (void *data)
{
	xmlEntity *entity = data;

	xmlFree ((xmlChar *) entity->name);
	xmlFree ((xmlChar *) entity->ExternalID);
	xmlFree ((xmlChar *) entity->SystemID);
	xmlFree (entity->content);
	xmlFree (entity->orig);
	g_free (entity);
}

static void
lsm_dom_parser_start_document (void *user_data)
{
	LsmDomSaxParserState *state = user_data;

	state->state = STATE;
	state->is_error = FALSE;
	state->error_depth = 0;
	state->entities = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, _free_entity);
}

static void
lsm_dom_parser_end_document (void *user_data)
{
	LsmDomSaxParserState *state = user_data;

	g_hash_table_unref (state->entities);
}

static void
lsm_dom_parser_start_element(void *user_data,
			     const xmlChar *name,
			     const xmlChar **attrs)
{
	LsmDomSaxParserState *state = user_data;
	LsmDomNode *node;
	int i;

	if (state->is_error) {
		state->error_depth++;
		return;
	}

	if (state->document == NULL) {
		state->document = lsm_dom_implementation_create_document (NULL, (char *) name);
		state->current_node = LSM_DOM_NODE (state->document);

		g_return_if_fail (LSM_IS_DOM_DOCUMENT (state->document));
	}

	node = LSM_DOM_NODE (lsm_dom_document_create_element (LSM_DOM_DOCUMENT (state->document), (char *) name));

	if (LSM_IS_DOM_NODE (node) && lsm_dom_node_append_child (state->current_node, node) != NULL) {
		if (attrs != NULL)
			for (i = 0; attrs[i] != NULL && attrs[i+1] != NULL; i += 2)
				lsm_dom_element_set_attribute (LSM_DOM_ELEMENT (node),
							       (char *) attrs[i],
							       (char *) attrs[i+1]);

		state->current_node = node;
		state->is_error = FALSE;
		state->error_depth = 0;
	} else {
		state->is_error = TRUE;
		state->error_depth = 1;
	}
}

static void
lsm_dom_parser_end_element (void *user_data,
			    const xmlChar *name)
{
	LsmDomSaxParserState *state = user_data;

	if (state->is_error) {
		state->error_depth--;
		if (state->error_depth > 0) {
			return;
		}

		state->is_error = FALSE;
		return;
	}

	state->current_node = lsm_dom_node_get_parent_node (state->current_node);
}

static void
lsm_dom_parser_characters (void *user_data, const xmlChar *ch, int len)
{
	LsmDomSaxParserState *state = user_data;

	if (!state->is_error) {
		LsmDomNode *node;
		char *text;

		text = g_strndup ((char *) ch, len);
		node = LSM_DOM_NODE (lsm_dom_document_create_text_node (LSM_DOM_DOCUMENT (state->document), text));

		lsm_dom_node_append_child (state->current_node, node);

		g_free (text);
	}
}

static xmlEntityPtr
lsm_dom_parser_get_entity (void *user_data, const xmlChar *name)
{
	LsmDomSaxParserState *state = user_data;
	xmlEntity *entity;
	const char *utf8;

	entity = g_hash_table_lookup (state->entities, name);
	if (entity != NULL)
		return entity;

	utf8 = lsm_dom_get_entity ((char *) name);
	if (utf8 != NULL) {
		entity = xmlNewEntity (NULL, name, XML_INTERNAL_GENERAL_ENTITY, NULL, NULL, (xmlChar *) utf8);

		g_hash_table_insert (state->entities, (char *) name, entity);

		return entity;
	}

	return xmlGetPredefinedEntity(name);
}

static void
lsm_dom_parser_declare_entity (void * user_data, const xmlChar * name, int type,
			       const xmlChar * publicId, const xmlChar * systemId,
			       xmlChar * content)
{
	LsmDomSaxParserState *state = user_data;

	if (content != NULL) {
		xmlEntity *entity;

		entity = xmlNewEntity (NULL, name, type, publicId, systemId, content);

		g_hash_table_insert (state->entities, (char *) name, entity);
	}
}

static void
lsm_dom_parser_warning (void *user_data, const char *msg, ...)
{
	va_list args;

	va_start(args, msg);
	lsm_warning_dom ("%s", msg);
	va_end(args);
}

static void
lsm_dom_parser_error (void *user_data, const char *msg, ...)
{
	va_list args;

	va_start(args, msg);
	lsm_warning_dom ("%s", msg);
	va_end(args);
}

static void
lsm_dom_parser_fatal_error (void *user_data, const char *msg, ...)
{
	va_list args;

	va_start(args, msg);
	lsm_warning_dom ("%s", msg);
	va_end(args);
}

static xmlSAXHandler sax_handler = {
	.warning = lsm_dom_parser_warning,
	.error = lsm_dom_parser_error,
	.fatalError = lsm_dom_parser_fatal_error,
	.startDocument = lsm_dom_parser_start_document,
	.endDocument = lsm_dom_parser_end_document,
	.startElement = lsm_dom_parser_start_element,
	.endElement = lsm_dom_parser_end_element,
	.characters = lsm_dom_parser_characters,
	.getEntity = lsm_dom_parser_get_entity,
	.entityDecl = lsm_dom_parser_declare_entity
};

static GQuark
lsm_dom_document_error_quark (void)
{
	static GQuark q = 0;

        if (q == 0) {
                q = g_quark_from_static_string ("lsm-dom-error-quark");
        }

        return q;
}

#define LSM_DOM_DOCUMENT_ERROR lsm_dom_document_error_quark ()

typedef enum {
	LSM_DOM_DOCUMENT_ERROR_INVALID_XML
} LsmDomDocumentError;

#if LIBXML_VERSION >= 21100
static LsmDomDocument *
_parse_memory (LsmDomDocument *document, LsmDomNode *node,
	       const void *buffer, int size, GError **error)
{
	static LsmDomSaxParserState state;
        xmlParserCtxt *xml_parser_ctxt;

	state.document = document;
	if (node != NULL)
		state.current_node = node;
	else
		state.current_node = LSM_DOM_NODE (document);

	if (size < 0)
		size = strlen (buffer);

        xml_parser_ctxt = xmlNewSAXParserCtxt (&sax_handler, &state);
        if (xml_parser_ctxt == NULL) {
                g_set_error (error,
                             LSM_DOM_DOCUMENT_ERROR,
                             LSM_DOM_DOCUMENT_ERROR_INVALID_XML,
                             "Failed to create parser context");
                return NULL;
        }

        xmlCtxtReadMemory (xml_parser_ctxt, buffer, size, NULL, NULL, 0);

        if (!xml_parser_ctxt->wellFormed) {
                if (state.document !=  NULL)
                        g_object_unref (state.document);
                state.document = NULL;

                lsm_debug_dom ("[DomParser::parse] Invalid document");

                g_set_error (error,
                             LSM_DOM_DOCUMENT_ERROR,
                             LSM_DOM_DOCUMENT_ERROR_INVALID_XML,
                             "Invalid document");
        }

        xmlFreeParserCtxt(xml_parser_ctxt);

	return state.document;
}
#else
static LsmDomDocument *
_parse_memory (LsmDomDocument *document, LsmDomNode *node,
	       const char *buffer, gssize size, GError **error)
{
	static LsmDomSaxParserState state;

	state.document = document;
	if (node != NULL)
		state.current_node = node;
	else
		state.current_node = LSM_DOM_NODE (document);

	if (size < 0)
		size = strlen (buffer);

	if (xmlSAXUserParseMemory (&sax_handler, &state, buffer, size) < 0) {
		if (state.document !=  NULL)
			g_object_unref (state.document);
		state.document = NULL;

		lsm_debug_dom ("[LsmDomParser::from_memory] Invalid document");

		g_set_error (error,
			     LSM_DOM_DOCUMENT_ERROR,
			     LSM_DOM_DOCUMENT_ERROR_INVALID_XML,
			     "Invalid document.");
	}

	return state.document;
}
#endif

/**
 * lsm_dom_document_append_from_memory:
 * @document: a #LsmDomDocument
 * @node: a #LsmDomNode
 * @buffer: a memory buffer holding xml data
 * @size: size of the xml data, in bytes, -1 if NULL terminated
 * @error: an error placeholder
 *
 * Append a chunk of xml tree to an existing document. The resulting nodes will be appended to
 * @node, or to @document if @node == NULL.
 *
 * Size set to -1 indicates the buffer is NULL terminated.
 */

void
lsm_dom_document_append_from_memory (LsmDomDocument *document, LsmDomNode *node,
				     const char *buffer, gssize size, GError **error)
{
	g_return_if_fail (LSM_IS_DOM_DOCUMENT (document));
	g_return_if_fail (LSM_IS_DOM_NODE (node) || node == NULL);
	g_return_if_fail (buffer != NULL);

	_parse_memory (document, node, buffer, size, error);
}

/**
 * lsm_dom_document_new_from_memory:
 * @buffer: xml data
 * @size: size of the data, in bytes, -1 if NULL terminated
 * @error: an error placeholder
 *
 * Create a new document from a memory data buffer.
 */

LsmDomDocument *
lsm_dom_document_new_from_memory (const char *buffer, gssize size, GError **error)
{
	g_return_val_if_fail (buffer != NULL, NULL);

	return _parse_memory (NULL, NULL, buffer, size, error);
}

/**
 * lsm_dom_document_new_from_file:
 * @file: a #GFile
 * @error: an error placeholder
 *
 * Create a new document from a #GFile.
 */

static LsmDomDocument *
lsm_dom_document_new_from_file (GFile *file, GError **error)
{
	LsmDomDocument *document;
	gsize size = 0;
	char *contents = NULL;

	if (!g_file_load_contents (file, NULL, &contents, &size, NULL, error))
		return NULL;

	document = lsm_dom_document_new_from_memory (contents, size, error);

	g_free (contents);

	return document;
}

/**
 * lsm_dom_document_new_from_path:
 * @path: a file path
 * @error: an error placeholder
 *
 * Create a new document from the data stored in @path.
 */

LsmDomDocument *
lsm_dom_document_new_from_path (const char *path, GError **error)
{
	LsmDomDocument *document;
	GFile *file;

	g_return_val_if_fail (path != NULL, NULL);

	file = g_file_new_for_path (path);

	document = lsm_dom_document_new_from_file (file, error);

	g_object_unref (file);

	if (document != NULL)
		lsm_dom_document_set_path (document, path);

	return document;
}

/**
 * lsm_dom_document_new_from_url:
 * @url: a file url
 * @error: an error placeholder
 *
 * Create a new document from the data stored at @url.
 */

LsmDomDocument *
lsm_dom_document_new_from_url (const char *url, GError **error)
{
	LsmDomDocument *document;
	GFile *file;

	g_return_val_if_fail (url != NULL, NULL);

	file = g_file_new_for_uri (url);

	document = lsm_dom_document_new_from_file (file, error);

	g_object_unref (file);

	if (document != NULL)
		lsm_dom_document_set_url (document, url);

	return document;
}

/**
 * lsm_dom_document_save_to_stream:
 * @document: a #LsmDomDocument
 * @stream: stream to save to
 * @error: an error placeholder
 *
 * Save @document as an xml representation into @stream.
 */

void
lsm_dom_document_save_to_stream (LsmDomDocument *document, GOutputStream *stream, GError **error)
{
	g_return_if_fail (LSM_IS_DOM_DOCUMENT (document));
	g_return_if_fail (G_IS_OUTPUT_STREAM (stream));

	lsm_dom_node_write_to_stream (LSM_DOM_NODE (document), stream, error);
}

/**
 * lsm_dom_document_save_to_memory:
 * @document: a #LsmDomDocument
 * @buffer: (out callee-allocates): placeholder for a pointer to the resulting data buffer
 * @size: (out) (optional): placeholder for the data size
 * @error: placeholder for a #GError
 *
 * Save @document as an xml representation into @buffer.
 */

void
lsm_dom_document_save_to_memory	(LsmDomDocument *document, char **buffer, gsize *size, GError **error)
{
	GOutputStream *stream;

	if (buffer != NULL)
		*buffer = NULL;
	if (size != NULL)
		*size = 0;

	g_return_if_fail (document != NULL);
	g_return_if_fail (buffer != NULL);

	stream = g_memory_output_stream_new (NULL, 0, g_realloc, g_free);
	if (stream == NULL) {
		*buffer = NULL;
		if (size != NULL)
			*size = 0;
		return;
	}

	lsm_dom_document_save_to_stream (document, G_OUTPUT_STREAM (stream), error);
	g_output_stream_close (G_OUTPUT_STREAM (stream), NULL, error);

	if (size != NULL)
		*size = g_memory_output_stream_get_data_size (G_MEMORY_OUTPUT_STREAM (stream));
	*buffer = g_memory_output_stream_steal_data (G_MEMORY_OUTPUT_STREAM (stream));

	g_object_unref (stream);
}

/**
 * lsm_dom_document_save_to_path:
 * @document: a #LsmDomDocument
 * @path: a file path
 * @error: placeholder for a #GError
 *
 * Save @document as an xml representation to a file, replacing the already existing file if needed.
 */

void
lsm_dom_document_save_to_path (LsmDomDocument *document, const char *path, GError **error)
{
	GFile *file;
	GFileOutputStream *stream;

	g_return_if_fail (path != NULL);

	file = g_file_new_for_path (path);
	stream = g_file_create (file, G_FILE_CREATE_REPLACE_DESTINATION, NULL, error);
	if (stream != NULL) {
		lsm_dom_document_save_to_stream (document, G_OUTPUT_STREAM (stream), error);
		g_object_unref (stream);
	}
	g_object_unref (file);
}

/**
 * lsm_dom_document_save_to_url:
 * @document: a #LsmDomDocument
 * @url: an url
 * @error: placeholder for a #GError
 *
 * Save @document as an xml representation to @url, replacing the already existing file if needed.
 */

void
lsm_dom_document_save_to_url (LsmDomDocument *document, const char *url, GError **error)
{
	GFile *file;
	GFileOutputStream *stream;

	g_return_if_fail (url != NULL);

	file = g_file_new_for_uri (url);
	stream = g_file_create (file, G_FILE_CREATE_REPLACE_DESTINATION, NULL, error);
	if (stream != NULL) {
		lsm_dom_document_save_to_stream (document, G_OUTPUT_STREAM (stream), error);
		g_object_unref (stream);
	}
	g_object_unref (file);
}
