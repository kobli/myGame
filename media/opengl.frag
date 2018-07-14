uniform sampler2D textures[4];
uniform mat4 mInvWorld;

varying vec2 texCoord;
varying float texComp[4];
varying float alpha;

void main (void)
{
	gl_FragColor = vec4(0, 0, 0, 255);

	for(int i=0; i<4; i++) {
		gl_FragColor += texComp[i]*texture2D(textures[i], texCoord);
	}
	gl_FragColor.a = alpha;
}
