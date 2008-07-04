! 
! This program is free software; you can redistribute it and/or
! modify it under the terms of the GNU General Public License
! as published by the Free Software Foundation; either version 2
! of the License, or (at your option) any later version.
! 
subroutine fsubf(arg,res)
  implicit none
  integer arg,res
  write(*,*) 'fsubf called'
  res=arg*2
  return
end
