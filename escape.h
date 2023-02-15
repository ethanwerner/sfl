/*
MIT License

Copyright (c) 2023 Ethan Werner

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// escape.h - ANSI escape code macros and functions


#ifndef ESCAPE_H
#define ESCAPE_H


#include <stdio.h>


#define CSI	"\x1B["


#ifdef __cplusplus
extern "C" {
#endif


#define ANSI_CURSOR_UP( n )  CSI #n "A"
void ansi_cursor_up( int n );

#define ANSI_CURSOR_DOWN( n ) CSI #n "B"
void ansi_cursor_down( int n );

#define ANSI_CURSOR_LEFT( n )  CSI #n "C"
void ansi_cursor_left( int n );

#define ANSI_CURSOR_RIGHT( n )  CSI #n "D"
void ansi_cursor_right( int n );

#define ANSI_CURSOR_LINE_UP( n ) CSI #n "E"
void ansi_cursor_line_up( int n );

#define ANSI_CURSOR_POSITION( x, y )  CSI #y ";" #x "H"
void ansi_cursor_position( int x, int y );

#define ANSI_CLEAR_SCREEN  CSI "2J"
void ansi_clear_screen( void );

#define ANSI_CLEAR_SCREEN_START CSI "1J"
void ansi_clear_screen_start( void );

#define ANSI_CLEAR_SCREEN_END CSI "0J"
void ansi_clear_screen_end( void );

#define ANSI_CLEAR_LINE CSI "2K"
void ansi_clear_line( void );

#define ANSI_CLEAR_LINE_START CSI "1L"
void ansi_clear_line_start( void );

#define ANSI_CLEAR_LINE_END  CSI "0L"
void ansi_clear_line_end( void );

#define ANSI_SCROLL_UP( n ) CSI #n "S"
void ansi_scroll_up( int n );

#define ANSI_SCROLL_DOWN( n ) CSI #n "T"
void ansi_scroll_down( int n );


#define ANSI_SGR  "m"

#define ANSI_SGR_RESET          CSI "0" ANSI_SGR
#define ANSI_SGR_BOLD	        CSI "1" ANSI_SGR
#define ANSI_SGR_UNDERLINE      CSI "4" ANSI_SGR
#define ANSI_SGR_BLINK          CSI "5" ANSI_SGR
#define ANSI_SGR_REVERSE        CSI "7" ANSI_SGR
#define ANSI_SGR_STRIKETHROUGH  CSI "9" ANSI_SGR
#define ANSI_SGR_OVERLINE       CSI "53" ANSI_SGR


#define ANSI_SGR_FG_RGB( r, g, b )  CSI "38;2;" #r ";" #g ";" #b ANSI_SGR
void ansi_sgr_fg_rgb( int r, int g, int b );

#define ANSI_SGR_BG_RGB( r, g, b )  CSI "48;2;" #r ";" #g ";" #b ANSI_SGR
void ansi_sgr_bg_rgb( int r, int g, int b );

#define ANSI_SGR_FG_256( n )  CSI "38;5;" #n ANSI_SGR
void ansi_sgr_fg_256( int r, int g, int b );

#define ANSI_SGR_BG_256( n )  CSI "48;5;" #n ANSI_SGR
void ansi_sgr_bg_256( int r, int g, int b );


#define ANSI_SGR_FG( n )         CSI "3" #n ANSI_SGR
#define ANSI_SGR_BG( n )         CSI "4" #n ANSI_SGR
#define ANSI_SGR_FG_BRIGHT( n )  CSI "9" #n ANSI_SGR
#define ANSI_SGR_BG_BRIGHT( n )  CSI "10" #n ANSI_SGR


#define FG_BLACK    FG BLACK
#define FG_RED      FG RED
#define FG_GREEN    FG GREEN
#define FG_YELLOW   FG YELLOW
#define FG_BLUE     FG BLUE
#define FG_MAGENTA  FG MAGENTA
#define FG_CYAN     FG CYAN
#define FG_WHITE    FG WHITE

#define BG_BLACK    BG BLACK
#define BG_RED      BG RED
#define BG_GREEN    BG GREEN
#define BG_YELLOW   BG YELLOW
#define BG_BLUE     BG BLUE
#define BG_MAGENTA  BG MAGENTA
#define BG_CYAN     BG CYAN
#define BG_WHITE    BG WHITE


#ifdef __cplusplus
}
#endif

#endif // ESCAPE_H


#ifdef ESCAPE_IMPLEMENTATION

void ansi_cursor_up( int n ) { fprintf( stdout, CSI "%dA", n ); }
void ansi_cursor_down( int n ) { fprintf( stdout, CSI "%dB", n ); }
void ansi_cursor_left( int n ) { fprintf( stdout, CSI "%dC", n ); }
void ansi_cursor_right( int n ) { fprintf( stdout, CSI "%dD", n ); }
void ansi_cursor_line_up( int n ) { fprintf( stdout, CSI "%dE", n ); }
void ansi_cursor_position( int x, int y ) { fprintf( stdout, CSI "%d;%dH", y, x ); }
void ansi_clear_screen( void ) { fputs( CSI "2J", stdout ); }
void ansi_clear_screen_start( void ) { fputs( CSI "1J", stdout ); }
void ansi_clear_screen_end( void ) { fputs( CSI "0J", stdout ); }
void ansi_clear_line( void ) { fputs( CSI "2L", stdout ); }
void ansi_clear_line_start( void ) { fputs( CSI "1L", stdout ); }
void ansi_clear_line_end( void ) { fputs( CSI "0L", stdout ); }
void ansi_scroll_up( int n ) { fprintf( stdout, "%dS", n ); }
void ansi_scroll_down( int n ) { fprintf( stdout, "%dT", n ); }

void ansi_sgr_fg_rgb( int r, int g, int b ) {	fprintf( stdout, CSI "38;2;%d;%d;%d" ANSI_SGR, r, g, b ); }
void ansi_sgr_bg_rgb( int r, int g, int b ) {	fprintf( stdout, CSI "48;2;%d;%d;%d" ANSI_SGR, r, g, b ); }
void ansi_sgr_fg_256( int r, int g, int b ) {	fprintf( stdout, CSI "38;5;%d;%d;%d" ANSI_SGR, r, g, b ); }
void ansi_sgr_bg_256( int r, int g, int b ) {	fprintf( stdout, CSI "48;5;%d;%d;%d" ANSI_SGR, r, g, b ); }

#endif // ESCAPE_IMPLEMENTATION