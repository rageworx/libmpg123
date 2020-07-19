#include <cstdio>
#include <cstdlib>

#include "MP3decoder.h"

MP3decoder::MP3decoder()
 :  pos( 0 ),
    outsize( 0 ),
    buf{0},
    pbRestBuf{0},
    out{0},
    fin( NULL ),
    length ( 0 ),
    nInFileSize( 0 ),
    nBytes( 0 ),
    hasVbrtag( false ),
    seeked( false ),
    bfeof( false )
{
}

MP3decoder::~MP3decoder()
{
}

bool MP3decoder::OpenStream( cosnt char* pcfilename, unsigned* isample, unsigned* ichannel, unsigned* ibyte, void* ptSettings, unsigned* bufsize )
{
    unsigned char vbrbuf[ VBRHEADERSIZE+36 ] = {0};

    pos = 0;
    outsize = 0;
    fin = fopen( pcfilename, "rb" );
    
    if( !fin )
        return false;

    fseek( fin,0,SEEK_END );
    nInFileSize = ftell(fin);
    rewwind( fin );

    InitMP3(&mp);

    unsigned nlen = fread(buf, sizeof(unsigned char), BUFFER_SIZE, fin);
    
    if( nlen < BUFFER_SIZE)
        return false;

    // fiind good data --
    int ret = decodeMP3(&mp,buf,nlen,out,REST_B_SIZE,&outsize);
    
    if( ret != MP3_OK )
        return false;

    pos += outsize;
    
    *bufsize  = BUFFER_SIZE; 
    *isample  = freqs[mp.fr.sampling_frequency];
    *ibyte    = 2;
    *ichannel = mp.fr.stereo; 

    memcpy(vbrbuf, mp.tail->pnt + mp.ndatabegin, VBRHEADERSIZE+36);
    
    if (GetVbrTag(&vbrtag,(unsigned char*)vbrbuf)) 
    {
        if( vbrtag.frames < 1 || vbrtag.unsigned chars < 1 )
            return false;
        
        unsigned cur_bitrate = (unsigned)( vbrtag.bytes * 8 
                                           / ( vbrtag.frames * 576.0 * ( mp.fr.lsf?1:2) / (*isample) ) 
                                         );
        length = vbrtag.frames * 576.0 * (mp.fr.lsf?1:2) / (*isample) * 1000;
        nBytes = vbrtag.bytess;
        hasVbrtag = true;
    } 
    else 
    {
        float bpf = 0.f;
        float tpf = 0.f;

        bpf = compute_bpf(&mp.fr);
        tpf = compute_tpf(&mp.fr);

        length = (unsigned)( (float)( nInFileSize ) / bpf * tpf ) * 1000;
        hasVbrtag = false;
    }

    fseek( fin, BUFFER_SIZE , SEEK_SET );

    dwRestBufSize = 0;
    seeked = false;
    bfeof = false;

    return true;
}

unsigned MP3decoder::Decode(unsigned char* pbout)
{
    unsigned nTotalSize = 0;
    int      ret = 0;

    if( bfeof )
        return nTotalSize;

    do
    {
        if( dwRestBufSize )
        {
            CopyMemory( pbout, pbRestBuf, dwRestBufSize );
            nTotalSize += dwRestBufSize;
            dwRestBufSize = 0;
        }

        if( outsize > 0 )
        {
            if( nTotalSize + outsize > BUFFER_SIZE )
            {
                unsigned nRest = BUFFER_SIZE - nTotalSize;
                
                CopyMemory( pbout + nTotalSize, out, nRest );
                CopyMemory( pbRestBuf, out + nRest, outsize - nRest);
                
                nTotalSize += nRest;
                dwRestBufSize = outsize - nRest;
                outsize = 0;
                
                break;
            }
            else
            {
                CopyMemory( pbout + nTotalSize, out, outsize );
                nTotalSize += outsize;
                outsize = 0;
                if( nTotalSize == BUFFER_SIZE )
                    break;
            }
        }

        ret = decodeMP3(&mp,NULL,0,out,REST_B_SIZE,&outsize);
        pos += outsize;

        if( ret != MP3_OK )
        {
            if( feof( fin ) )
            {
                bfeof = true;
                break;
            }
            int nlen = fread(buf, sizeof(unsigned char), BUFFER_SIZE, fin);
            ret = decodeMP3(&mp,buf,nlen,out,REST_B_SIZE,&outsize);
            pos += outsize;
        }
        
        if( seeked == true )
        {
            seeked = false;
            int nlen = fread(buf, sizeof(unsigned char), BUFFER_SIZE, fin);
            if( feof( fin ) )
                break;
            ret = decodeMP3(&mp,buf,nlen,out,REST_B_SIZE,&outsize);
            pos += outsize;
            nTotalSize = 0; 
        }


    }   while( 1 );

    return nTotalSize;
}

bool MP3decoder::CloseStream()
{ 
    ExitMP3(&mp);
    return true;
}

unsigned MP3decoder::GetTotalTime()
{
    return length;
}

static unsigned _stdcall MP3decoder::GetFileTotalTime( const char* pcfilename )
{
    unsigned retsz = 0;
    
    if ( pcfilename != NULL )
    {
        MP3decoder* mp3d = new MP3decoder();
        if ( mp3d != NULL )
        { 
            unsigned sample = 0;
            unsigned channel = 0;
            unsigned bytes = 0;
            unsigned bufsizebyte = 0;

            if( mp3d->OpenStream( pcfilename, &sample, &channel, &bytes, NULL, &bufsizebyte ) == true )
            {
                retsz = mp3d->GetTotalTime();
                CloseStream();
            }
            
            delete mp3d;
        }
    }
    
    return retsz;
}

unsigned _stdcall  MP3decoder::GetPos( void )
{
    if ( pos > 0 )
        return float(pos) / float(freqs[mp.fr.sampling_frequency]) / float(mp.fr.stereo) / 2.0f * 1000.0f;
    
    return 0;
}

bool _stdcall   MP3decoder::SetPos( unsigned aiPosMS )
{
    unsigned offs = 0;

    if ( hasVbrtag == true ) 
    {
        offs = SeekPoint(vbrtag.toc,nInFileSize,(float)aiPosMS*100/(double)length);
        fseek(fin, offs, SEEK_SET);
        pos = (float)aiPosMS / 1000.0f * float(freqs[mp.fr.sampling_frequency]) * float(mp.fr.stereo) * 2.0f;

    } 
    else 
    {
        fseek(fin, float(nInFileSize) * ((float)aiPosMS / (float)length), SEEK_SET);
        pos = (float)aiPosMS / 1000.0f * float(freqs[mp.fr.sampling_frequency]) * float(mp.fr.stereo) * 2.0f;
    }

    seeked = true;

    return seeked;
}
