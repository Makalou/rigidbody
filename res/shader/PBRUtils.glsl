const float PI = 3.14159265359;

float DistributionGGX(float NoH, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NoH2 = NoH*NoH;

    float num   = a2;
    float denom = fma(NoH2 ,(a2 - 1.0) ,1.0);
    denom = PI * denom * denom;

    return num / denom;
}
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = fma(NdotV ,(1.0 - k) , k);

    return num / denom;
}
float GeometrySmith(float NdotV, float NdotL, float roughness)
{
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

//brdf

vec3 cook_torrance(in float NdotH, in float NdotV, in float NdotL, in float roughness,in vec3 F)
{
    float NDF = DistributionGGX(NdotH,roughness);//normal distribution function
    float G = GeometrySmith(NdotV,NdotL,roughness);
    vec3 numerator    = NDF * G * F;
    float denominator = fma(4.0 * NdotV , NdotL , 0.0001);
    return numerator / denominator;
}

vec3 brdf(in vec3 L, in vec3 V, in vec3 N, in vec3 albedo, in float metallic, in float roughness,in vec3 F0)
{
    vec3 H = normalize(V+L);

    float NdotH  = max(dot(N, H), 0.0);
    //float NDF = DistributionGGX(NdotH,roughness);//normal distribution function
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    vec3 F = fresnelSchlick(max(dot(H,V),0.0),F0);

    vec3 Ks =F;
    vec3 Kd = mix(vec3(1.0) - Ks, vec3(0.0), metallic);

    return Kd*albedo+cook_torrance(NdotH,NdotV,NdotL,roughness,F);
}

void inverse_gama_correct(inout vec3 color)
{
    color = pow(color, vec3(2.2));
}

void gama_correct(inout vec3 color)
{
    color = pow(color, vec3(1.0/2.2));
}

void gama_correct2(inout vec3 col)
{
    col = max( vec3(0), col - 0.004);
    col = (col*(6.2*col + .5)) / (col*(6.2*col+1.7) + 0.06);
}

//tone mapping
vec3 reinhard(vec3 origin)
{
    return origin/(origin+vec3(1.0));
}

vec3 sigmod(vec3 origin)
{
    return 2*(vec3(1.0)/(vec3(1.0)+exp(-origin))-vec3(0.5));
}

//IBL
vec3 getIBLAbiment(in vec3 N, in vec3 V, in vec3 F0,in float metallic,in float roughness, in vec3 irradiance,in vec3 albedo)
{
    vec3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0,roughness);
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    vec3 diffuse = irradiance * albedo;
    return (kD * diffuse);
}
