C 
C This program is free software; you can redistribute it and/or
C modify it under the terms of the GNU General Public License
C as published by the Free Software Foundation; either version 2
C of the License, or (at your option) any later version.
C 

      subroutine fsub3(arg,res)
      implicit none
      integer*4 arg,res
      write(*,*) 'fsub3 called'
      res=arg*4
      return
      end
