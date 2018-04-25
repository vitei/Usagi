


#define c_None              ( 0 )
#define c_Blend             ( c_None + 1 )
#define c_Mul               ( c_Blend + 1 )
#define c_Add               ( c_Mul + 1 )

#define c_Local				0
#define c_World				1

#ifndef AUTO_BUILD  // The model builder should define this, below are the defaults
#define OUTPUT_DEFERRED     ( 1 )
#define USE_VERTEX_COLOR    ( 0 )       // @@ id="vertex_color"
#define USE_DIFFUSE			( 1 )
#define USE_EMISSIVE		( 0 )		// @@ id="emissive"
#define USE_NORMAL_MAP      ( 0 )       // @@ id="normal_map"
#define USE_SPEC_MASK       ( 0 )       // @@ id="spec_mask"
#define USE_REFLECTION		( 0 )
#define NUM_LIGHT           ( 2 )

#endif  // AUTO_BUILD

#define SKINNING            ( 0 )       // @@ id="skinning" choice="[0, 9]" type="dynamic"
#define COORDINATE          ( c_Local ) // @@ id="coord" choice="c_Local, c_World" type="dynamic"

// Using these defines as we may want to optimize
// Also looking like we may transpose our matrices
#define MTX44_MUL_POS3(pos, mtx) (vec4(pos, 1.0) * mtx)
#define MTX44_MUL_VEC3(vec, mtx) (vec4(vec, 0.0) * mtx)

#define MTX43_MUL_POS3(pos, mtx) (vec4(pos, 1.0) * mtx)
#define MTX43_MUL_VEC3(pos, mtx) (vec4(pos, 0.0) * mtx)

#define MTX44_MUL_POS4(pos, mtx) (pos * mtx)
#define MTX44_MUL_VEC4(vec, mtx) (vec * mtx)


// vec4[2] * vec2
#define TRANSFORM_UV(uv, mtx) vec2(mtx[0].xy * uv.x + mtx[0].zw * uv.y + mtx[1].xy)

