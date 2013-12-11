#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>

#include <sys/types.h>
#include <sys/syscall.h>

#include "capture_slice.h"

#if 0
#else

static UsageEnvironment *_env = 0;

#define SINK_PORT 3030

#define VIDEO_WIDTH 320
#define VIDEO_HEIGHT 240
#define FRAME_PER_SEC 25
#define FPS FRAME_PER_SEC
#define KBITS 300

pid_t gettid()
{
	return syscall(SYS_gettid);
}


// 使用 webcam + x264
class WebcamFrameSource : public FramedSource
{
	capture_t *mp_capture;
	int m_started;
	void *mp_token;

public:
	WebcamFrameSource (UsageEnvironment &env)
		: FramedSource(env)
	{
		fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
		mp_capture = capture_open("/dev/video0", VIDEO_WIDTH, VIDEO_HEIGHT, FPS, KBITS);
		if (!mp_capture) {
			fprintf(stderr, "%s: open /dev/video0 err\n", __func__);
			exit(-1);
		}

		m_started = 0;
		mp_token = 0;
	}

	~WebcamFrameSource ()
	{
		fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
		
		if (m_started) {
			envir().taskScheduler().unscheduleDelayedTask(mp_token);
		}

		if (mp_capture)
			capture_close(mp_capture);
	}

protected:
	virtual void doGetNextFrame ()
	{
		if (m_started) return;
		m_started = 1;

		// 根据 fps, 计算等待时间
		double delay = 1000.0 / FRAME_PER_SEC;
		int to_delay = delay * 1000;	// us

		mp_token = envir().taskScheduler().scheduleDelayedTask(to_delay,
				getNextFrame, this);
	}

private:
	static void getNextFrame (void *ptr)
	{
		((WebcamFrameSource*)ptr)->getNextFrame1();
	}

	void dump(const void *data, int len)
	{
		unsigned char *p = (unsigned char*)data;
		for (int i = 0; i < len; i++) {
			fprintf(stderr, "%02x ", p[i]);
		}
		fprintf(stderr, "\n");
	}

	void getNextFrame1 ()
	{
		void *outbuf;
		int64_t pts;
		int outlen = capture_next_slice(mp_capture, &outbuf, &pts);
		if (outlen <= 0) {
			m_started = 0;
			fprintf(stderr, "fifo_ empty!!\n");
			return;
		}

		// save outbuf
		gettimeofday(&fPresentationTime, 0);
		fFrameSize = outlen;
		if (fFrameSize > fMaxSize) {
			fNumTruncatedBytes = fFrameSize - fMaxSize;
			fFrameSize = fMaxSize;
		}
		else {
			fNumTruncatedBytes = 0;
		}

		memmove(fTo, outbuf, fFrameSize);

		// notify
		afterGetting(this);

		m_started = 0;
	}
};

class WebcamOndemandMediaSubsession : public OnDemandServerMediaSubsession
{
public:
	static WebcamOndemandMediaSubsession *createNew (UsageEnvironment &env, FramedSource *source)
	{
		return new WebcamOndemandMediaSubsession(env, source);
	}

protected:
	WebcamOndemandMediaSubsession (UsageEnvironment &env, FramedSource *source)
		: OnDemandServerMediaSubsession(env, True) // reuse the first source
	{
		fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
		mp_source = source;
		mp_sdp_line = 0;
	}

	~WebcamOndemandMediaSubsession ()
	{
		fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
		if (mp_sdp_line) free(mp_sdp_line);
	}

private:
	static void afterPlayingDummy (void *ptr)
	{
		fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
		// ok
		WebcamOndemandMediaSubsession *This = (WebcamOndemandMediaSubsession*)ptr;
		This->m_done = 0xff;
	}

	static void chkForAuxSDPLine (void *ptr)
	{
		WebcamOndemandMediaSubsession *This = (WebcamOndemandMediaSubsession *)ptr;
		This->chkForAuxSDPLine1();
	}

	void chkForAuxSDPLine1 ()
	{
		//fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
		if (mp_dummy_rtpsink->auxSDPLine())
			m_done = 0xff;
		else {
			int delay = 100*1000;	// 100ms
			nextTask() = envir().taskScheduler().scheduleDelayedTask(delay,
					chkForAuxSDPLine, this);
		}
	}

protected:
	virtual const char *getAuxSDPLine (RTPSink *sink, FramedSource *source)
	{
		fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
		if (mp_sdp_line) return mp_sdp_line;

		mp_dummy_rtpsink = sink;
		mp_dummy_rtpsink->startPlaying(*source, 0, 0);
		//mp_dummy_rtpsink->startPlaying(*source, afterPlayingDummy, this);
		chkForAuxSDPLine(this);
		m_done = 0;
		envir().taskScheduler().doEventLoop(&m_done);
		mp_sdp_line = strdup(mp_dummy_rtpsink->auxSDPLine());
		mp_dummy_rtpsink->stopPlaying();

		return mp_sdp_line;
	}

	virtual RTPSink *createNewRTPSink(Groupsock *rtpsock, unsigned char type, FramedSource *source)
	{
		fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
		return H264VideoRTPSink::createNew(envir(), rtpsock, type);
	}

	virtual FramedSource *createNewStreamSource (unsigned sid, unsigned &bitrate)
	{
		fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
		bitrate = 500;
		return H264VideoStreamFramer::createNew(envir(), new WebcamFrameSource(envir()));
	}

private:
	FramedSource *mp_source;	// 对应 WebcamFrameSource
	char *mp_sdp_line;
	RTPSink *mp_dummy_rtpsink;
	char m_done;
};

int main (int argc, char **argv)
{
	// env
	TaskScheduler *scheduler = BasicTaskScheduler::createNew();
	_env = BasicUsageEnvironment::createNew(*scheduler);

	// rtsp server
	RTSPServer *rtspServer = RTSPServer::createNew(*_env, 8554);
	if (!rtspServer) {
		fprintf(stderr, "ERR: create RTSPServer err\n");
		::exit(-1);
	}

	// add live stream
	do {
		WebcamFrameSource *webcam_source = 0;

		ServerMediaSession *sms = ServerMediaSession::createNew(*_env, "webcam", 0, "Session from /dev/video0"); 
		sms->addSubsession(WebcamOndemandMediaSubsession::createNew(*_env, webcam_source));
		rtspServer->addServerMediaSession(sms);

		char *url = rtspServer->rtspURL(sms);
		*_env << "using url \"" << url << "\"\n";
		delete [] url;
	} while (0);

	// run loop
	_env->taskScheduler().doEventLoop();

	return 1;
}

#endif//

