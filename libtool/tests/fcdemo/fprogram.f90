! 
! This program is free software; you can redistribute it and/or
! modify it under the terms of the GNU General Public License
! as published by the Free Software Foundation; either version 2
! of the License, or (at your option) any later version.
! 
program fprogram
  implicit none
  integer arg,res

  write(*,*) 'Welcome to GNU libtool Fortran demo!'
  write(*,*) 'Real programmers write in FORTRAN.'
  arg=2

  call fsub(arg,res)

  write(*,*) 'fsub returned, saying that 2 *',arg,' =',res

  if (res.eq.4) then
     write(*,*) 'fsub is ok!'
  endif

  call fsub3(arg,res)

  write(*,*) 'fsub3 returned, saying that 4 *',arg,' =',res

  if (res.eq.8) then
     write(*,*) 'fsub3 is ok!'
  endif

  stop
end
