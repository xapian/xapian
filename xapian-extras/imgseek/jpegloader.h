/******************************************************************************
    Modifications: Copyright (C) 2009 Lemur Consulting Limited
    imgSeek ::  Fast jpeg loader
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
******************************************************************************/

#ifndef JPEGLOADER_H
#define JPEGLOADER_H

int calcScale(int width,   int height,   int target_width,   int target_height);
struct jpeg_decompress_struct loadJPEG(QImage& img, const char* path);
	
#endif  
