
BUFFER_LAYOUT(1, UBO_MATERIAL_ID) uniform Material
{
    vec4 diffColor;         // @@ id="diffuse" default="1 1 1" item="color" 
    vec4 texsrt0[2];        // @@ id="texsrt0" type="texsrt" hint="albedo0"
    vec4 texsrt1[2];        // @@ id="texsrt1" type="texsrt" hint="albedo1"
    vec4 texsrt2[2];        // @@ id="texsrt2" type="texsrt" hint="normal0"
    vec4 texsrt3[2];        // @@ id="texsrt3" type="texsrt" hint="specular0"
    float alphaRef;         // Alpha reference value (defaulting to 0 for now)
    float weightTex;        // @@ id="weight_tex" default="1.0" min="-1" max="1"
    float specInt;          // @@ id="spec_int" min="0" max="1" default="0.2"
    float specPower;        // @@ id="spec_power" min="0" default="50"
    float rimInt;           // @@ id="rim_int" min="0" max="1" default="0.5"
    float rimPower;         // @@ id="rim_power" min="0" default="2.5"
};
