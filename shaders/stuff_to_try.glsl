
float i,e,g,R,s;vec3 q,p,d=vec3((FC.yx-.5*r)/r,.8);for(q.zy--;i++<99.;){e+=i/9e9;o.rgb+=hsv(p.y,q.y,min(e*i,.01));s=3.;p=q+=d*e*R*.25;g+=p.y/s;p=vec3(log2(R=length(p))+t*.2,exp2(mod(-p.z,s)/R)-.23,p);for(e=--p.y;s<6e3;s+=s)e+=-abs(dot(sin(p.xz*s),cos(p.zy*s))/s*.5);}

for(float i,d,z;i++<1e2;o+=(cos(z+vec4(3,4,5,0))+2.)/d/z){vec3 p=z*normalize(FC.rgb*2.-r.xyy);p.z+=2.;d=max(-p.y,0.);p.y+=d+d;z+=d=.2*(.01+.1*d+length(cos(p.yz/dot(p,p)/.1-t/4.))/++d);}o=tanh(o/8e2);
