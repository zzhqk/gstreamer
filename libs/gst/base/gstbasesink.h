/* GStreamer
 * Copyright (C) 1999,2000 Erik Walthinsen <omega@cse.ogi.edu>
 *                    2000 Wim Taymans <wtay@chello.be>
 *
 * gstbasesink.h: 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GST_BASESINK_H__
#define __GST_BASESINK_H__

#include <gst/gst.h>

G_BEGIN_DECLS


#define GST_TYPE_BASESINK  		(gst_basesink_get_type())
#define GST_BASESINK(obj) 		(G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_BASESINK,GstBaseSink))
#define GST_BASESINK_CLASS(klass) 	(G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_BASESINK,GstBaseSinkClass))
#define GST_BASESINK_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_BASESINK, GstBaseSinkClass))
#define GST_IS_BASESINK(obj)  		(G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_BASESINK))
#define GST_IS_BASESINK_CLASS(obj) 	(G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_BASESINK))

#define GST_BASESINK_CLOCK(obj)		(GST_BASESINK (obj)->clock)
#define GST_BASESINK_PAD(obj)		(GST_BASESINK (obj)->sinkpad)

typedef struct _GstBaseSink GstBaseSink;
typedef struct _GstBaseSinkClass GstBaseSinkClass;

struct _GstBaseSink {
  GstElement 	 element;

  GstPad 	*sinkpad;
  GstActivateMode	pad_mode;

  GQueue	*preroll_queue; /* with PREROLL_LOCK */
  gint		 preroll_queue_max_len; /* with PREROLL_LOCK */

  guint64	 offset;
  gboolean 	 has_loop;
  gboolean 	 has_chain;

  GstClock 	*clock;
  GstClockID     clock_id;
  GstClockTime   end_time;
  
  gboolean       eos;
  gboolean       need_preroll;
  gboolean       have_preroll;
};

struct _GstBaseSinkClass {
  GstElementClass parent_class;

  GstCaps*      (*get_caps)     (GstBaseSink *sink);
  gboolean      (*set_caps)     (GstBaseSink *sink, GstCaps *caps);

  GstBuffer*    (*buffer_alloc) (GstBaseSink *sink, guint64 offset, guint size,
		                 GstCaps *caps);

  void		(*get_times)    (GstBaseSink *sink, GstBuffer *buffer, 
		                 GstClockTime *start, GstClockTime *end);

  void          (*event)        (GstBaseSink *sink, GstEvent *event);
  GstFlowReturn (*preroll)      (GstBaseSink *sink, GstBuffer *buffer);
  GstFlowReturn (*render)       (GstBaseSink *sink, GstBuffer *buffer);
};

GType gst_basesink_get_type(void);

G_END_DECLS

#endif /* __GST_BASESINK_H__ */