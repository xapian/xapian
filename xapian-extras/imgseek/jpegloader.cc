/******************************************************************************
    Modifications: Copyright (C) 2009 Lemur Consulting Limited

    imgSeek ::  Fast JPEG loader
    ---------------------------------------
    begin                : January 23, 2005 
    email                : nieder|at|mail.ru

    Copyright (C) 2005 Ricardo Niederberger Cabral
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
******************************************************************************

Reference implementations:

http://mail.gnome.org/archives/nautilus-list/2001-April/msg00451.html
digikam's digikam/kioslave/DIGIKAMTHUMBNAIL.CPP

******************************************************************************/

#define XMD_H   // avoid typedef name clash with libjpeg

/* QT Includes */
#include <qimage.h>
#include <qfile.h>
//#include <qdir.h>
//#include <qwmatrix.h>

#include <iostream>
// NOTE: when running build-ext.sh (auto swig wrappers) this namespace line has to be commented
using namespace std;

/* ANSI C Includes */
extern "C"
{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <jpeglib.h>
  //#include <io.h>   /// only needed on Win32
#include <sys/types.h>
}

#include "jpegloader.h"

struct myjpeg_error_mgr : public jpeg_error_mgr
{
    jmp_buf setjmp_buffer;
};

extern "C"
{
    static void myjpeg_error_exit(j_common_ptr cinfo)
    {
        myjpeg_error_mgr* myerr =
            (myjpeg_error_mgr*) cinfo->err;

        char buffer[JMSG_LENGTH_MAX];
        (*cinfo->err->format_message)(cinfo, buffer);
        cout << buffer << endl;
        longjmp(myerr->setjmp_buffer, 1);
    }
}

int calcScale(int width, int height, int target_width, int target_height) {
	if (width/8 > target_width && height/8 > target_height)
		return 8;
	if (width/4 > target_width && height/4 > target_height)
		return 4;
	if (width/2 > target_width && height/2 > target_height)
		return 2;
	return 1;
}

struct jpeg_decompress_struct loadJPEG(QImage& img, const char* path) {
    struct jpeg_decompress_struct cinfo;
    struct myjpeg_error_mgr jerr;
   
    FILE* inputFile=fopen(QFile::encodeName(QString(path)), "rb");
    if(!inputFile)
        return cinfo;

    // JPEG error handling - thanks to Marcus Meissner
    cinfo.err             = jpeg_std_error(&jerr);
    cinfo.err->error_exit = myjpeg_error_exit;
    
    if (setjmp(jerr.setjmp_buffer)) {
        jpeg_destroy_decompress(&cinfo);
        fclose(inputFile);
        return cinfo;
    }
		
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, inputFile);
    jpeg_read_header(&cinfo, TRUE);

    cinfo.scale_num = 1;
    cinfo.scale_denom = calcScale(cinfo.image_width, cinfo.image_height, 128,128);

    // Create QImage
    cinfo.dct_method = JDCT_FASTEST;
    cinfo.do_fancy_upsampling = FALSE;
    
    jpeg_start_decompress(&cinfo);

    switch(cinfo.output_components) {
    case 3:
    case 4:
        img.create( cinfo.output_width, cinfo.output_height, 32 );
        break;
    case 1: // B&W image
        img.create( cinfo.output_width, cinfo.output_height,
                    8, 256 );
        for (int i=0; i<256; i++)
            img.setColor(i, qRgb(i,i,i));
        break;
    default:
        return cinfo;
    }
    uchar** lines = img.jumpTable();
    while (cinfo.output_scanline < cinfo.output_height)
        jpeg_read_scanlines(&cinfo, lines + cinfo.output_scanline,
                            cinfo.output_height);
    jpeg_finish_decompress(&cinfo);

    // Expand 24->32 bpp
    if ( cinfo.output_components == 3 ) {
        for (uint j=0; j<cinfo.output_height; j++) {
            uchar *in = img.scanLine(j) + cinfo.output_width*3;
            QRgb *out = (QRgb*)( img.scanLine(j) );

            for (uint i=cinfo.output_width; i--; ) {
                in-=3;
                out[i] = qRgb(in[0], in[1], in[2]);
            }
        }
    }

    jpeg_destroy_decompress(&cinfo);
    fclose(inputFile);

    return cinfo;
}
