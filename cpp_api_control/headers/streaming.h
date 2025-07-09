#ifndef SERVER_H 
#define SERVER_H

#include <iostream> 
#include <cstdlib>
#include <stdexcept> 
#include <gst/gst.h> 
#include <gst/rtsp-server/rtsp-server.h>
#include "crow.h"

typedef struct _CustomData {
    gboolean is_live; 
    GstElement *pipeline; 
    GMainLoop *loop; 
} CustomData; 

static void cb_messsage(GstBus *bus, GstMessage *msg, CustomData *data) {
  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_ERROR: {
      GError *err;
      gchar *debug;

      gst_message_parse_error (msg, &err, &debug);
      g_print ("Error: %s\n", err->message);
      g_printerr("Debugging info: %s\n", debug); 
      g_error_free (err);
      g_free (debug);

      gst_element_set_state (data->pipeline, GST_STATE_READY);
      g_main_loop_quit (data->loop);
      break;
    }
    case GST_MESSAGE_EOS:
      /* end-of-stream */
      gst_element_set_state (data->pipeline, GST_STATE_READY);
      g_main_loop_quit (data->loop);
      break;
    case GST_MESSAGE_BUFFERING: {
      gint percent = 0;

      /* If the stream is live, we do not care about buffering. */
      if (data->is_live) break;

      gst_message_parse_buffering (msg, &percent);
      g_print ("Buffering (%3d%%)\r", percent);
      /* Wait until buffering is complete before start/resume playing */
      if (percent < 100)
        gst_element_set_state (data->pipeline, GST_STATE_PAUSED);
      else
        gst_element_set_state (data->pipeline, GST_STATE_PLAYING);
      break;
    }
    case GST_MESSAGE_CLOCK_LOST:
      /* Get a new clock */
      gst_element_set_state (data->pipeline, GST_STATE_PAUSED);
      gst_element_set_state (data->pipeline, GST_STATE_PLAYING);
      break;
    default:
      /* Unhandled message */
      break;
    }
}

static void start_streaming() {
  GstElement *pipeline;
  GstElement *videoconvert; 
  GstElement *decoder; 
  GstElement *ximagesink; 
  GstElement *src; 
  GstElement *tcpserversink; 
  GstElement *sink;
  GstElement *v4l; 

  GstBus *bus;
  GstStateChangeReturn ret;
  GMainLoop *main_loop;
  CustomData data;
  
  /* Initialize GStreamer */
  // gst_init (&argc, &argv);
  gst_init();

  /* Initialize our data structure */
  memset (&data, 0, sizeof (data));

  /* Build the pipeline */
  pipeline = gst_pipeline_new("cam-pipeline"); 
  if (!pipeline) {
    g_printerr("Failed to create pipeline\n"); 
    return -1; 
  } 


  /* Testing Streaming: gst-launch-1.0 -v videotestsrc ! x264enc tune=zerolatency bitrate=500 speed-preset=superfast ! rtph264pay ! udpsink port=5000 host=$HOST */ 
  videoconvert = gst_element_factory_make("videoconvert", "videoconvert"); 
  ximagesink = gst_element_factory_make("ximagesink", "ximagesink");
  v4l = gst_element_factory_make("v4l2src", "v4l2src"); 
  tcpserversink = gst_element_factory_make("tcpserversink", "tcpserversink");

  g_object_set(v4l, "device", "/dev/video0", NULL); 
  g_object_set(tcpserversink, "host", "127.0.0.1", NULL); 
  g_object_set(tcpserversink, "port", "5000", NULL);

  gst_bin_add_many(GST_BIN(pipeline), v4l, videoconvert, ximagesink, NULL); 
  gst_element_link_many(v4l, videoconvert, ximagesink, NULL); 

  if (gst_element_link_many(v4l, videoconvert, ximagesink, NULL) == true) {
    gst_object_unref(pipeline); 
    return -1; 
  }  

  if (pipeline == NULL) {
    g_printerr ("Unable to launch pipeline err"); 
    return -1; 
  } 

  /* Start playing */
  ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (pipeline);
    return -1;
  } else if (ret == GST_STATE_CHANGE_NO_PREROLL) {
    data.is_live = TRUE;
  }

  bus = gst_element_get_bus(pipeline); 

  main_loop = g_main_loop_new (NULL, FALSE);
  data.loop = main_loop;
  data.pipeline = pipeline;

  gst_bus_add_signal_watch (bus);
  g_signal_connect (bus, "message", G_CALLBACK (cb_message), &data);

  g_main_loop_run (main_loop);

  /* Free resources */
  g_main_loop_unref (main_loop);
  gst_object_unref (bus);
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);

  return 0;
}


#endif