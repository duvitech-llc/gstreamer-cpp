#ifndef _GST_TEST_APP_H_
#define _GST_TEST_APP_H_

#include <iostream>
#include "gst/gst.h"

// gstreamer app structure
typedef struct _App App;
struct _App
{
  GstElement *pipeline;
  GstElement *source;
  GstElement *infilter;
  GstElement *decoder;
  GstElement *conv;
  GstElement *filter;
  GstElement *sink;  
};


typedef struct thread_info {
   pthread_t        thread_id;          /* ID returned by pthread_create() */
   pthread_mutex_t  lock;               /* lock object for thread */
   int              done;
   int              finished;
}ThreadContext;

class GstreamerTest
{
public:
    GstreamerTest();
    ~GstreamerTest();

    bool isRunning(void);
    bool StartPipeline(void);
    bool StopPipeline(void);
    bool PushBuffer(unsigned char* pData, long length); // jpeg buffer

private:
    ThreadContext   mGstreamerThreadInfo;
    bool            mInitialized {false};
};


#endif
