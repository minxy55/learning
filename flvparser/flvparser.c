#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

enum {
    TagType_AUDIO = 0x08,
    TagType_VIDEO = 0x09,
    TagType_SCRIPT = 0x12,
};

typedef struct {
    unsigned char tag_type;
    unsigned long data_size;
    unsigned long timestamp;
    unsigned long timestamp_extended;
    unsigned long stream_id;
    unsigned char data[0];
} FLVTag;

typedef struct {
    unsigned char frame_type : 4;
    unsigned char codec_id : 4;
    unsigned char data[0];
} FLVVideoSection;

typedef struct {
    unsigned char frame_type : 4;
    unsigned char sample_rate : 2;
	unsigned char sample_length : 1;
	unsigned char sound_type : 1;
    unsigned char data[0];
} FLVAudioSection;

typedef struct {
	unsigned char type;
} FLVScriptSection;

typedef struct {
    unsigned char flv_tag[3];
    unsigned char version;
    unsigned char type_flags_reserved : 5;
    unsigned char type_flags_audio : 1;
    unsigned char type_flags_reserved2 : 1;
    unsigned char type_flags_video : 1;
    unsigned long data_offset;
} FLVFileHeader;

static const char *filename = "./entertainment.flv";

unsigned char fop_get_be8(FILE *fp)
{
    unsigned char _tmp = 0;

    fread(&_tmp, 1, 1, fp);
    
    return _tmp;
}

unsigned short fop_get_be16(FILE *fp)
{
    unsigned short _tmp = 0;

    fread(&_tmp, 2, 1, fp);

    return (((_tmp & 0xff) << 8) | (_tmp >> 8));
}

unsigned long fop_get_be24(FILE *fp)
{
    unsigned long _tmp = 0;

    fread(&_tmp, 3, 1, fp);

    return (((_tmp & 0x000000ff) << 16) |
            ((_tmp & 0x0000ff00) << 0) |
            ((_tmp & 0x00ff0000) >> 16));
}

unsigned long fop_get_be32(FILE *fp)
{
    unsigned long _tmp = 0;

    fread(&_tmp, 4, 1, fp);

    return (((_tmp & 0x000000ff) << 24) |
            ((_tmp & 0x0000ff00) << 8) |
            ((_tmp & 0x00ff0000) >> 8) |
            ((_tmp & 0xff000000) >> 24));
}

static const char *tag_type_to_str(unsigned char tag_type)
{
    if (tag_type == TagType_AUDIO)
        return "Audio";
    else if (tag_type == TagType_VIDEO)
        return "Video";
    else if (tag_type == TagType_SCRIPT)
        return "Script";
    else
        return "";
}

static const char *audio_frame_type_to_str(unsigned char frame_type)
{
	switch(frame_type)
	{
	case 0: return "Linear PCM, platform endian";
	case 1: return "ADPCM";
	case 2: return "MP3";
	case 3: return "Linear PCM, little endian";
	case 4: return "Nellymoser 16KHz mono";
	case 5: return "Nellymoser 8KHz mono";
	case 6: return "Nellymoser";
	case 7: return "G.711 A-law logarithmic PCM";
	case 8: return "G.711 mu-law logarithmic PCM";
	case 9: return "reserved";
	case 10: return "AAC";
	case 11: return "Speex";
	case 14: return "MP3 8KHz";
	case 15: return "Device-specific sound";
	default: return "";
	}
}

static const char *audio_sample_rate_to_str(unsigned char sample_rate)
{
	switch(sample_rate)
	{
	case 0: return "5.5kHz";
	case 1: return "11kHz";
	case 2: return "22kHz";
	case 3:	return "44kHz";
	default: return "";
	}
}

static const char *video_frame_type_to_str(unsigned char frame_type)
{
	switch (frame_type)
	{
	case 1: return "Key frame(for AVC, a seekable frame)";
	case 2: return "Inter frame(for AVC, a non-seekable frame)"; 
	case 3: return "disposable inter frame(H.263 only";
	case 4: return "generated keyframe(reserved for server use only";
	case 5: return "video info/command frame";
	default: return "";
	}
}

static const char *video_codec_id_to_str(unsigned char codec_id)
{
	switch (codec_id)
	{
	case 1: return "JPEG";
	case 2: return "Sorenson H.263";
	case 3: return "Screen video";
	case 4: return "On2 VP6";
	case 5: return "On2 VP6 with alpha channel";
	case 6: return "Screen video version2";
	case 7: return "AVC";
	default: return "";
	}
}

int main(int argc, char *argv[])
{
    FLVFileHeader file_header = {0};
    unsigned long file_size = 0;
	FILE *fp = NULL;

	fp = fopen(filename, "rb");

	if (fp == NULL)
	{
        printf("failed to open file %s\n", filename);
        return -1;
    }
    
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    file_header.flv_tag[0] = fop_get_be8(fp);
    file_header.flv_tag[1] = fop_get_be8(fp);
    file_header.flv_tag[2] = fop_get_be8(fp);
    file_header.version = fop_get_be8(fp);

    unsigned char flags = fop_get_be8(fp);
    file_header.type_flags_audio = (flags & 0x04) >> 2;
    file_header.type_flags_video = (flags & 0x01) >> 0;
    file_header.data_offset = fop_get_be32(fp);
    
    printf("flv_tag: %c%c%c\n"
        "version:%d\n"
        "type_flags_audio: %d\n"
        "type_flags_video: %d\n"
        "data_offset: %ld\n",
        file_header.flv_tag[0], file_header.flv_tag[1], file_header.flv_tag[2],
        file_header.version,
        file_header.type_flags_audio,
        file_header.type_flags_video,
        file_header.data_offset);
    
    int pos = 0;
    for (pos = file_header.data_offset; pos < file_size;)
    {
        FLVTag tag = {0};
	FLVAudioSection ad = {0};
	FLVVideoSection vd = {0};
        unsigned long prevous_tag_size = 0;
	unsigned long gotbytes = 0;
	unsigned char _tmp = 0;
        
        prevous_tag_size = fop_get_be32(fp);
        printf("prevous_tag_size: %lx\n", 
            prevous_tag_size);

        tag.tag_type = fop_get_be8(fp);
        tag.data_size = fop_get_be24(fp);
        tag.timestamp = fop_get_be24(fp);
        tag.timestamp_extended = fop_get_be8(fp);
        tag.stream_id = fop_get_be24(fp);

        printf("\ttag_type: %s\n"
            "\tdata_size: %lx\n"
            "\ttimestamp: %ld\n"
            "\ttimestamp_extended: %ld\n"
            "\tstream_id: %ld\n",
            tag_type_to_str(tag.tag_type),
            tag.data_size,
            tag.timestamp,
            tag.timestamp_extended,
            tag.stream_id);

	switch(tag.tag_type)
	{
	case TagType_AUDIO:
		_tmp = fop_get_be8(fp);
		ad.frame_type = _tmp >> 4;
		ad.sample_rate = (_tmp >> 2) & 3;
		ad.sample_length = (_tmp >> 1) & 1;
		ad.sound_type = (_tmp >> 0) & 1;
		gotbytes = 1;
		printf("\t\tframe_type: %s\n"
			"\t\tsample_rate: %s\n",
			audio_frame_type_to_str(ad.frame_type),
			audio_sample_rate_to_str(ad.sample_rate));
		break;
	case TagType_VIDEO:
		_tmp = fop_get_be8(fp);
		vd.frame_type = _tmp >> 4;
		vd.codec_id = _tmp & 0x0f;
		gotbytes = 1;
		printf("\t\tframe_type: %s\n"
			"\t\tcodec_id: %s\n",
			video_frame_type_to_str(vd.frame_type),
			video_codec_id_to_str(vd.codec_id));
		break;
	default:
		break;
	}

        fseek(fp, tag.data_size - gotbytes, SEEK_CUR);
        pos += 88;
        pos += tag.data_size;
    }

	fclose(fp);
}

