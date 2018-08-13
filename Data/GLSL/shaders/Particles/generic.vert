#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"

#define BIRTH_TIME		ao_life.x
#define INV_LIFE_TIME	ao_life.y
#define CURRENT_TIME	vEffectConsts.x

#define SIZE_START		vEffectConsts.y
#define SIZE_END		vEffectConsts.z
#define SIZE_CLAMP		vEffectConsts.w
#define DRAG            vEffectConsts2.x
#define GRAVITY         vEffectConsts2.y

#define ROTATION_START	ao_rotation.x
#define ROTATION_END	ao_rotation.y

BUFFER_LAYOUT(1,  UBO_MATERIAL_ID) uniform Material
{
	vec4 vEffectConsts;
    vec4 vEffectConsts2;
};

// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;
ATTRIB_LOC(1) in vec4 ao_color;
ATTRIB_LOC(2) in vec2 ao_life;		// Particle life time (x: birth, y: lifetime).
ATTRIB_LOC(3) in vec2 ao_rotation;	// Particle rotation (x: start, y: end)
ATTRIB_LOC(4) in vec3 ao_velocity;	// Motion of the particle
ATTRIB_LOC(5) in vec4 ao_uvRange;     // Select a sub-section of the image


out VertexData
{
    ATTRIB_LOC(0) vec4    vo_vColor;
    ATTRIB_LOC(1) vec2    vo_vRotSize;
    ATTRIB_LOC(2) vec4    vo_vUVRange;

} vertexData;

// Calculate particle ratio.
float CalcParticleRatio(float birth, float inv_life_time)
{
    float c_time = CURRENT_TIME - birth;
    float ratio = c_time * inv_life_time;
    return clamp(ratio, 0.0, 1.0);
}

vec4 CalculateColor(float ratio)
{
    vec4 vColor = ao_color;
    vColor.w *= 1.0-ratio;
	return vColor;
}

void main(void)
{
    float lifeTime = CURRENT_TIME - BIRTH_TIME;
    float gravityAcc = GRAVITY * lifeTime;

	float ratio = CalcParticleRatio(BIRTH_TIME, INV_LIFE_TIME);
	// Calculate rotation.
    float rot = mix(ROTATION_START, ROTATION_END, ratio);
    float size;
    if(ratio < 0.05)
    {
    	size = (ratio * SIZE_START)*20.0;
    }
    else
    {
    	float newRatio = (ratio - 0.05)*0.8f;
    	size = min(mix(SIZE_START, SIZE_END, newRatio), SIZE_CLAMP);
    }

	float fTime =  CURRENT_TIME - BIRTH_TIME;
    vec3 vVelocity = ao_velocity * pow(DRAG, lifeTime);
    vVelocity.y += gravityAcc;
    vec3 vPosition = ao_position + (vVelocity * fTime);

    vec4 vColor = CalculateColor(ratio);

    // TODO: Use a model matrix
	vec4 vWorldPos	= /*mModelMat **/ vec4(vPosition, 1.0);

	vertexData.vo_vColor = vColor;
	vertexData.vo_vRotSize.x = rot;
	vertexData.vo_vRotSize.y = size;
    vertexData.vo_vUVRange = ao_uvRange;

	gl_Position	= vWorldPos;
}
