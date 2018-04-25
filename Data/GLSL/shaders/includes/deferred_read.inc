SAMPLER_LOC(1, 0) uniform sampler2D sampler0; 	// Depth
SAMPLER_LOC(1, 1) uniform sampler2D sampler1;	// Normal
SAMPLER_LOC(1, 2) uniform sampler2D sampler2;	// Diffuse + spec pow
SAMPLER_LOC(1, 3) uniform sampler2D sampler3;	// Emissive
SAMPLER_LOC(1, 4) uniform sampler2D sampler4;	// Specular color

// FIXME: Wasting a component of the normal, can reverse calculate from just two

void GetDeferredDataSpecFS(in vec2 texCoord, in vec3 frustumRay, out vec3 pos, out vec3 normal, out vec3 color, out vec3 specColor, out float specularPow)
{
	vec4 vRead0 = texture(sampler0, texCoord);
	vec4 vRead1 = texture(sampler1, texCoord);
	vec4 vRead2 = texture(sampler2, texCoord);
	vec4 vRead4 = texture(sampler4, texCoord);

	float fPixelDepth = vRead0.x;
	normal	= normalize(vRead1.xyz);
	color	= vRead2.xyz;
	specColor = vRead4.xyz;
	specularPow = vRead2.w * 128.f;

	pos = (fPixelDepth * frustumRay);
}

void GetDeferredDataFS(in vec2 texCoord, in vec3 frustumRay, out vec3 pos, out vec3 normal, out vec3 color)
{
	// Incredibly wasteful, position can be reverse calculated from the depth and x,y co-ordiantes
	// Put into screen space only two components of the normal are required
	vec4 vRead0 = texture(sampler0, texCoord);
	vec4 vRead1 = texture(sampler1, texCoord);
	vec4 vRead2 = texture(sampler2, texCoord);

	float fPixelDepth = vRead0.x;
	normal	= normalize(vRead1.xyz);
	color	= vRead2.xyz;

	pos = (fPixelDepth * frustumRay);
}

vec3 GetDeferredEmissive(in vec2 texCoord)
{
	return texture(sampler3, texCoord).rgb;
}

void GetDeferredData3D(in vec2 texCoord, in vec3 vVSValue, out vec3 pos, out vec3 normal, out vec3 color)
{
	// Incredibly wasteful, position can be reverse calculated from the depth and x,y co-ordiantes
	// Put into screen space only two components of the normal are required
	vec4 vRead0 = texture(sampler0, texCoord);
	vec4 vRead1 = texture(sampler1, texCoord);
	vec4 vRead2 = texture(sampler2, texCoord);

	normal	= normalize(vRead1.xyz);
	color	= vRead2.xyz;

	vec3 vViewRay = vec3(vVSValue.xy * (vNearFar.y / vVSValue.z), vNearFar.y);
	float fNormalizedDepth = vRead0.x;
	pos = vViewRay * fNormalizedDepth;
}

void GetDeferredDataSpec3D(in vec2 texCoord, in vec3 vVSValue, out vec3 pos, out vec3 normal, out vec3 color, out vec3 specCol, out float specPow)
{
	// Incredibly wasteful, position can be reverse calculated from the depth and x,y co-ordiantes
	// Put into screen space only two components of the normal are required
	vec4 vRead0 = texture(sampler0, texCoord);
	vec4 vRead1 = texture(sampler1, texCoord);
	vec4 vRead2 = texture(sampler2, texCoord);
	vec4 vRead4 = texture(sampler4, texCoord);

	normal	= normalize(vRead1.xyz);
	color	= vRead2.xyz;
	specPow = vRead2.w * 128.f;
	specCol = vRead4.xyz;

	vec3 vViewRay = vec3(vVSValue.xy * (vNearFar.y / vVSValue.z), vNearFar.y);
	float fNormalizedDepth = vRead0.x;
	pos = vViewRay * fNormalizedDepth;
}
