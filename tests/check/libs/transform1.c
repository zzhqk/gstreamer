/* GStreamer
 *
 * some unit tests for GstBaseTransform
 *
 * Copyright (C) 2008 Wim Taymans <wim.taymans@gmail.com>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <gst/gst.h>
#include <gst/check/gstcheck.h>
#include <gst/base/gstbasetransform.h>

#undef FAILING_TESTS

#include "test_transform.c"

static gboolean buffer_alloc_pt1_called;

static GstFlowReturn
buffer_alloc_pt1 (GstPad * pad, guint64 offset, guint size, GstCaps * caps,
    GstBuffer ** buf)
{
  GST_DEBUG_OBJECT (pad, "buffer_alloc called %" G_GUINT64_FORMAT ", %u, %"
      GST_PTR_FORMAT, offset, size, caps);

  buffer_alloc_pt1_called = TRUE;

  *buf = gst_buffer_new_and_alloc (size);
  gst_buffer_set_caps (*buf, caps);

  return GST_FLOW_OK;
}

static gboolean set_caps_pt1_called;

static gboolean
set_caps_pt1 (GstBaseTransform * trans, GstCaps * incaps, GstCaps * outcaps)
{
  GST_DEBUG_OBJECT (trans, "set_caps called");

  set_caps_pt1_called = TRUE;

  return TRUE;
}

/* basic passthrough, we don't have any transform functions so we can only
 * perform passthrough. We also don't have caps, which is fine */
GST_START_TEST (basetransform_chain_pt1)
{
  TestTransData *trans;
  GstBuffer *buffer;
  GstFlowReturn res;
  GstCaps *caps;

  klass_set_caps = set_caps_pt1;
  trans = gst_test_trans_new ();
  trans->buffer_alloc = buffer_alloc_pt1;

  GST_DEBUG_OBJECT (trans, "buffer without caps, size 20");

  buffer = gst_buffer_new_and_alloc (20);

  buffer_alloc_pt1_called = FALSE;
  set_caps_pt1_called = FALSE;;
  res = gst_test_trans_push (trans, buffer);
  fail_unless (res == GST_FLOW_OK);
#ifdef FAILING_TESTS
  /* FIXME, passthough without pad-alloc, do pad-alloc on the srcpad */
  fail_unless (buffer_alloc_pt1_called == TRUE);
#endif
  fail_unless (set_caps_pt1_called == FALSE);

  buffer = gst_test_trans_pop (trans);
  fail_unless (buffer != NULL);
  fail_unless (GST_BUFFER_SIZE (buffer) == 20);
  /* caps should not have been set */
  fail_unless (GST_BUFFER_CAPS (buffer) == NULL);

  gst_buffer_unref (buffer);

  GST_DEBUG_OBJECT (trans, "buffer without caps, size 10");

  buffer = gst_buffer_new_and_alloc (10);
  buffer_alloc_pt1_called = FALSE;
  set_caps_pt1_called = FALSE;;
  res = gst_test_trans_push (trans, buffer);
  fail_unless (res == GST_FLOW_OK);
#ifdef FAILING_TESTS
  /* FIXME, passthough without pad-alloc, do pad-alloc on the srcpad */
  fail_unless (buffer_alloc_pt1_called == TRUE);
#endif
  fail_unless (set_caps_pt1_called == FALSE);

  buffer = gst_test_trans_pop (trans);
  fail_unless (buffer != NULL);
  fail_unless (GST_BUFFER_SIZE (buffer) == 10);
  /* caps should not have been set */
  fail_unless (GST_BUFFER_CAPS (buffer) == NULL);

  gst_buffer_unref (buffer);

  /* proxy buffer-alloc without caps */
  GST_DEBUG_OBJECT (trans, "alloc without caps, size 20");

  buffer_alloc_pt1_called = FALSE;
  set_caps_pt1_called = FALSE;;
  res = gst_pad_alloc_buffer (trans->srcpad, 0, 20, NULL, &buffer);
  fail_unless (res == GST_FLOW_OK);
  fail_unless (buffer_alloc_pt1_called == TRUE);
  fail_unless (set_caps_pt1_called == FALSE);
  gst_buffer_unref (buffer);

  /* with caps buffer */
  GST_DEBUG_OBJECT (trans, "alloc with caps, size 10");

  caps = gst_caps_new_simple ("foo/x-bar", NULL);
  buffer_alloc_pt1_called = FALSE;
  set_caps_pt1_called = FALSE;;
  res = gst_pad_alloc_buffer (trans->srcpad, 0, 10, caps, &buffer);
  fail_unless (res == GST_FLOW_OK);
  fail_unless (buffer_alloc_pt1_called == TRUE);
  fail_unless (set_caps_pt1_called == FALSE);
  gst_buffer_unref (buffer);

  /* once more */
  buffer_alloc_pt1_called = FALSE;
  set_caps_pt1_called = FALSE;;
  res = gst_pad_alloc_buffer (trans->srcpad, 0, 10, caps, &buffer);
  fail_unless (res == GST_FLOW_OK);
  fail_unless (buffer_alloc_pt1_called == TRUE);
  fail_unless (set_caps_pt1_called == FALSE);
  gst_buffer_unref (buffer);

  gst_caps_unref (caps);

  gst_test_trans_free (trans);
}

GST_END_TEST;

static gboolean set_caps_pt2_called;

static gboolean
set_caps_pt2 (GstBaseTransform * trans, GstCaps * incaps, GstCaps * outcaps)
{
  GST_DEBUG_OBJECT (trans, "set_caps called");

  set_caps_pt2_called = TRUE;

  fail_unless (gst_caps_is_equal (incaps, outcaps));

  return TRUE;
}

/* basic passthrough, we don't have any transform functions so we can only
 * perform passthrough with same caps */
GST_START_TEST (basetransform_chain_pt2)
{
  TestTransData *trans;
  GstBuffer *buffer;
  GstCaps *caps;
  GstFlowReturn res;

  klass_set_caps = set_caps_pt2;
  trans = gst_test_trans_new ();
  trans->buffer_alloc = buffer_alloc_pt1;

  /* first buffer */
  caps = gst_caps_new_simple ("foo/x-bar", NULL);

  GST_DEBUG_OBJECT (trans, "buffer with caps, size 20");

  buffer = gst_buffer_new_and_alloc (20);
  gst_buffer_set_caps (buffer, caps);

  buffer_alloc_pt1_called = FALSE;
  set_caps_pt2_called = FALSE;
  res = gst_test_trans_push (trans, buffer);
  fail_unless (res == GST_FLOW_OK);
#ifdef FAILING_TESTS
  /* FIXME, passthough without pad-alloc, do pad-alloc on the srcpad */
  fail_unless (buffer_alloc_pt1_called == TRUE);
#endif
  fail_unless (set_caps_pt2_called == TRUE);

  buffer = gst_test_trans_pop (trans);
  fail_unless (buffer != NULL);
  fail_unless (GST_BUFFER_SIZE (buffer) == 20);
  fail_unless (GST_BUFFER_CAPS (buffer) == caps);

  gst_buffer_unref (buffer);

  /* with caps buffer */
  GST_DEBUG_OBJECT (trans, "alloc with caps, size 20");

  buffer_alloc_pt1_called = FALSE;
  set_caps_pt2_called = FALSE;;
  res = gst_pad_alloc_buffer (trans->srcpad, 0, 20, caps, &buffer);
  fail_unless (res == GST_FLOW_OK);
  fail_unless (buffer_alloc_pt1_called == TRUE);
  fail_unless (set_caps_pt2_called == FALSE);
  gst_buffer_unref (buffer);

  gst_caps_unref (caps);

  /* second buffer, renegotiates, keeps extra type arg in caps */
  caps = gst_caps_new_simple ("foo/x-bar", "type", G_TYPE_INT, 1, NULL);

  GST_DEBUG_OBJECT (trans, "buffer with caps, size 10");

  buffer = gst_buffer_new_and_alloc (10);
  gst_buffer_set_caps (buffer, caps);

  buffer_alloc_pt1_called = FALSE;
  set_caps_pt2_called = FALSE;
  res = gst_test_trans_push (trans, buffer);
  fail_unless (res == GST_FLOW_OK);
#ifdef FAILING_TESTS
  /* FIXME, passthough without pad-alloc, do pad-alloc on the srcpad */
  fail_unless (buffer_alloc_pt1_called == TRUE);
#endif
  fail_unless (set_caps_pt2_called == TRUE);

  buffer = gst_test_trans_pop (trans);
  fail_unless (buffer != NULL);
  fail_unless (GST_BUFFER_SIZE (buffer) == 10);
  fail_unless (GST_BUFFER_CAPS (buffer) == caps);

  gst_buffer_unref (buffer);

  /* with caps buffer */
  GST_DEBUG_OBJECT (trans, "alloc with caps, size 20");

  buffer_alloc_pt1_called = FALSE;
  set_caps_pt2_called = FALSE;;
  res = gst_pad_alloc_buffer (trans->srcpad, 0, 20, caps, &buffer);
  fail_unless (res == GST_FLOW_OK);
  fail_unless (buffer_alloc_pt1_called == TRUE);
  fail_unless (set_caps_pt2_called == FALSE);
  gst_buffer_unref (buffer);

  gst_caps_unref (caps);

  /* with caps that is a superset */
  caps = gst_caps_new_simple ("foo/x-bar", NULL);

  GST_DEBUG_OBJECT (trans, "alloc with superset caps, size 20");

  buffer_alloc_pt1_called = FALSE;
  set_caps_pt2_called = FALSE;;
  res = gst_pad_alloc_buffer (trans->srcpad, 0, 20, caps, &buffer);
  fail_unless (res == GST_FLOW_OK);
  fail_unless (buffer_alloc_pt1_called == TRUE);
  fail_unless (set_caps_pt2_called == FALSE);
  gst_buffer_unref (buffer);

  gst_caps_unref (caps);

  gst_test_trans_free (trans);
}

GST_END_TEST;

static gboolean transform_ip_1_called;
static gboolean transform_ip_1_writable;

static GstFlowReturn
transform_ip_1 (GstBaseTransform * trans, GstBuffer * buf)
{
  GST_DEBUG_OBJECT (trans, "transform called");

  transform_ip_1_called = TRUE;
  transform_ip_1_writable = gst_buffer_is_writable (buf);

  GST_DEBUG_OBJECT (trans, "writable: %d", transform_ip_1_writable);

  return GST_FLOW_OK;
}

/* basic in-place, check if the _ip function is called, buffer should
 * be writable. no setcaps is set */
GST_START_TEST (basetransform_chain_ip1)
{
  TestTransData *trans;
  GstBuffer *buffer;
  GstFlowReturn res;

  klass_transform_ip = transform_ip_1;
  trans = gst_test_trans_new ();
  trans->buffer_alloc = buffer_alloc_pt1;

  GST_DEBUG_OBJECT (trans, "buffer without caps, size 20");

  buffer = gst_buffer_new_and_alloc (20);

  transform_ip_1_called = FALSE;;
  transform_ip_1_writable = TRUE;;
  buffer_alloc_pt1_called = FALSE;
  res = gst_test_trans_push (trans, buffer);
  fail_unless (res == GST_FLOW_OK);
  fail_unless (transform_ip_1_called == TRUE);
  fail_unless (transform_ip_1_writable == TRUE);
#ifdef FAILING_TESTS
  /* FIXME, in-place without pad-alloc, do pad-alloc on the srcpad */
  fail_unless (buffer_alloc_pt1_called == TRUE);
#endif

  buffer = gst_test_trans_pop (trans);
  fail_unless (buffer != NULL);
  fail_unless (GST_BUFFER_SIZE (buffer) == 20);
  gst_buffer_unref (buffer);

  GST_DEBUG_OBJECT (trans, "buffer without caps extra ref, size 20");

  buffer = gst_buffer_new_and_alloc (20);
  /* take additional ref to make it non-writable */
  gst_buffer_ref (buffer);

  fail_unless (GST_MINI_OBJECT_REFCOUNT_VALUE (buffer) == 2);

  transform_ip_1_called = FALSE;;
  transform_ip_1_writable = FALSE;;
  buffer_alloc_pt1_called = FALSE;
  res = gst_test_trans_push (trans, buffer);
  fail_unless (res == GST_FLOW_OK);
  fail_unless (transform_ip_1_called == TRUE);
  /* copy should have been taken with pad-alloc */
  fail_unless (transform_ip_1_writable == TRUE);
  fail_unless (buffer_alloc_pt1_called == TRUE);
  /* after push, get rid of the final ref we had */
  gst_buffer_unref (buffer);

  buffer = gst_test_trans_pop (trans);
  fail_unless (buffer != NULL);
  fail_unless (GST_BUFFER_SIZE (buffer) == 20);

  /* output buffer has refcount 1 */
  fail_unless (GST_MINI_OBJECT_REFCOUNT_VALUE (buffer) == 1);
  gst_buffer_unref (buffer);

  /* with caps buffer */
  GST_DEBUG_OBJECT (trans, "alloc without caps, size 20");

  buffer_alloc_pt1_called = FALSE;
  res = gst_pad_alloc_buffer (trans->srcpad, 0, 20, NULL, &buffer);
  fail_unless (res == GST_FLOW_OK);
  fail_unless (buffer_alloc_pt1_called == TRUE);
  gst_buffer_unref (buffer);

  gst_test_trans_free (trans);
}

GST_END_TEST;

static gboolean set_caps_1_called;

static gboolean
set_caps_1 (GstBaseTransform * trans, GstCaps * incaps, GstCaps * outcaps)
{
  GstCaps *caps;

  GST_DEBUG_OBJECT (trans, "set_caps called");

  set_caps_1_called = TRUE;

  caps = gst_caps_new_simple ("foo/x-bar", NULL);

  fail_unless (gst_caps_is_equal (incaps, caps));
  fail_unless (gst_caps_is_equal (outcaps, caps));

  gst_caps_unref (caps);

  return TRUE;
}

/* basic in-place, check if the _ip function is called, buffer should be
 * writable. we also set a setcaps function and see if it's called. */
GST_START_TEST (basetransform_chain_ip2)
{
  TestTransData *trans;
  GstBuffer *buffer;
  GstFlowReturn res;
  GstCaps *caps;

  klass_transform_ip = transform_ip_1;
  klass_set_caps = set_caps_1;

  trans = gst_test_trans_new ();
  trans->buffer_alloc = buffer_alloc_pt1;

  /* with caps buffer */
  GST_DEBUG_OBJECT (trans, "alloc without caps, size 20");

  buffer_alloc_pt1_called = FALSE;
  res = gst_pad_alloc_buffer (trans->srcpad, 0, 20, NULL, &buffer);
  fail_unless (res == GST_FLOW_OK);
  fail_unless (buffer_alloc_pt1_called == TRUE);
  fail_unless (GST_BUFFER_SIZE (buffer) == 20);
  fail_unless (GST_BUFFER_CAPS (buffer) == NULL);
  gst_buffer_unref (buffer);

  caps = gst_caps_new_simple ("foo/x-bar", NULL);

  /* with caps buffer */
  GST_DEBUG_OBJECT (trans, "alloc with caps, size 20");

  buffer_alloc_pt1_called = FALSE;
  res = gst_pad_alloc_buffer (trans->srcpad, 0, 20, caps, &buffer);
  fail_unless (res == GST_FLOW_OK);
  fail_unless (buffer_alloc_pt1_called == TRUE);
  fail_unless (GST_BUFFER_SIZE (buffer) == 20);
  fail_unless (GST_BUFFER_CAPS (buffer) == caps);
  gst_buffer_unref (buffer);

  /* first try to push a buffer without caps, this should fail */
  buffer = gst_buffer_new_and_alloc (20);

  GST_DEBUG_OBJECT (trans, "buffer without caps, size 20");

  transform_ip_1_called = FALSE;;
  transform_ip_1_writable = FALSE;;
  buffer_alloc_pt1_called = FALSE;
  set_caps_1_called = FALSE;;
  res = gst_test_trans_push (trans, buffer);
  fail_unless (res == GST_FLOW_NOT_NEGOTIATED);
  fail_unless (transform_ip_1_called == FALSE);
  fail_unless (transform_ip_1_writable == FALSE);
  fail_unless (set_caps_1_called == FALSE);
  fail_unless (buffer_alloc_pt1_called == FALSE);

  /* try to push a buffer with caps */
  GST_DEBUG_OBJECT (trans, "buffer with caps, size 20");

  buffer = gst_buffer_new_and_alloc (20);
  gst_buffer_set_caps (buffer, caps);

  transform_ip_1_called = FALSE;
  transform_ip_1_writable = FALSE;
  set_caps_1_called = FALSE;;
  buffer_alloc_pt1_called = FALSE;
  res = gst_test_trans_push (trans, buffer);
  fail_unless (res == GST_FLOW_OK);
  fail_unless (transform_ip_1_called == TRUE);
  fail_unless (transform_ip_1_writable == TRUE);
  fail_unless (set_caps_1_called == TRUE);
#ifdef FAILING_TESTS
  /* FIXME, in-place without pad-alloc, do pad-alloc on the srcpad */
  fail_unless (buffer_alloc_pt1_called == TRUE);
#endif

  buffer = gst_test_trans_pop (trans);
  fail_unless (buffer != NULL);
  fail_unless (GST_BUFFER_SIZE (buffer) == 20);
  fail_unless (GST_BUFFER_CAPS (buffer) == caps);
  gst_buffer_unref (buffer);

  /* with caps buffer */
  GST_DEBUG_OBJECT (trans, "alloc with caps, size 20");

  buffer_alloc_pt1_called = FALSE;
  res = gst_pad_alloc_buffer (trans->srcpad, 0, 20, caps, &buffer);
  fail_unless (res == GST_FLOW_OK);
  fail_unless (buffer_alloc_pt1_called == TRUE);
  gst_buffer_unref (buffer);

  GST_DEBUG_OBJECT (trans, "buffer with caps extra ref, size 20");

  buffer = gst_buffer_new_and_alloc (20);
  gst_buffer_set_caps (buffer, caps);
  /* take additional ref to make it non-writable */
  gst_buffer_ref (buffer);

  fail_unless (GST_MINI_OBJECT_REFCOUNT_VALUE (buffer) == 2);

  transform_ip_1_called = FALSE;;
  transform_ip_1_writable = FALSE;;
  buffer_alloc_pt1_called = FALSE;
  res = gst_test_trans_push (trans, buffer);
  fail_unless (res == GST_FLOW_OK);
  fail_unless (transform_ip_1_called == TRUE);
  fail_unless (transform_ip_1_writable == TRUE);
  fail_unless (buffer_alloc_pt1_called == TRUE);
  /* after push, get rid of the final ref we had */
  gst_buffer_unref (buffer);

  buffer = gst_test_trans_pop (trans);
  fail_unless (buffer != NULL);
  fail_unless (GST_BUFFER_SIZE (buffer) == 20);
  fail_unless (GST_BUFFER_CAPS (buffer) == caps);

  /* output buffer has refcount 1 */
  fail_unless (GST_MINI_OBJECT_REFCOUNT_VALUE (buffer) == 1);
  gst_buffer_unref (buffer);

  /* with caps buffer */
  GST_DEBUG_OBJECT (trans, "alloc with caps, size 20");

  buffer_alloc_pt1_called = FALSE;
  res = gst_pad_alloc_buffer (trans->srcpad, 0, 20, caps, &buffer);
  fail_unless (res == GST_FLOW_OK);
  fail_unless (buffer_alloc_pt1_called == TRUE);
  gst_buffer_unref (buffer);

  gst_caps_unref (caps);

  gst_test_trans_free (trans);
}

GST_END_TEST;

static GstStaticPadTemplate sink_template_ct1 = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("baz/x-foo")
    );

static gboolean set_caps_ct1_called;

static gboolean
set_caps_ct1 (GstBaseTransform * trans, GstCaps * incaps, GstCaps * outcaps)
{
  GstCaps *caps1, *caps2;

  GST_DEBUG_OBJECT (trans, "set_caps called");

  caps1 = gst_caps_new_simple ("baz/x-foo", NULL);
  caps2 = gst_caps_new_simple ("foo/x-bar", NULL);

  fail_unless (gst_caps_is_equal (incaps, caps1));
  fail_unless (gst_caps_is_equal (outcaps, caps2));

  set_caps_ct1_called = TRUE;;

  gst_caps_unref (caps1);
  gst_caps_unref (caps2);

  return TRUE;
}

static gboolean transform_ct_1_called;
static gboolean transform_ct_1_writable;

static GstFlowReturn
transform_ct1 (GstBaseTransform * trans, GstBuffer * in, GstBuffer * out)
{
  transform_ct_1_called = TRUE;
  transform_ct_1_writable = gst_buffer_is_writable (out);

  GST_DEBUG_OBJECT (trans, "writable: %d", transform_ct_1_writable);

  return GST_FLOW_OK;
}

static GstCaps *
transform_caps_ct1 (GstBaseTransform * trans, GstPadDirection dir,
    GstCaps * caps)
{
  GstCaps *res;

  if (dir == GST_PAD_SINK) {
    res = gst_caps_new_simple ("foo/x-bar", NULL);
  } else {
    res = gst_caps_new_simple ("baz/x-foo", NULL);
  }
  return res;
}

static gboolean
transform_size_ct1 (GstBaseTransform * trans, GstPadDirection direction,
    GstCaps * caps, guint size, GstCaps * othercaps, guint * othersize)
{
  if (direction == GST_PAD_SINK) {
    *othersize = size * 2;
  } else {
    *othersize = size / 2;
  }

  return TRUE;
}

gboolean buffer_alloc_ct1_called;

static GstFlowReturn
buffer_alloc_ct1 (GstPad * pad, guint64 offset, guint size, GstCaps * caps,
    GstBuffer ** buf)
{
  GstCaps *outcaps;

  GST_DEBUG_OBJECT (pad, "buffer_alloc called %" G_GUINT64_FORMAT ", %u, %"
      GST_PTR_FORMAT, offset, size, caps);

  buffer_alloc_ct1_called = TRUE;

  outcaps = gst_caps_new_simple ("foo/x-bar", NULL);
  fail_unless (gst_caps_is_equal (outcaps, caps));
  gst_caps_unref (outcaps);

  *buf = gst_buffer_new_and_alloc (size);
  gst_buffer_set_caps (*buf, caps);

  return GST_FLOW_OK;
}

/* basic copy-transform, check if the transform function is called,
 * buffer should be writable. we also set a setcaps function and
 * see if it's called. */
GST_START_TEST (basetransform_chain_ct1)
{
  TestTransData *trans;
  GstBuffer *buffer;
  GstFlowReturn res;
  GstCaps *incaps, *outcaps;

  sink_template = &sink_template_ct1;
  klass_transform = transform_ct1;
  klass_set_caps = set_caps_ct1;
  klass_transform_caps = transform_caps_ct1;
  klass_transform_size = transform_size_ct1;

  trans = gst_test_trans_new ();
  trans->buffer_alloc = buffer_alloc_ct1;

  incaps = gst_caps_new_simple ("baz/x-foo", NULL);
  outcaps = gst_caps_new_simple ("foo/x-bar", NULL);

#if 0
  /* without caps buffer, I think this should fail */
  GST_DEBUG_OBJECT (trans, "alloc without caps, size 20");

  buffer_alloc_ct1_called = FALSE;
  res = gst_pad_alloc_buffer (trans->srcpad, 0, 20, NULL, &buffer);
  fail_unless (res == GST_FLOW_NOT_NEGOTIATED);
  /* should not call pad-alloc because the caps and sizes are different */
  fail_unless (buffer_alloc_ct1_called == FALSE);
#endif

  /* with wrong caps buffer */
  GST_DEBUG_OBJECT (trans, "alloc with wrong caps, size 20");

  buffer_alloc_ct1_called = FALSE;
  res = gst_pad_alloc_buffer (trans->srcpad, 0, 20, outcaps, &buffer);
  fail_unless (res == GST_FLOW_NOT_NEGOTIATED);
  fail_unless (buffer_alloc_ct1_called == TRUE);

  /* with caps buffer */
  GST_DEBUG_OBJECT (trans, "alloc with caps, size 20");

  buffer_alloc_ct1_called = FALSE;
  res = gst_pad_alloc_buffer (trans->srcpad, 0, 20, incaps, &buffer);
  fail_unless (res == GST_FLOW_OK);
  /* should not call pad-alloc because the caps and sizes are different */
  fail_unless (buffer_alloc_ct1_called == FALSE);
  gst_buffer_unref (buffer);

  /* first try to push a buffer without caps, this should fail */
  buffer = gst_buffer_new_and_alloc (20);

  GST_DEBUG_OBJECT (trans, "buffer without caps");

  transform_ct_1_called = FALSE;;
  transform_ct_1_writable = FALSE;;
  set_caps_ct1_called = FALSE;;
  buffer_alloc_ct1_called = FALSE;
  res = gst_test_trans_push (trans, buffer);
  fail_unless (res == GST_FLOW_NOT_NEGOTIATED);
  fail_unless (transform_ct_1_called == FALSE);
  fail_unless (transform_ct_1_writable == FALSE);
  fail_unless (set_caps_ct1_called == FALSE);
  fail_unless (buffer_alloc_ct1_called == FALSE);

  /* try to push a buffer with caps */
  buffer = gst_buffer_new_and_alloc (20);
  gst_buffer_set_caps (buffer, incaps);

  GST_DEBUG_OBJECT (trans, "buffer with caps %" GST_PTR_FORMAT, incaps);

  transform_ct_1_called = FALSE;
  transform_ct_1_writable = FALSE;
  set_caps_ct1_called = FALSE;;
  buffer_alloc_ct1_called = FALSE;
  res = gst_test_trans_push (trans, buffer);
  fail_unless (res == GST_FLOW_OK);
  fail_unless (transform_ct_1_called == TRUE);
  fail_unless (transform_ct_1_writable == TRUE);
  fail_unless (set_caps_ct1_called == TRUE);
  fail_unless (buffer_alloc_ct1_called == TRUE);

  buffer = gst_test_trans_pop (trans);
  fail_unless (buffer != NULL);
  fail_unless (GST_BUFFER_SIZE (buffer) == 40);
  fail_unless (gst_caps_is_equal (GST_BUFFER_CAPS (buffer), outcaps));
  gst_buffer_unref (buffer);

  buffer = gst_buffer_new_and_alloc (20);
  gst_buffer_set_caps (buffer, incaps);
  /* take additional ref to make it non-writable */
  gst_buffer_ref (buffer);

  fail_unless (GST_MINI_OBJECT_REFCOUNT_VALUE (buffer) == 2);

  GST_DEBUG_OBJECT (trans, "buffer with caps %" GST_PTR_FORMAT, incaps);

  transform_ct_1_called = FALSE;;
  transform_ct_1_writable = FALSE;;
  buffer_alloc_ct1_called = FALSE;
  res = gst_test_trans_push (trans, buffer);
  fail_unless (res == GST_FLOW_OK);
  fail_unless (transform_ct_1_called == TRUE);
  fail_unless (transform_ct_1_writable == TRUE);
  fail_unless (buffer_alloc_ct1_called == TRUE);
  /* after push, get rid of the final ref we had */
  gst_buffer_unref (buffer);

  buffer = gst_test_trans_pop (trans);
  fail_unless (buffer != NULL);
  fail_unless (GST_BUFFER_SIZE (buffer) == 40);
  fail_unless (gst_caps_is_equal (GST_BUFFER_CAPS (buffer), outcaps));

  /* output buffer has refcount 1 */
  fail_unless (GST_MINI_OBJECT_REFCOUNT_VALUE (buffer) == 1);
  gst_buffer_unref (buffer);

  gst_caps_unref (incaps);
  gst_caps_unref (outcaps);

  gst_test_trans_free (trans);
}

GST_END_TEST;


static Suite *
gst_basetransform_suite (void)
{
  Suite *s = suite_create ("GstBaseTransform");
  TCase *tc = tcase_create ("general");

  suite_add_tcase (s, tc);
  tcase_add_test (tc, basetransform_chain_pt1);
  tcase_add_test (tc, basetransform_chain_pt2);
  tcase_add_test (tc, basetransform_chain_ip1);
  tcase_add_test (tc, basetransform_chain_ip2);
  tcase_add_test (tc, basetransform_chain_ct1);

  return s;
}

GST_CHECK_MAIN (gst_basetransform);