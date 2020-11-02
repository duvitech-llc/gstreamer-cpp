#include "gstPipelineApp.h"
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>
#include <iostream>
#include <stdlib.h>

#define SAMPLE_RATE 1

using namespace std;


static App s_app;

static GstFlowReturn new_sample (GstElement *sink, App *data){
    GstSample *sample;

    /* Retrieve the buffer */
    g_signal_emit_by_name (sink, "pull-sample", &sample);
    if (sample) {
        /* The only thing we do in this example is print a * to indicate a received buffer */
        g_print ("*");
        
        g_print("Retrieving sample\n") ;
        GstBuffer *buffer  = gst_sample_get_buffer(sample);   
        GstMemory* memory = gst_buffer_get_all_memory(buffer );
        GstMapInfo map_info;
        if(! gst_memory_map(memory, &map_info, GST_MAP_READ)) {            
            printf("Flow Error\r\n");  
        }else{
            // we have a nv12 buffer here map_info.data map_info.size            
            g_print("Update Image\n") ;
            if(data->updateImage != NULL){
                data->updateImage(map_info.data, map_info.size);
            }else{  
                g_print("Update Image IS NULL\n") ;
            }
            gst_memory_unmap(memory, &map_info);            
        }
        gst_memory_unref(memory);
        gst_sample_unref (sample);
        return GST_FLOW_OK;
    }
        
    return GST_FLOW_ERROR;
}

void* GstreamerThread(void* context) {
    g_print("Enter GstreamerThread\n");
    ThreadContext *pctx = (ThreadContext *) context;
    
    App *app = &s_app;
    GstCaps *outcaps;
    GstCaps *incaps;
    GstBus *bus;
    GstMessage *msg;
    GstStateChangeReturn ret;

    /* Create gstreamer elements */
    app->pipeline = gst_pipeline_new (NULL);
    g_assert (app->pipeline);
    
    app->source = gst_element_factory_make ("appsrc", NULL);
    g_assert (app->source);

    app->infilter = gst_element_factory_make ("capsfilter", NULL);
    g_assert (app->infilter);

    app->decoder = gst_element_factory_make ("jpegdec", NULL);
    g_assert (app->decoder);

    app->conv = gst_element_factory_make ("videoconvert", NULL);
    g_assert (app->conv);

    app->filter = gst_element_factory_make ("capsfilter", NULL);
    g_assert (app->filter);

    app->sink = gst_element_factory_make ("appsink", NULL);
    g_assert (app->sink);
    
    gst_bin_add_many (GST_BIN (app->pipeline), app->source, app->infilter, app->decoder, app->conv, app->filter, app->sink, NULL);
    gst_element_link_many (app->source, app->infilter, app->decoder, app->conv, app->filter, app->sink, NULL);

    // configure filter for jpegdec
    incaps = gst_caps_new_simple ("image/jpeg",
        "width", G_TYPE_INT, 640,
        "height", G_TYPE_INT, 480,
        "framerate",GST_TYPE_FRACTION, 0, 1,
        NULL);

    g_object_set (G_OBJECT (app->infilter), "caps", incaps, NULL);
    gst_caps_unref (incaps);

    // configure filter for appsink
    outcaps = gst_caps_new_simple ("video/x-raw",
        "format", G_TYPE_STRING, "NV12",
        "width", G_TYPE_INT, 640,
        "height", G_TYPE_INT, 480, NULL);

    g_object_set (G_OBJECT (app->filter), "caps", outcaps, NULL);
    g_object_set (app->sink, "emit-signals", TRUE, "caps", outcaps, NULL);

    // configure app sink    
    g_signal_connect (app->sink, "new-sample", G_CALLBACK (new_sample), app);
    gst_caps_unref (outcaps);

    g_print("Set Pipeline to playing\n") ;
    /* Start playing */
    ret = gst_element_set_state (app->pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        gst_object_unref (app->pipeline);        
    }else{
        g_print("Pipeline Running\n") ;
        /* Wait until error or EOS */
        bus = gst_element_get_bus (app->pipeline);
        msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

        /* Parse message */
        if (msg != NULL) {
            GError *err;
            gchar *debug_info;

            switch (GST_MESSAGE_TYPE (msg)) {
            case GST_MESSAGE_ERROR:
                gst_message_parse_error (msg, &err, &debug_info);
                g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
                g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
                g_clear_error (&err);
                g_free (debug_info);
                break;
            case GST_MESSAGE_EOS:
                g_print ("End-Of-Stream reached.\n");
                break;
            default:
                /* We should not reach here because we only asked for ERRORs and EOS */
                g_printerr ("Unexpected message received.\n");
                break;
            }
            gst_message_unref (msg);
        }
        g_print("Pipeline Exiting\n") ;
        /* Free resources */
        gst_object_unref (bus);
        gst_element_set_state (app->pipeline, GST_STATE_NULL);
        gst_object_unref (app->pipeline);
        
        pctx->finished = 0;
    }

    pthread_exit(NULL);
    return NULL;
}

bool GstreamerPipeline::StartPipeline(void(*fnUpdateImage)(unsigned char *, unsigned long)){
    cout << "Starting Pipeline" << endl;

    // initialize structure    
    App *app = &s_app;

    app->pipeline = NULL;
    app->source = NULL;
    app->infilter = NULL;
    app->decoder = NULL;
    app->conv = NULL;
    app->filter = NULL;
    app->sink = NULL;
    app->updateImage = NULL;

    if(fnUpdateImage != NULL){
        g_print("Assign UpdateImage Function\n");
        // set update function
        app->updateImage = fnUpdateImage;
    }

    if(mGstreamerThreadInfo.thread_id == 0){
        pthread_attr_t  threadAttr_;
        pthread_attr_init(&threadAttr_);
        pthread_attr_setdetachstate(&threadAttr_, PTHREAD_CREATE_DETACHED);
        pthread_mutex_init(&mGstreamerThreadInfo.lock, NULL);
        
        int result  = pthread_create( &mGstreamerThreadInfo.thread_id, &threadAttr_, GstreamerThread, &mGstreamerThreadInfo);
        if(result != 0){
            g_print("Failed to create GStreamer Thread\n");
            pthread_mutex_destroy(&mGstreamerThreadInfo.lock);
        }else{
            g_print("GStreamer Thread Created\n");
        }

        pthread_attr_destroy(&threadAttr_);

    }

    return true;
}

bool GstreamerPipeline::StopPipeline(void){
    cout << "Stoping Pipeline" << endl;

    // stop thread
    if(mGstreamerThreadInfo.thread_id != 0){
        // push eos to pipeline and wait for it to stop
        App *app = &s_app;
        
        g_print("Pushing EOS\n") ;
        gst_app_src_end_of_stream (GST_APP_SRC (app->source));
        struct timespec sleepTime;
        memset(&sleepTime, 0, sizeof(sleepTime));
        sleepTime.tv_nsec = 100000000;
        while (mGstreamerThreadInfo.done) {
            if(mGstreamerThreadInfo.finished==1){
	            g_print("thread is finished\n");
                break;
            }
            nanosleep(&sleepTime, NULL);
        }

        pthread_mutex_destroy(&mGstreamerThreadInfo.lock);
        memset(&mGstreamerThreadInfo, 0, sizeof(mGstreamerThreadInfo));
    }
    return true;
}

bool GstreamerPipeline::PushBuffer(unsigned char* pData, long length){
    GstFlowReturn ret;
    GstBuffer * jpegBuffer = NULL;
    GstMapInfo map;
    guint8 *data;
    
    g_print("Push JPEG Buffer\n") ;
    if(mGstreamerThreadInfo.thread_id != 0){
        // push eos to pipeline and wait for it to stop
        App *app = &s_app;
        
        jpegBuffer = gst_buffer_new_and_alloc (length);  /* Set its timestamp and duration */
        GST_BUFFER_TIMESTAMP (jpegBuffer) = gst_util_uint64_scale (1, GST_SECOND, SAMPLE_RATE);
        GST_BUFFER_DURATION (jpegBuffer) = gst_util_uint64_scale (1, GST_SECOND, SAMPLE_RATE);
        gst_buffer_map (jpegBuffer, &map, GST_MAP_WRITE);
        data = (guint8 *)map.data;

        memcpy(data, pData, length);

        gst_buffer_unmap (jpegBuffer, &map);

        /* Push the buffer into the appsrc */
        g_signal_emit_by_name ((GstSample *)app->source, "push-buffer", jpegBuffer, &ret);
        if(ret==GST_FLOW_OK){
            return true;
        }else{
            return false;
        }
        
    }
    
    return false;
    
}

bool GstreamerPipeline::isRunning(void){
    return true;
}

GstreamerPipeline::GstreamerPipeline(void){
    const gchar *nano_str;
    guint major, minor, micro, nano;
    cout << "Object is being created" << endl;
    memset(&mGstreamerThreadInfo, 0, sizeof(mGstreamerThreadInfo));

    gst_init (NULL, NULL);

    gst_version (&major, &minor, &micro, &nano);

    if (nano == 1)
        nano_str = "(CVS)";
    else if (nano == 2)
        nano_str = "(Prerelease)";
    else
        nano_str = "";

    cout << "This program is linked against GStreamer " << major << "." << minor << "." << micro << " " << nano_str << endl;

    mInitialized = true;
}

GstreamerPipeline::~GstreamerPipeline(void){
    cout << "Object is being deleted" << endl;

    if(mInitialized){
        StopPipeline();
    }

}