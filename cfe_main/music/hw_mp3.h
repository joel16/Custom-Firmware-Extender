#ifndef __HW__MP3_H__
#define __HW__MP3_H__
#define OUTPUT_BUFFER_SIZE	(1152*2*4) /* Must be an integer multiple of 4 + of sample_per_frame (1152). */
int hw_mp3_play();
int SeekNextFrame(SceUID fd);
int HW_MP3PlayFile(SceSize args, void *argp);
#endif
