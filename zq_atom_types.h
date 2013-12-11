#ifndef ZQ_ATOM_TYPES_H
#define ZQ_ATOM_TYPES_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif // c++

#define BE_FOURCC( ch0, ch1, ch2, ch3 ) \
	( (uint32_t)(unsigned char)(ch0) | \
	  ( (uint32_t)(unsigned char)(ch1) << 8 ) | \
	  ( (uint32_t)(unsigned char)(ch2) << 16 ) | \
	  ( (uint32_t)(unsigned char)(ch3) << 24 ) )

	// top pkt atom type, atoms pkt
#define ZQ_ATOMS_TYPE_AUDIO	BE_FOURCC('A','U','D','I')
#define ZQ_ATOMS_TYPE_VIDEO	BE_FOURCC('V','I','D','E')

	// sync pkt
#define ZQ_ATOM_TYPE_SYNC BE_FOURCC('S','Y','N','C')
	struct zq_atom_sync_data
	{
		char* sync_data;
	};

#define CONST_ATOM_SYNC "\020\000\000\000SYNCZZZZZZZZ"
		

	// must bigger than audio_header and video_header
	struct zq_atom_media_header_data
	{
		char reserved[64];
	};
#define ZQ_ATOM_TYPE_AUDIO_HEADER BE_FOURCC('A','U','H','D')
	struct zq_atom_audio_header_data
	{
		char stream_codec_type; // 0x0f (AAC)
		char reserved[3];

		unsigned int id;		// stream id
		int channels;
		int bitspersample;
		int sampling_rate;

		unsigned int pts;	// ms
		//unsigned int pts;	// freq 45KHz
	};

#define ZQ_ATOM_TYPE_VIDEO_HEADER BE_FOURCC('V','I','H','D')
	struct zq_atom_video_header_data
	{
		char stream_codec_type; // 0x1b (H264)
		char frame_type;
		char reserved[2];

		unsigned int id;		// stream id
		int width;
		int height;

		//unsigned int pts;	// ms
		unsigned int pts;	// freq 45KHz
		unsigned int dts;	// freq 45KHz
	};

#define ZQ_ATOM_TYPE_DATA BE_FOURCC('D','A','T','A')
// all data

#define ZQ_ATOM_SPEEX_DATA BE_FOURCC('S','P','E','X')
	struct zq_atom_speex_header
	{
		/** 对于speex, 总是使用 8k, 16bits, mono, 并且每个包 20ms(160个samples)
		 */
		short len1;	// 包长度
		short reserved;	// 
		unsigned int stamp;	// 时间戳, 使用 45KHz
	};

#ifdef __cplusplus
}
#endif

#endif // zq_atom_types.h

