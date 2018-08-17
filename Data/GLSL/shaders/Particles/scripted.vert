#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"

#define BIRTH_TIME      ao_life.x
#define INV_LIFE_TIME   ao_life.y
#define CURRENT_TIME    fEffectTime

#define SIZE_START      vEffectConsts.y
#define SIZE_END        vEffectConsts.z
#define SIZE_CLAMP      vEffectConsts.w
#define DRAG            fAirResistance

#define ROTATION_START  ao_rotation.x
#define ROTATION_SPEED  ao_rotation.y
#define ROTATION_ATTEN  ao_rotation.z

const float pi_over_2 = 1.5707963267;
const float two_pi = 6.28318530;
const float one_over_2pi = 0.159154943;
const float pi = 3.14159265;

BUFFER_LAYOUT(1,  UBO_MATERIAL_ID) uniform Material
{
    vec4    vGravityDir;
    vec4    vColorSet[3];
    vec4    vColorTiming;  // In, peak, out, repetition
    vec4    vAlphaValues;   // Initial, intermediate, ending alpha
    vec2    vAlphaTiming;   // In, out
    vec4    vScaling;
    vec2    vScaleTiming;   // In, out
    vec2    vUVScale[2];
    float   fColorBehaviour;
    float   fAirResistance;
    float   fUpdatePosition;
    float   fColorAnimRepeat;
    vec4    vEnvColor;
    bool    bLocalEffect;
};

BUFFER_LAYOUT(1,  UBO_CUSTOM0_ID) uniform Custom0
{
    float   fEffectTime;
};

// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;
ATTRIB_LOC(1) in vec2 ao_size;
ATTRIB_LOC(2) in vec2 ao_uvOffset;
ATTRIB_LOC(3) in vec3 ao_velocity;    // Motion of the particle
ATTRIB_LOC(4) in float ao_colorId;
ATTRIB_LOC(5) in vec2 ao_life;        // Particle life time (x: birth, y: lifetime).
ATTRIB_LOC(6) in vec3 ao_rotation;    // Particle rotation (x: start, y: speed, z: attenuation)



out VertexData
{
    INT_LOC(0) vec4    vo_vColor;
    INT_LOC(1) vec2    vo_vSize;
    INT_LOC(2) float   vo_fRot;
    INT_LOC(3) vec4    vo_vUVRange[2];
    INT_LOC(5) vec3    vo_velocity;

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
    vec4 vColor;
    if(fColorBehaviour >= 2.0)
    {
        float fTempRatio = ratio*fColorAnimRepeat;
        ratio = fract(fTempRatio);
        if(mod(fTempRatio, 2.0)>=1.0)
        {
            ratio = 1.0 - ratio;
        }

        if(ratio > vColorTiming.z)
        {
            float fLerp = ratio - vColorTiming.z;
            fLerp /= (1.0f - vColorTiming.z);
            vColor = mix(vColorSet[1], vColorSet[2], fLerp);
        }
        else if(ratio > vColorTiming.y)
        {
            vColor = vColorSet[1];
        }
        else if(ratio > vColorTiming.x)
        {
            float fLerp = ratio - vColorTiming.x;
            fLerp /= (vColorTiming.y - vColorTiming.x);
            vColor = mix(vColorSet[0], vColorSet[1], fLerp);
        }
        else
        {
            vColor = vColorSet[0];
        }
    }
    else if(fColorBehaviour >= 1.0)
    {
        if(ao_colorId >= 1.5)
            vColor = vColorSet[2];
        else
            vColor = mix(vColorSet[0], vColorSet[1], ao_colorId);
    }
    else
    {
        // Constant color
        vColor = vColorSet[0];
    }
    return vColor * vEnvColor;
}

vec2 CalculateScale(float ratio)
{
    float fScale;

    if(ratio > vScaleTiming.y)
    {
        // Ending alpha
        float fLerp = ratio-vScaleTiming.y;
        fLerp /= (1.0-vScaleTiming.y);
        fScale = mix(vScaling.y, vScaling.z, fLerp);
    }
    else if(ratio > vScaleTiming.x)
    {
        // Intermediate alpha
        fScale = vScaling.y;
    }
    else
    {
        // Initial alpha
        float fLerp = ratio;
        fLerp /= (vScaleTiming.x);
        fScale = mix(vScaling.x, vScaling.y, fLerp);
    }

    return fScale * ao_size;
}



float CalculateAlpha(float ratio)
{
    float fAlpha;

    if(ratio > vAlphaTiming.y)
    {
        // Ending alpha
        float fLerp = ratio-vAlphaTiming.y;
        fLerp /= (1.0-vAlphaTiming.y);
        fAlpha = mix(vAlphaValues.y, vAlphaValues.z, fLerp);
    }
    else if(ratio > vAlphaTiming.x)
    {
        // Intermediate alpha
        fAlpha = vAlphaValues.y;
    }
    else
    {
        // Initial alpha
        float fLerp = ratio;
        fLerp /= (vAlphaTiming.x);
        fAlpha = mix(vAlphaValues.x, vAlphaValues.y, fLerp);
    }

    return fAlpha;
}


float CalculateRotation(float fLifeTime)
{
    float fRotation = ROTATION_START + (ROTATION_SPEED*fLifeTime);
    // Bring into the range -PI to PI
    fRotation += pi;
    fRotation *= one_over_2pi;
    fRotation = fract(fRotation);
    fRotation *= two_pi;
    fRotation -= pi;

    return fRotation;
}


vec3 CalculatePosition(float fLifeTime)
{
    // s = ut + (1/2)a t^2
    vec3 vVelocity = ao_velocity * pow(DRAG, fLifeTime); // A total hack, get ready to see particles moving back towards the center

    vVelocity += (0.5*vGravityDir.xyz)*fLifeTime;
    vec3 vPos = (vVelocity * fLifeTime);

    if(bLocalEffect)
    {
        vertexData.vo_velocity  = vec4(normalize(mix(ao_velocity, vVelocity, fUpdatePosition)), 0.0f) * mModelMat;
    }   
    else
    {
        vertexData.vo_velocity  = normalize(mix(ao_velocity, vVelocity, fUpdatePosition));
    }

    

    return (vPos * fUpdatePosition) + ao_position;
}


vec4 CalculateUVCoords(float lifetime, int index)
{
    vec4 uvRange;
    uvRange.xy = ao_uvOffset;
    uvRange.zw = vUVScale[index];

    return uvRange;
}

void main(void)
{
    float lifetime = CURRENT_TIME - BIRTH_TIME;

    float ratio = CalcParticleRatio(BIRTH_TIME, INV_LIFE_TIME);
    // Calculate rotation.
    vec4 vColor;
    vColor = CalculateColor(ratio);
    vColor.a *= CalculateAlpha(ratio);

    vertexData.vo_vColor = vColor;

    vertexData.vo_vSize = CalculateScale(ratio);

    vertexData.vo_fRot = CalculateRotation(lifetime);


    vec3 vPosition = CalculatePosition(lifetime);

    vec4 vWorldPos    = vec4(vPosition, 1.0);;
    if(bLocalEffect)
    {
       vWorldPos = vec4( vWorldPos * mModelMat, 1.0 );
    }   

    vertexData.vo_vUVRange[0] = CalculateUVCoords(lifetime, 0);
    vertexData.vo_vUVRange[1] = CalculateUVCoords(lifetime, 1);

    gl_Position = vWorldPos;
}
