#include <math.h>
#include <stdio.h>
#define R(v){return(v);}
#define F(x,n)for(i x=n;x--;)
#define O(a,b,c,d)a operator b(c r)R(d)
typedef int i;typedef float f;struct v{
f x,y,z;v(){}v(f a,f b,f c){x=a;y=b;z=c
;}O(v,+,v,v(x+r.x,y+r.y,z+r.z))O(v,*,f,
v(x*r,y*r,z*r))O(f,%,v,x*r.x+y*r.y+z*r.
z)O(v,^,v,v(y*r.z-z*r.y,z*r.x-x*r.z,x*r
.y-y*r.x))v operator!()R(*this*(1/sqrt(
*this%*this)))};i m,G[]={247570,280596,
280600,249748,18578,18577,231184,16,16}
,u=1;f Z()R((u=u*9973^97,u>>8)*6e-8+.5)
i T(v o,v d,f&t,v&n){t=1e9;m=0;f p=-o.z
/d.z;if(.01<p)t=p,n=v(0,0,1),m=1;F(k,19
)F(j,9)if(G[j]&1<<k){v p=o+v(-k,0,-j-4)
;f b=p%d,c=p%p-1,q=b*b-c;if(q>0){f s=-b
-sqrt(q);if(s<t&&s>.01)t=s,n=!(p+d*t),m
=2;}}R(m)}v S(v o,v d){f t;v n;i q=T(o,
d,t,n);if(!q)R(v(.7,.6,1)*pow(1-d.z,4))
v h=o+d*t,l=!(v(9+Z(),9+Z(),16)+h*-1),r
=d+n*(n%d*-2);f b=l%n;if(b<0||T(h,l,t,n
))b=0;f p=pow(l%r*(b>0),99);m=ceil(h.
x/5)+ceil(h.y/5);m=3-2*(m&1);if(q&1)R(v
(3,m,m)*(b/5+.1))R(v(p,p,p)+S(h,r)*.5)}
i main(){printf("P6 512 512 255 ");v g=
!v(-6,-16,0),a=!(v(0,0,1)^g)*.002,b=!(g
^a)*.002,c=(a+b)*-256+g;F(y,512)F(x,512
){v p(13,13,13);F(r,64){v t=a*(Z()-.5)
+b*(Z()-.5);p=S(v(17,16,8)+t*99,!(t*-99
+(a*(Z()+x)+b*(y+Z())+c)*16))*3.5+p;}p\
rintf("%c%c%c",(i)p.x,(i)p.y,(i)p.z);}}
