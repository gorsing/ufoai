/*
 Copyright (C) 2001-2006, William Joseph.
 All Rights Reserved.

 This file is part of GtkRadiant.

 GtkRadiant is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 GtkRadiant is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with GtkRadiant; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "glfont.h"
#include <GL/gl.h>
#include "debugging/debugging.h"

#include <cairo.h>
#include <pango/pangocairo.h>
#include <vector>

GLFont glfont_create (const std::string& font_string)
{
    const int atlas_width = 256;
    const int atlas_height = 256;
    const int cell_size = 16; 

    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, atlas_width, atlas_height);
    cairo_t* cr = cairo_create(surface);

    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);

    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    PangoLayout* layout = pango_cairo_create_layout(cr);
    PangoFontDescription* font_desc = pango_font_description_from_string(font_string.c_str());
    pango_layout_set_font_description(layout, font_desc);
    pango_font_description_free(font_desc);

    int font_height = 0;
    std::vector<int> char_widths(256, cell_size);

    for (int i = 0; i < 256; i++) {
        if (i < 32) continue;

        char buf[2] = { (char)i, '\0' };
        pango_layout_set_text(layout, buf, 1);

        int w, h;
        pango_layout_get_pixel_size(layout, &w, &h);
        if (h > font_height) font_height = h;
        char_widths[i] = w;

        int x = (i % 16) * cell_size;
        int y = (i / 16) * cell_size;

        cairo_move_to(cr, x, y);
        pango_cairo_show_layout(cr, layout);
    }
    
    g_object_unref(layout);
    
    if (font_height > cell_size) font_height = cell_size;

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned char* data = cairo_image_surface_get_data(surface);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlas_width, atlas_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    GLuint font_list_base = glGenLists(256);
    
    for (int i = 0; i < 256; i++) {
        glNewList(font_list_base + i, GL_COMPILE);

        if (i >= 32) {
            float u = (float)(i % 16) * cell_size / atlas_width;
            float v = (float)(i / 16) * cell_size / atlas_height;
            float uw = (float)char_widths[i] / atlas_width;
            float vh = (float)font_height / atlas_height;

            glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT);
            
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texture_id);
            
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            glBegin(GL_QUADS);
            glTexCoord2f(u, v + vh);      glVertex2f(0, font_height);
            glTexCoord2f(u + uw, v + vh); glVertex2f(char_widths[i], font_height);
            glTexCoord2f(u + uw, v);      glVertex2f(char_widths[i], 0);
            glTexCoord2f(u, v);           glVertex2f(0, 0);
            glEnd();

            glPopAttrib();

            glTranslatef(char_widths[i], 0, 0);
        }
        glEndList();
    }

    globalOutputStream() << "Generated Cairo Texture Atlas for font " << font_string << "\n";

    return GLFont(font_list_base, font_height, texture_id);
}

void glfont_release (GLFont& font)
{
    if (font.getDisplayList() != 0) {
        glDeleteLists(font.getDisplayList(), 256);
    }
    if (font.getTexture() != 0) {
        GLuint tex = font.getTexture();
        glDeleteTextures(1, &tex);
    }
    font = GLFont(0, 0, 0);
}
