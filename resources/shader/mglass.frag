uniform sampler2D mglass_tex;
uniform sampler2D content_tex;
uniform float alpha;
varying vec4 texcoord;

void main(void)
{
	//gl_FragColor = vec4(1,1,0,1);
	vec4 mglass_rgba = texture2D(mglass_tex, texcoord.st);
	vec4 content_rgba = texture2D(content_tex, texcoord.st);

	float inner_mask = mglass_rgba.b - mglass_rgba.r;
	vec3 out_rgb = mglass_rgba.rrr + inner_mask * content_rgba.rgb;
	//vec3 out_rgb = content_rgba.rgb;
	gl_FragColor = vec4(out_rgb, mglass_rgba.a * alpha);
	//gl_FragColor = vec4(1,0,0,mglass_rgba.a * alpha);
}