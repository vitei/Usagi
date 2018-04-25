#ifndef PRINTER_H
#define PRINTER_H

#include <stdio.h>
#include <stdarg.h>

void printerInit( void );
void printerTerm( void );

void printerOpen( const char* path );
void printerClose( void );

void printerWrite( const char* p, ... );

FILE* printerHandle( void );

#endif // PRINTER_H
