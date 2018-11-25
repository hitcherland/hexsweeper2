#include <game.h>

Layout::Layout( const char* sourceFile) {
    char* source_tmp = new char[ 13 + strlen( sourceFile ) ];
    FILE *f = fopen( source, "rb" );
    struct json_object *tmp;
    long length;
    char* json_string;

    sprintf( source_tmp, "layouts/%s.json", sourceFile );
    source = source_tmp;
        printf( "layout file: %s\n", source );

    if( f ) {                                    
        fseek( f, 0, SEEK_END );                 
        length = ftell( f );                     
        fseek( f, 0, SEEK_SET);                  
        json_string = (char *) malloc( length ); 
        if( json_string ) {                      
            fread( json_string, 1, length, f );  
        }                                        
        fclose( f );                             
    } else {                                     
        printf( "%s is not a file\n", source );
    }                                            

    if( json_string ) {
        //json_tokener_parse( json_string );
        //json = json_tokener_parse( json_string );
    }
    printf( "json_string: %s\n", json_string );
    free( json_string );

}
