#ifndef __MP3DECODER_H__
#define __MP3DECODER_H__

#include <mpg123.h>
#include <mpglib.h>

class MP3decoder
{
    public:
        MP3decoder();
        ~MP3decoder();

    public:
        bool _stdcall       OpenStream( cosnt char* pcfilename, int* isample, int* ichannel, int* ibyte, void* ptSettings, unsigned* bufsize );
        unsigned _stdcall   Decode(unsigned char* pbout);
        bool _stdcall       CloseStream();
        unsigned _stdcall   GetTotalTime();
        unsigned _stdcall   GetPos();
        bool _stdcall       SetPos( unsigned aiPosMS );

    public:
        static unsigned _stdcall GetFileTotalTime( const char* pcfilename );


    private:
        #define BUFFER_SIZE 16384
        #define REST_B_SIZE 8192
        
    private:
        unsigned    pos;
        struct \
        mpstr       mp;
        char        buf[BUFFER_SIZE];
        char        pbRestBuf[REST_B_SIZE];
        char        out[REST_B_SIZE];
        unsigned    outsize;
        FILE*       fin;

        unsigned    dwBufSize;
        unsigned    dwRestBufSize;

        VBRTAGDATA  vbrtag;
        unsigned    length;
        unsigned    nInFileSize;
        unsigned    nBytes;
        bool        hasVbrtag;
        
        bool        seeked;  
        bool        bfeof;
};
#endif /// of __MP3DECODER_H__