uniform mat4 mWorldViewProj;

varying vec2 texCoord;
varying float texComp[4];
varying float alpha;

void main(void)
{
	gl_Position = mWorldViewProj * gl_Vertex;
	
	texCoord = (gl_TextureMatrix[0]*gl_MultiTexCoord0).xy;
	
	int fti = int(gl_Color.r*255.);
	int sti = int(gl_Color.g*255.);
	float ftPerc = gl_Color.b; 
	alpha = gl_Color.a;

	for(int i=0; i<4; i++) {
		texComp[i] = 0.0;
	}
	if(fti == sti)
		ftPerc = 1.;
	else
		texComp[sti] = 1.0-ftPerc;
	texComp[fti] = ftPerc;
}
