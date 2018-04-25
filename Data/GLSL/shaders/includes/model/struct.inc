

struct VertexOutput
{
    vec3 pos_v;
	vec3 pos_w;
    vec3 nrm_v;
#if USE_NORMAL_MAP
    vec4 tan_v;
#endif
#if USE_VERTEX_COLOR
    vec4 color;
#endif
    vec4 uv0_uv1;
#if USE_SPEC_MASK || USE_NORMAL_MAP    
    vec4 uv2_uv3;
#endif
};
