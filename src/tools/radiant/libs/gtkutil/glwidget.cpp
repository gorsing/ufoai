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

#include "glwidget.h"
#include "debugging/debugging.h"
#include "igl.h"

#include <gtk/gtk.h>
#include <GL/gl.h>

void (*GLWidget_sharedContextCreated) () = 0;
void (*GLWidget_sharedContextDestroyed) () = 0;

static unsigned int g_context_count = 0;

namespace gtkutil {

GdkGLContext* GLWidget::onCreateContext(GtkWidget* widget, gpointer user_data) {
    GdkWindow* window = gtk_widget_get_window(widget);
    GError* error = NULL;
    GdkGLContext* context = gdk_window_create_gl_context(window, &error);

    if (!context) {
        globalErrorStream() << "Failed to create GL context: " << error->message << "\n";
        g_error_free(error);
        return NULL;
    }

    gdk_gl_context_set_forward_compatible(context, FALSE);
    gdk_gl_context_set_use_es(context, FALSE);
    return context;
}

GLWidget::GLWidget(bool zBuffer) : _zBuffer(zBuffer) {
    _widget = gtk_gl_area_new();

    gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(_widget), zBuffer);
    gtk_gl_area_set_has_alpha(GTK_GL_AREA(_widget), TRUE);

    g_signal_connect(_widget, "create-context", G_CALLBACK(onCreateContext), this);
    g_signal_connect(_widget, "realize", G_CALLBACK(onRealize), this);
    g_signal_connect(_widget, "unrealize", G_CALLBACK(onUnrealize), this);
}

GLWidget::operator GtkWidget*() const {
    return _widget;
}

bool GLWidget::makeCurrent(GtkWidget* widget) {
    if (!GTK_IS_GL_AREA(widget)) return false;

    gtk_gl_area_make_current(GTK_GL_AREA(widget));
    return gtk_gl_area_get_error(GTK_GL_AREA(widget)) == NULL;
}

void GLWidget::swapBuffers(GtkWidget* widget) {
    if (!GTK_IS_GL_AREA(widget)) return;

    glFlush();
    gtk_widget_queue_draw(widget);
}

void GLWidget::onRealize(GtkWidget* widget, GLWidget* self) {
    gtk_gl_area_make_current(GTK_GL_AREA(widget));

    if (++g_context_count == 1) {
        GlobalOpenGL().contextValid = true;

        if (GLWidget_sharedContextCreated) {
            GLWidget_sharedContextCreated();
        }
    }
}

void GLWidget::onUnrealize(GtkWidget* widget, GLWidget* self) {
    if (--g_context_count == 0) {
        GlobalOpenGL().contextValid = false;

        if (GLWidget_sharedContextDestroyed) {
            GLWidget_sharedContextDestroyed();
        }
    }
}

}
