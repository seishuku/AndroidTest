#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <android/log.h>
#include <sys/time.h>
#include <time.h>
#include "android_native_app_glue.h"
#include "font.h"

#define LOG(prio, ...) ((void)__android_log_print(prio, "Android App", __VA_ARGS__))

double fTime=0.0, fTimeStep=0.0;
float fps=0.0;

double GetClock(void)
{
	struct timespec ts;

	if(!clock_gettime(CLOCK_MONOTONIC, &ts))
		return ts.tv_sec+(double)ts.tv_nsec/1000000000.0;

	return 0.0;
}

struct
{
	int32_t initialFormat;
	bool running;
	float x, y;
	uint32_t width, height;
} appState;

static int32_t app_handle_input(struct android_app *app, AInputEvent *event)
{
	switch(AInputEvent_getType(event))
	{
		case AINPUT_EVENT_TYPE_MOTION:
			appState.x=AMotionEvent_getX(event, 0);
			appState.y=AMotionEvent_getY(event, 0);
			return 1;

		case AINPUT_EVENT_TYPE_KEY:
			LOG(ANDROID_LOG_INFO, "Key event: action=%d keyCode=%d metaState=0x%x", AKeyEvent_getAction(event), AKeyEvent_getKeyCode(event), AKeyEvent_getMetaState(event));
			break;

		default:
			break;
	}

	return 0;
}

static void app_handle_cmd(struct android_app *app, int32_t cmd)
{
	switch(cmd)
	{
		case APP_CMD_INIT_WINDOW:
			appState.initialFormat=ANativeWindow_getFormat(app->window);

			appState.width=ANativeWindow_getWidth(app->window);
			appState.height=ANativeWindow_getHeight(app->window);

			ANativeWindow_setBuffersGeometry(app->window, appState.width, appState.height, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM);

			appState.running=true;
			break;

		case APP_CMD_TERM_WINDOW:
			appState.running=false;

			ANativeWindow_setBuffersGeometry(app->window, ANativeWindow_getWidth(app->window), ANativeWindow_getHeight(app->window), appState.initialFormat);
			break;

		case APP_CMD_LOST_FOCUS:
			appState.running=false;
			break;
	}
}

void point(ANativeWindow_Buffer *buffer, uint32_t x, uint32_t y, float c[3])
{
	if(x<1)
		return;
	if(x>buffer->width-1)
		return;
	if(y<1)
		return;
	if(y>buffer->height-1)
		return;

	int i=4*(y*buffer->width+x);

	((uint8_t *)buffer->bits)[i+0]=(unsigned char)(c[2]*255.0f)&0xFF;
	((uint8_t *)buffer->bits)[i+1]=(unsigned char)(c[1]*255.0f)&0xFF;
	((uint8_t *)buffer->bits)[i+2]=(unsigned char)(c[0]*255.0f)&0xFF;
}

unsigned char buffer1[320*240], buffer2[320*240];

uint8_t FireRed[256]=
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,2,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,6,6,6,7,7,8,8,8,9,9,10,10,11,
	11,12,12,13,13,14,14,15,15,16,17,17,18,18,19,20,20,21,22,22,23,23,24,25,25,26,27,27,28,29,30,30,31,32,32,33,34,34,35,36,36,37,38,38,39,40,40,41,41,42,43,43,44,44,45,46,46,47,47,48,48,49,49,50,
	50,51,51,52,52,52,53,53,54,54,54,55,55,55,56,56,56,57,57,57,57,58,58,58,58,59,59,59,59,59,60,60,60,60,60,60,60,61,61,61,61,61,61,61,61,61,61,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,
	62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62
};

uint8_t FireGreen[256]=
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,
	3,3,3,3,3,3,4,4,4,4,4,4,5,5,5,5,5,6,6,6,6,7,7,7,7,8,8,8,8,9,9,9,9,10,10,10,11,11,11,12,12,12,13,13,13,14,14,14,15,15,15,16,16,16,17,17,18,18,18,19,19,19,20,20,
	21,21,21,22,22,23,23,23,24,24,25,25,26,26,26,27,27,28,28,29,29,29,30,30,31,31,32,32,32,33,33,34,34,35,35,35,36,36,37,37,37,38,38,39,39,40,40,40,41,41,42,42,42,43,43,43,44,44,45,45,45,46,46,46,
	47,47,47,48,48,48,49,49,49,50,50,50,50,51,51,51,52,52,52,52,53,53,53,53,54,54,54,54,55,55,55,55,55,56,56,56,56,56,57,57,57,57,57,57,58,58,58,58,58,58,59,59,59,59,59,59,59,59,60,60,60,60,60,60
};

uint8_t FireBlue[256]=
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5,5,
	5,5,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,7,7,8,8,8,8,8,8,8,8,8,8,9,9,9,9,9,9,9,9,9,10,10,10,10,10,10,10,10,11,11,11,11,11
};

#define BUFFER_WIDTH 320
#define BUFFER_HEIGHT 240

uint8_t buffer1[BUFFER_WIDTH*BUFFER_HEIGHT], buffer2[BUFFER_WIDTH*BUFFER_HEIGHT];

static void render(struct android_app *app)
{

	ANativeWindow_Buffer buffer;

	if(app->window==NULL)
	{
		LOG(ANDROID_LOG_WARN, "No has window!\n");
		return;
	}

	if(ANativeWindow_lock(app->window, &buffer, NULL)<0)
	{
		LOG(ANDROID_LOG_WARN, "Could not lock window...\n");
		return;
	}

	memset(buffer.bits, 0, sizeof(uint32_t)*buffer.width*buffer.height);

	for(uint32_t i=0;i<BUFFER_WIDTH*4;i++)
		buffer1[rand()%(BUFFER_WIDTH*4)]=rand()%255;

	for(uint32_t y=2;y<BUFFER_HEIGHT-1;y++)
	{
		for(uint32_t x=1;x<BUFFER_WIDTH-1;x++)
		{
			buffer2[y*BUFFER_WIDTH+x]=
				(
				buffer1[(y-1)*BUFFER_WIDTH+(x-1)]+
				buffer1[(y+1)*BUFFER_WIDTH+x]+
				buffer1[(y-2)*BUFFER_WIDTH+(x+1)]+
				buffer1[(y-2)*BUFFER_WIDTH+x]
				)/4;
		}
	}

	for(uint32_t y=0;y<BUFFER_HEIGHT;y++)
	{
		int flipy=BUFFER_HEIGHT-1-y;

		for(uint32_t x=0;x<BUFFER_WIDTH;x++)
		{
			point(&buffer,
				  x+(buffer.width/2)-(BUFFER_WIDTH/2),
				  y+buffer.height-BUFFER_HEIGHT-100,
			(float[])
			{
				(FireBlue[buffer2[flipy*BUFFER_WIDTH+x]]<<2)/255.0f,
				(FireGreen[buffer2[flipy*BUFFER_WIDTH+x]]<<2)/255.0f,
				(FireRed[buffer2[flipy*BUFFER_WIDTH+x]]<<2)/255.0f
			});
		}
	}

	memcpy(buffer1, buffer2, BUFFER_WIDTH*BUFFER_HEIGHT);

	Font_Print(&buffer, 0, buffer.height-100, "FPS:%0.1f\nFrame time:%0.5fms", fps, fTimeStep*1000.0f);

	ANativeWindow_unlockAndPost(app->window);
}

void android_main(struct android_app *app)
{
	app->userData=&appState;
	app->onAppCmd=app_handle_cmd;
	app->onInputEvent=app_handle_input;

	memset(buffer1, 0, BUFFER_WIDTH*BUFFER_HEIGHT);
	memset(buffer2, 0, BUFFER_WIDTH*BUFFER_HEIGHT);


	while(1)
	{
		int ident, events;
		struct android_poll_source *source;

		while((ident=ALooper_pollAll(appState.running?0:-1, NULL, &events, (void **)&source))>=0)
		{
			// Process this event.
			if(source!=NULL)
				source->process(app, source);

			// Check if we are exiting.
			if(app->destroyRequested!=0)
				return;
		}

		if(appState.running)
		{
			static float avgfps=0.0f;

			double StartTime=GetClock();
			render(app);

			fTimeStep=(float)(GetClock()-StartTime);
			fTime+=fTimeStep;
			avgfps+=1.0f/fTimeStep;

			// Average over 100 frames
			static uint32_t Frames=0;
			if(Frames++>100)
			{
				fps=avgfps/Frames;
				avgfps=0.0f;
				Frames=0;
			}
		}
	}
}