//Credit for this code goes to this page:
//https://stackoverflow.com/questions/3508643/how-do-i-convert-a-photo-from-color-to-black-and-white-using-opengl

uniform sampler2D source;

void main()
{
	vec4 sourceFragment = texture2D(source, gl_TexCoord[0].xy);
    float luminance = (sourceFragment.r + sourceFragment.g + sourceFragment.b) / 3.0;
	gl_FragColor = vec4(luminance, luminance, luminance, 1);
}