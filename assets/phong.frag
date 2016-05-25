#version 330


struct booleanCut
{
	vec3 center;
	vec3 uvw;
	int type;
};

uniform int			uNumCuts;
uniform booleanCut	uCutArray[10];
uniform sampler2D	uTex0;
uniform int			uTexturingMode;
uniform ivec2       uFreq;
uniform int			uLightAll;
uniform vec4		uSpaceParams;
uniform vec4		uSpacePos;
uniform float		uCutAlpha;
uniform bool		uBackfaceCulling;
uniform int			uCutMode;

in VertexData	{
	vec4 realPosition;
	vec4 position;
	vec3 normal;
	vec4 color;
	vec2 texCoord;
} vVertexIn;

out vec4 oFragColor;

//boolean operators

//
float sphere (vec3 p, float r)
{
    return length (p) - r;
}
/*
float box (vec3 p, vec3 u, vec3 v, float w)
{
    return length (max (abs (u) - v + vec3 (w), .0)) - w;

}
*/
float cylinder (vec3 p, vec3 n)
{
    return length (p.xz - n.xy) - n.z;
}

float opS( float d1, float d2 )
{
    return max(-d2,d1);
}

vec2 opU( vec2 d1, vec2 d2 )
{
	return (d1.x<d2.x) ? d1 : d2;
}

float opUx( float d1, float d2 )
{
	return (d1<d2) ? d1 : d2;
}

vec3 opRep( vec3 p, vec3 c )
{
    return mod(p,c)-0.5*c;
}




// Based on OpenGL Programming Guide (8th edition), p 457-459.
float checkered( in vec2 uv, in ivec2 freq )
{
	vec2 checker = fract( uv * freq );
	vec2 edge = fwidth( uv ) * freq;
	float mx = max( edge.x, edge.y );

	vec2 pattern = smoothstep( vec2(0.5), vec2(0.5) + edge, checker );
	pattern += 1.0 - smoothstep( vec2(0.0), edge, checker );

	float factor = pattern.x * pattern.y + ( 1.0 - pattern.x ) * ( 1.0 - pattern.y );
	return mix( factor, 0.5, smoothstep( 0.0, 0.75, mx ) );
}


/*
bool cylinderCut(vec4 point, vec4 pos, vec4 uvw)
{
	
	vec4 relPos = point - pos;

	return(	abs(relPos.x) < boxUVW.x && 
			abs(relPos.y) < boxUVW.y && 
			length(relPos) < length(ballDims);
}
*/

bool boxCut(vec4 p, vec4 boxPos, vec4 boxUVW)
{
	vec4 relPos = p - boxPos;
	
	return(	abs(relPos.x) < boxUVW.x && 
			abs(relPos.y) < boxUVW.y && 
			abs(relPos.z) < boxUVW.z
	);
}

bool ballCut(vec4 p, vec4 ballPos, vec4 ballDims)
{
	
	vec4 relPos = p - ballPos;

	return length(relPos) < length(ballDims);
	
}

bool cut(vec4 point, vec4 cutPos, vec4 uvw)
{
	if(uCutMode == 1)
		return boxCut(point, cutPos, uvw);
	else if (uCutMode == 2)
		return ballCut(point, cutPos, uvw);
	else return false;
}

void main()
{
	// set diffuse and specular colors
	vec3 cDiffuse = vVertexIn.color.rgb;
	vec3 cSpecular = vec3( 0.3 );
	
	//extra lightsource will be unnecessary for this application, 
	//therefore 
	vec3 vLightPosition = -vVertexIn.realPosition.xyz;

	// lighting calculations
	vec3 L = normalize( vLightPosition - vVertexIn.position.xyz );
	vec3 E = normalize( -vVertexIn.position.xyz );
	vec3 H = normalize( L + E );

	vec3 N = vec3( 0.0 );

	N = normalize( vVertexIn.normal );

	// Calculate coefficients.
	
	float phong = 1.0;

	//it may or may not present an advantage to render backwards polygons as it may present a good portion of information within a cut

	if(uBackfaceCulling)
		phong = max( dot( N, L ), 0.0 );
	else
		phong = max( dot( N, L ), dot(-N,L) );
	

	const float kMaterialShininess = 10.0;
	const float kNormalization = ( kMaterialShininess + 8.0 ) / ( 3.14159265 * 8.0 );
	float blinn = pow( max( dot( N, H ), 0.0 ), kMaterialShininess ) * kNormalization;

	// diffuse coefficient
	vec3 diffuse = vec3( phong );

	if( uTexturingMode == 1 ) {
		diffuse *= vec3( 0.7, 0.5, 0.3 );
		diffuse *= 0.5 + 0.5 * checkered( vVertexIn.texCoord, uFreq );
	}
	else if ( uTexturingMode == 2 )
		diffuse *= texture( uTex0, vVertexIn.texCoord.st ).rgb;
	else if ( uTexturingMode == 3 )
		diffuse *= vVertexIn.normal.rgb;
		

	// specular coefficient
	vec3 specular = blinn * cSpecular;
	
	vec3 finalColor = diffuse + specular;
	
	if(cut(vVertexIn.realPosition, 
		uSpacePos, 
		uSpaceParams ))
	{
		//apply transparency according to cut transparent
		//oFragColor = vec4( finalColor, uCutAlpha);
		if(uCutAlpha > 0.0f)
			oFragColor = vec4( finalColor, uCutAlpha);
		else 
			//better than zero transparency, doesn't solve janky alpha blending
			discard;
	}
	else
	{
		//solid
		oFragColor = vec4( finalColor, 1.0f );
	}

	
	// alpha
	//float alpha = ( uTexturingMode == 3 ) ? 0.75 : 1.0;
	//float alpha = 0.15;
	//float alpha = ( uTexturingMode == 3 ) ? 0.15 : 1.0;
	// final color
	//oFragColor = vec4( diffuse + specula, alpha );
	//oFragColor = vec4( vVertexIn.realPosition.xyz, alpha);
	//if(vVertexIn.position.xyz > vec3 (0.0, 0.0, 0.0))
	//bvec b =lessThan( vVertexIn.realPosition.xyz , vec3 (0.0) ); 
	//if(  bvec.x && bvec.y && bvec.z )


	//if(vVertexIn.realPosition.x < 1.0 && vVertexIn.realPosition.y < 1.0 && vVertexIn.realPosition.z < 0.0 )
	//if(testCut(vVertexIn.realPosition))
	/*
	if(!boxCut(vVertexIn.realPosition, 
		vec4( 5.0, 1.0, 1.0, 1.0), 
		vec4( 5.0, 5.0, 5.0, 1.0)))
		oFragColor = vec4( diffuse + specular, alpha );
	else
	*/
	


}
