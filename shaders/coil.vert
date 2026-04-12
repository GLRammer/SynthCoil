#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in float inAlpha;

layout(location = 0) out float fragAlpha;

layout(binding =0) uniform UB{
    mat4 model;
    mat4 view;
    mat4 proj;
    float scale;
} ub;

float scale = ub.scale;

layout(binding = 1) readonly buffer myBuff{
    float data[];
} buf;

// Apply gausian bump
float bumper(vec2 pos, float dist, float smval) {
    float dx = abs(length(pos)-dist);
    if(smval<=0.0){
        if(dx<=0.1){
            return 1;
        }
        return 0;
    }
    if(dx>=smval){
        return 0;
    }
    float y= exp(1.0+(1/(pow(dx/smval,2)-1)));
    return y;
}

// Loop through input frequencies and apply bump, taking the highest value of each
vec2 bumpRunner(vec3 pos) {
    int n=int(buf.data[1]);
    if(n<1){
        return vec2(0.0,0.0);
    }
    float smval=buf.data[0];
    int i=2;
    float y=0,a=0;
    while(i+1<n){
        float bump=bumper(pos.xz, buf.data[i]*scale,smval);
        float tempy= scale*bump*buf.data[i+1];
        if(tempy>y){
            y=tempy;
            a=bump;
        }
        i+=2;
    }
    return vec2(y,a);
}

void main()
{
    // FLAT RENDER VERSION
    // normalize for flat rendering
    // vec3 p = inPos / 12.0;
    // gl_Position = vec4(p.x, p.y, p.z, 1.0);

    // 3D RENDER VERSION
    vec2 temp=bumpRunner(inPos);
    gl_Position=ub.proj*ub.view*vec4(inPos.x,inPos.y-temp.x,inPos.z,1.0);

    // Clamp alpha just to be safe
    fragAlpha = clamp(temp.y, 0.0, 1.0);
}