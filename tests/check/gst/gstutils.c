/* GStreamer
 * Copyright (C) <2005> Thomas Vander Stichele <thomas at apestaart dot org>
 *
 * gstutils.c: Unit test for functions in gstutils
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

#include <gst/check/gstcheck.h>

#define SPECIAL_POINTER(x) ((void*)(19283847+(x)))

static int n_data_probes = 0;
static int n_buffer_probes = 0;
static int n_event_probes = 0;

static gboolean
data_probe (GstPad * pad, GstMiniObject * obj, gpointer data)
{
  n_data_probes++;
  g_assert (GST_IS_MINI_OBJECT (obj));
  g_assert (data == SPECIAL_POINTER (0));
  return TRUE;
}

static gboolean
buffer_probe (GstPad * pad, GstBuffer * obj, gpointer data)
{
  n_buffer_probes++;
  g_assert (GST_IS_BUFFER (obj));
  g_assert (data == SPECIAL_POINTER (1));
  return TRUE;
}

static gboolean
event_probe (GstPad * pad, GstEvent * obj, gpointer data)
{
  n_event_probes++;
  g_assert (GST_IS_EVENT (obj));
  g_assert (data == SPECIAL_POINTER (2));
  return TRUE;
}

GST_START_TEST (test_buffer_probe_n_times)
{
  GstElement *pipeline, *fakesrc, *fakesink;
  GstBus *bus;
  GstMessage *message;
  GstPad *pad;

  pipeline = gst_element_factory_make ("pipeline", NULL);
  fakesrc = gst_element_factory_make ("fakesrc", NULL);
  fakesink = gst_element_factory_make ("fakesink", NULL);

  g_object_set (fakesrc, "num-buffers", (int) 10, NULL);

  gst_bin_add_many (GST_BIN (pipeline), fakesrc, fakesink, NULL);
  gst_element_link (fakesrc, fakesink);

  pad = gst_element_get_pad (fakesink, "sink");
  gst_pad_add_data_probe (pad, G_CALLBACK (data_probe), SPECIAL_POINTER (0));
  gst_pad_add_buffer_probe (pad, G_CALLBACK (buffer_probe),
      SPECIAL_POINTER (1));
  gst_pad_add_event_probe (pad, G_CALLBACK (event_probe), SPECIAL_POINTER (2));
  gst_object_unref (pad);

  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  bus = gst_element_get_bus (pipeline);
  message = gst_bus_poll (bus, GST_MESSAGE_EOS, -1);
  gst_message_unref (message);
  gst_object_unref (bus);

  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);

  g_assert (n_buffer_probes == 10);     /* one for every buffer */
  g_assert (n_event_probes == 2);       /* new segment and eos */
  g_assert (n_data_probes == 12);       /* duh */
} GST_END_TEST;

static int n_data_probes_once = 0;
static int n_buffer_probes_once = 0;
static int n_event_probes_once = 0;

static gboolean
data_probe_once (GstPad * pad, GstMiniObject * obj, guint * data)
{
  n_data_probes_once++;
  g_assert (GST_IS_MINI_OBJECT (obj));

  gst_pad_remove_data_probe (pad, *data);

  return TRUE;
}

static gboolean
buffer_probe_once (GstPad * pad, GstBuffer * obj, guint * data)
{
  n_buffer_probes_once++;
  g_assert (GST_IS_BUFFER (obj));

  gst_pad_remove_buffer_probe (pad, *data);

  return TRUE;
}

static gboolean
event_probe_once (GstPad * pad, GstEvent * obj, guint * data)
{
  n_event_probes_once++;
  g_assert (GST_IS_EVENT (obj));

  gst_pad_remove_event_probe (pad, *data);

  return TRUE;
}

GST_START_TEST (test_buffer_probe_once)
{
  GstElement *pipeline, *fakesrc, *fakesink;
  GstBus *bus;
  GstMessage *message;
  GstPad *pad;
  guint id1, id2, id3;

  pipeline = gst_element_factory_make ("pipeline", NULL);
  fakesrc = gst_element_factory_make ("fakesrc", NULL);
  fakesink = gst_element_factory_make ("fakesink", NULL);

  g_object_set (fakesrc, "num-buffers", (int) 10, NULL);

  gst_bin_add_many (GST_BIN (pipeline), fakesrc, fakesink, NULL);
  gst_element_link (fakesrc, fakesink);

  pad = gst_element_get_pad (fakesink, "sink");
  id1 = gst_pad_add_data_probe (pad, G_CALLBACK (data_probe_once), &id1);
  id2 = gst_pad_add_buffer_probe (pad, G_CALLBACK (buffer_probe_once), &id2);
  id3 = gst_pad_add_event_probe (pad, G_CALLBACK (event_probe_once), &id3);
  gst_object_unref (pad);

  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  bus = gst_element_get_bus (pipeline);
  message = gst_bus_poll (bus, GST_MESSAGE_EOS, -1);
  gst_message_unref (message);
  gst_object_unref (bus);

  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);

  g_assert (n_buffer_probes_once == 1); /* can we hit it and quit? */
  g_assert (n_event_probes_once == 1);  /* i said, can we hit it and quit? */
  g_assert (n_data_probes_once == 1);   /* let's hit it and quit!!! */
} GST_END_TEST;

Suite *
gst_utils_suite (void)
{
  Suite *s = suite_create ("GstUtils");
  TCase *tc_chain = tcase_create ("general");

  suite_add_tcase (s, tc_chain);
  tcase_add_test (tc_chain, test_buffer_probe_n_times);
  tcase_add_test (tc_chain, test_buffer_probe_once);
  return s;
}

int
main (int argc, char **argv)
{
  int nf;

  Suite *s = gst_utils_suite ();
  SRunner *sr = srunner_create (s);

  gst_check_init (&argc, &argv);

  srunner_run_all (sr, CK_NORMAL);
  nf = srunner_ntests_failed (sr);
  srunner_free (sr);

  return nf;
}