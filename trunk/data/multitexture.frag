uniform sampler2D TextureUnit0;
uniform sampler2D TextureUnit1;

void main(void)
{
	vec4 value1 = texture2D(TextureUnit0, vec2(gl_TexCoord[0]));
	vec4 value2 = texture2D(TextureUnit1, vec2(gl_TexCoord[1]));
	//vec4 black = vec4(0.7,0.5,0.2,1.0);

	if(value2[0] < 0.05 && value2[1] < 0.05 && value2[2] < 0.05) {
		gl_FragColor = value1 * 0.5;
	}else {
		gl_FragColor = value2;	
	}
}
