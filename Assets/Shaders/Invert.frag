//Credit for this code goes to this page:
//https://stackoverflow.com/questions/35209868/how-to-invert-texture-colours-in-opengl

uniform sampler2D source;

void main()
{
	vec4 sourceFragment = texture2D(source, gl_TexCoord[0].xy);
	gl_FragColor = vec4(1.0 - sourceFragment.r, 1.0 - sourceFragment.g, 1.0 - sourceFragment.b, 1);
}