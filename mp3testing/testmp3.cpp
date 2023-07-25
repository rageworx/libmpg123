#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>

#include <mpg123.h>
#include <mpglib.h>

#define TEST_BUFF_SIZE      (64 * 1024)
#define RENDER_BUFF_SIZE    TEST_BUFF_SIZE

int main( int argc, char** argv )
{
    if ( argc > 1 )
    {
        const char* fname = argv[1];
        if ( access( fname, 0 ) != 0 )
        {
            fprintf( stderr, "Cannot open %s\n", fname );
            return -1;
        }

        FILE* fp = fopen( fname, "rb" );
        if ( fp == nullptr )
        {
            fprintf( stderr, "Cannot open file %s\n", fname );
            return -1;
        }

        fseek( fp, 0, SEEK_END );
        size_t fsz = ftell( fp );
        rewind( fp );

        printf( "file size = %zu bytes.\n", fsz );

        // Load little buff ..
        uint8_t buff[TEST_BUFF_SIZE] = {0};
        uint8_t obuff[TEST_BUFF_SIZE] = {0};
        size_t nlen = fread( buff, 1, TEST_BUFF_SIZE, fp );
        int osz = 0;

        printf( "file loaded for %zu bytes to test.\n", nlen );

        fclose( fp );

        // test MP3.
        struct mpstr mp = {0};
        InitMP3( &mp );

        printf( "decoding test : " );

        int ret = decodeMP3( &mp, (char*)buff, nlen, 
                             (char*)obuff, TEST_BUFF_SIZE, &osz );
        if ( ret == MP3_OK )
        {
            printf( "MP3 file OK.\n" );
        }
        else
        {
            printf( "Not MP3 file, error = %d\n", ret );
        }

        ExitMP3( &mp );

        return 0;
    }
    else
    {
        fprintf( stderr, "no parameter.\n" );
    }
    return -1;
}
