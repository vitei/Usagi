#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"

BUFFER_LAYOUT(1,  UBO_MATERIAL_ID) uniform Material
{
	mat4	mModel;
	vec4	vExtents;
	vec4	vColor;
	float   fArcStart;
	float   fArcEnd;
	bool 	bUseArc;
};

// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;

// Output attributes
ATTRIB_LOC(0) out vec4 vo_vColor;


void main(void)
{
	vec3 vPos = ao_position.xyz;
	if(bUseArc)
	{
		float angle = atan(vPos.x, vPos.z);
		if(angle < 0.0)
			angle += (2*3.1459);
		if(angle < fArcStart || angle > (fArcStart + fArcEnd))
		{
			vPos.x = 0.0f;
			vPos.z = 0.0f;
		}
	}

	vec3 vAdjustedSize = vPos * vExtents.xyz;

 	vec4 vPosition = vec4( vAdjustedSize, 1.0) * mModel;
 	//vec4 vPosition = vec4( ao_position.xy, -0.5, 1.0);
 	
	vec4 vViewPos = vPosition * mViewMat;	// Reverse ordering as a mat3x4
	vec4 vProjPos = vec4( vViewPos.xyz, 1.0 ) * mProjMat;

 	gl_Position		= vProjPos;	

	vo_vColor = vColor;	
}
