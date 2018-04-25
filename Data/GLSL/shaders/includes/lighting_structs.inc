

struct PointLight
{
	vec4	vPos;
	vec4	vColorSpec;
	vec4	vRange;
	vec4	vAmbient;
};


struct SpotLight
{
	mat4		mRotMat;
	vec4		vPos;
	vec4		vColorSpec;
	vec4		vAmbient;
	vec4		vRange;
	vec4		vDirection;
	float 		fCosSpotCutoff;
	float 		fOuterSpotRadius;
	float 		fCosInnerSpotCutoff;
};

struct ProjectionLight
{
	mat4		mModelMat;	// Our frustum model can be moved and rotated
	mat4		mTextureMat;
	mat4		mViewProjInv;
	vec4		vPos;
	vec4		vDirection;
	vec4		vColorSpec;
	vec4		vAmbient;
	vec4		vRange;
	// Other spot details are handled by the texture it's self
};

