#define until(a)    while(!(a))
#define unless(a)   if(!(a))
#define repeat      for(;;)
#define eq          ==
#define ne          !=
#define and         &&
#define or          ||
#define not         !
#define true        1
#define false       0

#define BYTERANGE   256

/* next 2 defs are different in HDM (Heavy Duty Muscat) */

#define LOF(p,c)    ((p)[(c)+1] << 8 | (p)[(c)])
#define LWIDTH      2         /* bytes in a Muscat length (3 in HDM) */

#define DATERMS     10101     /* word used to identify DA index files */
#define DARECS      23232     /* word used to identify DA record files */
#define TVSTART     (LWIDTH+1)
#define TVSIZE(p,c) (LOF(p,c)+1)
#define ILEN 4
#define L2(p,c) ((p)[(c)+1] << 8 | (p)[(c)])



