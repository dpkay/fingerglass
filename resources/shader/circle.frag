uniform float thickness;
uniform float blur;
uniform float alpha;
uniform vec4 color;
varying vec4 texcoord;

void main(void)
{
	vec4 shifted = (texcoord - vec4(0.5,0.5,0,0))*2;
	float dist = sqrt(shifted.x*shifted.x+shifted.y*shifted.y);
	
	float m = 1-blur-thickness/2;
	
	float ramp1 = clamp((dist-m+thickness+blur)/blur,0,1);
	float ramp2 = clamp((m-dist+thickness+blur)/blur,0,1);
	
	float product = ramp1*ramp2;
	gl_FragColor = vec4(product*color.r, product*color.g, product*color.b, product*alpha);
}