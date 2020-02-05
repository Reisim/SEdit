#version 330

uniform sampler2D texture;
uniform int useTex;
uniform vec4 vColor;

in vec2 texc;
in vec3 pColor;
in vec3 position;

out highp vec4 fColor;

void main(){
	if( useTex == 1 ){
		fColor = texture2D(texture, texc);
	}
	else if( useTex == 100 ){
		fColor = vec4( 1, 1, 1, texture2D(texture, texc).r ) * vColor;
	}
	else if( useTex == 2 ){
		fColor = vColor;
	}
	else{
		fColor = vec4( pColor, 1.0 );
	}
}

