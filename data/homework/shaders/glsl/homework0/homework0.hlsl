cbuffer UBO : register(b0){
    float4x4 projectionMatrix;
    float4x4 modelMatrix;
    float4x4 viewMatrix;
};
struct v2f{
    float4 pos : SV_POSITION;
    float3 color : TEXCOORD0;
};
#ifdef VS
    struct appdata{
        float3 inPos : POSITION;
        float3 inColor : COLOR;
    };
    v2f main(appdata v){
        v2f o;
        o.pos = mul(mul(mul(projectionMatrix, viewMatrix), modelMatrix), float4(v.inPos, 1));
        // o.pos = float4(v.inPos, 1);
        o.color = v.inColor;
        return o;
    }
#endif

#ifdef PS
    float4 main(v2f i) : SV_TARGET0{
        return float4(i.color, 1);
    }
#endif