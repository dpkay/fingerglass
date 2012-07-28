varying vec4 texcoord;

void main(void)
{
	texcoord = gl_MultiTexCoord0;
	gl_Position = ftransform();
}