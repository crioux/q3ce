#ifndef __INC_Q_PARSE_H
#define __INC_Q_PARSE_H

void Com_BeginParseSession( const char *filename );
void Com_EndParseSession( void );
int Com_GetCurrentParseLine( void );
void Com_ScriptError( const char *msg, ... );
void Com_ScriptWarning( const char *msg, ... );
void Com_UngetToken( void );
const char *Com_Parse( const char *(*data_p) );
const char *Com_ParseOnLine( const char *(*data_p) );
void Com_MatchToken( const char *(*buf_p), const char *match, qboolean warning =qfalse);
void Com_SkipBracedSection( const char *(*program) );
void Com_SkipRestOfLine ( const char *(*data) );
const char *Com_ParseRestOfLine( const char *(*data_p) );
gfixed Com_ParseFloat( const char *(*buf_p) );
int Com_ParseInt( const char *(*buf_p) );
void Com_Parse1DMatrix( const char *(*buf_p), int x, gfixed *m );
void Com_Parse2DMatrix( const char *(*buf_p), int y, int x, gfixed *m );
void Com_Parse3DMatrix( const char *(*buf_p), int z, int y, int x, gfixed *m );

#endif

