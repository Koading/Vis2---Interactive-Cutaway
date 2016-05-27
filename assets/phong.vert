#version 330

uniform mat4	ciModelViewProjection;
uniform mat4	ciModelView;
uniform mat3	ciNormalMatrix;

struct booleanCut
{
	vec3	center;
	vec3	uvw;
	int		type;
};

uniform int			uNumCuts;
//uniform booleanCut	uCutArray[10];
uniform vec4		uSpaceParams;

in vec4		ciPosition;
in vec3		ciNormal;
in vec4		ciColor;
in vec2		ciTexCoord0;

out VertexData {
	vec4 realPosition;
	vec4 position;
	vec3 normal;
	vec4 color;
	vec2 texCoord;
} vVertexOut;

void main()
{
	//if( boxCut(ciPosition, vec4(1.0,1.0,1.0,1.0), vec4(10.0,10.0,10.0,1.0)))
	{
		//i need some kind of realposition in the fragment stage
		vVertexOut.realPosition = ciPosition;
		vVertexOut.position = ciModelView * ciPosition;
		vVertexOut.normal = ciNormalMatrix * ciNormal;
		vVertexOut.color = ciColor;
		vVertexOut.texCoord = ciTexCoord0;
		gl_Position = ciModelViewProjection * ciPosition;
	}

}
