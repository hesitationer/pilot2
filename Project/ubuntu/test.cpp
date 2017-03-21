#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <HAL/rk32885.1/Apcap.h>
#include <YAL/fec/reciever.h>
#include <YAL/fec/frame.h>
#include <SDL2/SDL.h>
#include <vector>
#include <pthread.h>

extern "C"
{
#include <libavcodec/avcodec.h>	
}

using namespace androidUAV;
using namespace std;

int screen_w = 640;
int screen_h = 360;
const int pixel_w = 640;
const int pixel_h = 360;
uint32_t pixformat= SDL_PIXELFORMAT_IYUV;
uint8_t buffer[pixel_w*pixel_h*2] = {0};
const int INBUF_SIZE = 4096000;
uint8_t inbuf[INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];

/*
uint8_t filebuf[INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];
int filebuf_pos = 0;
int last_check = 4;

FILE * f = fopen("/home/my12doom/sendfile/F.h264", "rb");

int get_h264_frame(void *buf)
{
	if (!f)
		return 0;

	if (feof(f))
		exit(0);

	while(1)
	{
		if (filebuf_pos < sizeof(filebuf)-1024)
		{
			int block = fread(filebuf+filebuf_pos, 1, 1024, f);
			filebuf_pos += block;

		}
		else
		{
			filebuf_pos = 4;
			last_check = 4;
			exit(-1);
		}

		for(; last_check<filebuf_pos-4; last_check++)
		{
			if (filebuf[last_check] == 0 && filebuf[last_check+1] == 0 && filebuf[last_check+2] == 0 && filebuf[last_check+3] == 1)
			{
				int size = last_check;
				memcpy(buf, filebuf, size);
				memmove(filebuf, filebuf+size, filebuf_pos-size);
				filebuf_pos -= size;
				last_check = 4;

				return size;
			}
		}

	}

	return -1;
}

*/


class cb : public IFrameReciever
{
public:
	cb()
	{
		pthread_mutex_init(&_cs, NULL);
	}
	~cb()
	{
		pthread_mutex_destroy(&_cs);
	}
	int handle_event()
	{
		return 0;
	}
	int handle_frame(const frame &f)
	{
 		printf("frame.v=%d, %d/%d bytes\n", f.integrality, *(int*)f.payload, f.payload_size);
		frame * _frame = clone_frame(&f);

		pthread_mutex_lock(&_cs);

		frames.push_back(_frame);

		pthread_mutex_unlock(&_cs);
		return 0;
	}
	frame * get_frame()
	{
		pthread_mutex_lock(&_cs);

		if (frames.size() == 0)
		{
			pthread_mutex_unlock(&_cs);
			return NULL;
		}

		frame * f = frames[0];
		frames.erase(frames.begin());

		pthread_mutex_unlock(&_cs);

		return f;
	}
	pthread_mutex_t _cs;
	vector<frame*> frames;
};


int main(int argc,char** argv)
{
	avcodec_register_all();
	AVCodec * codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    AVFrame *picture = av_frame_alloc();
    AVCodecContext *c = avcodec_alloc_context3(codec);
    if(codec->capabilities&CODEC_CAP_TRUNCATED)
    	c->flags|= CODEC_FLAG_TRUNCATED; /* we do not send complete frames */
    c->thread_count = 4;
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "could not open codec\n");
        exit(1);
    }




	if(SDL_Init(SDL_INIT_VIDEO)) {
		printf( "Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}

	SDL_Window *screen = SDL_CreateWindow("Simplest Video Play SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_w, screen_h,SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
	if(!screen) {
		printf("SDL: could not create window - exiting:%s\n",SDL_GetError());
		return -1;
	}

	SDL_Renderer* sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
	SDL_Texture* sdlTexture = SDL_CreateTexture(sdlRenderer,pixformat, SDL_TEXTUREACCESS_STREAMING,pixel_w,pixel_h);


	cb * frame_cache = new cb();
	reciever *rec = new reciever(frame_cache);

	APCAP_RX rx("wlan0", 0);

	memset(buffer, 0x80, sizeof(buffer));
	AVPacket avpkt;
	av_init_packet(&avpkt);
	FILE *h264 = fopen("out.h264", "wb");
	while(1)
	{
		SDL_Event event;
		event.type = 0;
		SDL_PollEvent(&event);

		if(event.type==SDL_WINDOWEVENT)
			SDL_GetWindowSize(screen,&screen_w,&screen_h);
		else if(event.type==SDL_QUIT)
			break;
		else
		{
			// feed reciever with pcap packets
			uint8_t data[4096];
			int size = 0;
			while((size=rx.read(data, 4096)) > 0)
			{
				//printf("\r%d", i++);
				//fflush(stdout);
				rec->put_packet(data, size);
			}

			frame * f = frame_cache->get_frame();
			if (!f)
			{
				usleep(10000);
				continue;
			}

			if (!f->integrality)
			{
				release_frame(f);
				continue;
			}


			avpkt.size = *(int*)f->payload;
			avpkt.data = (uint8_t*)f->payload+4;

			fwrite(avpkt.data, 1, avpkt.size, h264);

			while (avpkt.size > 0) 
			{
				static int frame = 0;
				int got_picture = 0;
	            int len = avcodec_decode_video2(c, picture, &got_picture, &avpkt);
	            if (len < 0) {
	                fprintf(stderr, "Error while decoding frame %d\n", frame);
	                break;
	            }
	            if (got_picture) {
	                printf("showing frame %3d\n", frame);
	                fflush(stdout);

					//show_picture(picture);
					SDL_Rect sdlRect = {0, 0, screen_w, screen_h};
					memcpy(buffer, picture->data[0], pixel_w*pixel_h);
					//memcpy(buffer+pixel_w*pixel_h, picture->data[1], pixel_w*pixel_h/4);
					//memcpy(buffer+pixel_w*pixel_h*5/4, picture->data[2], pixel_w*pixel_h/4);

					SDL_UpdateTexture( sdlTexture, NULL, buffer, pixel_w);
					SDL_RenderClear( sdlRenderer );
					SDL_RenderCopy( sdlRenderer, sdlTexture, NULL, &sdlRect);
					SDL_RenderPresent( sdlRenderer );

					frame++;
	            }
	            avpkt.size -= len;
	            avpkt.data += len;
	        }

	        release_frame(f);
		}
	}

	SDL_Quit();

	return 0;
}
