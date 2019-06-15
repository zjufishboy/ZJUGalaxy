uniform sampler2D tex_disc;
uniform sampler2D tex_previous;
const vec3 cam_pos = vec3(0,1.5, -6);
const vec3 cam_dir = vec3(0, 0, 1);
const float cam_tan_half_v_angle = 1.;
const float disc_inner = 3.;
const float disc_mid = 3.2;
const float disc_outer =6.;
const float disc_thickness = 0.2;
const float step_time = 0.008;
const int steps = 200;
const float disc_rot = 0.5;


float noise( in vec3 x ) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);
    vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
    vec2 rg = textureLod( iChannel2, (uv+ 0.5)/256.0, 0.0 ).yx;
    return -1.0+2.0*mix( rg.x, rg.y, f.z );
}

vec3 saturate(vec3 x)
{
    return clamp(x, vec3(0.0), vec3(1.0));
}

float saturate(float x)
{
    return clamp(x, 0.0, 1.0);
}

float rand(vec2 coord) {
    return saturate(fract(sin(dot(coord, vec2(12.9898, 78.223))) * 43758.5453));
}

vec3 color_map(vec3 color) {
    return pow(color, vec3(2.0)) * 1.0;
}

float cascade_noise(vec3 radialCoords, float speed) {
    float noise1 = 1.0;
    vec3 rc = radialCoords + 2.0;               rc.y += iTime * speed;
    noise1 *= noise(rc * 3.0) * 0.3 + 0.3;      rc.y += iTime * speed;
    noise1 *= noise(rc * 12.0) * 0.3 + 0.2;     rc.y += iTime * speed;
    return noise1;
}

void get_disc(inout vec3 color, inout float alpha_remain, vec3 pos, vec3 old_pos) {
    float dist = length(pos - old_pos);
	float v_dist = pos.y;
	float v_alpha = 0.; 
  
    
    v_alpha = max(-abs(v_dist)  / disc_thickness + 1.0, v_alpha);
   	v_alpha = min(v_alpha, 1.);
    
    vec2 xz0 = pos.xz;
    
    float r = length(xz0);
	float h_alpha = (r >= disc_inner && r <= disc_mid) ? 1. : 0.;
    float ratio_mid = r / disc_mid;
    //h_alpha = (ratio_mid >= 1.) ? max(0., 1. + (r-disc_mid)/(disc_mid-disc_outer)) : h_alpha;
    h_alpha = (r >= disc_mid && r <= disc_outer) ? .8 : h_alpha;
    
    float alpha = h_alpha * v_alpha;
    alpha /= r * r/ 1.;
    
    vec3 cc = vec3(1.2, 1.2, 1.2);

    vec3 radialCoords;
    radialCoords.x = r ;
    radialCoords.y = atan(xz0.y, xz0.x);
    radialCoords.z = v_dist;
        
    float speed = 0.06;
    
    float noise1 = cascade_noise(radialCoords + 2.0, 0.26);
    float noise2 = cascade_noise(radialCoords + 4.0, 0.03);
    float t = (r - disc_inner) / (disc_outer - disc_inner);
    float noise = mix(noise1, noise2, t);
    cc = mix(vec3(1.2), vec3(1, .8, .8), t);
    
    alpha *=  noise * 0.998 +  0.002;
    alpha *= saturate(mix(1.0, -.0, t));
    alpha *= step_time * 5.0;
    //alpha *= dist;
    
	color += alpha * alpha_remain * cc;
	alpha_remain *= (1. - alpha);
	bool in_photon_sphere = dot(pos, pos) <= 1.;
	alpha_remain = in_photon_sphere ? 0. : alpha_remain;
}

void light_step(float h2, inout vec3 pos, inout vec3 v) {
	float r2 = dot(pos, pos);
	vec3 acc = -1.5 * h2 * pos / pow(r2, 2.5) * 1. ;
    float t = step_time;
    float dd = dot(normalize(pos), normalize(v));
    float step_add = 0.;
    step_add += r2 >= 5. ? pow(r2, 0.45) : 0.;
    step_add *= saturate(pos.y);
    step_add *= 1. - dd * .98;
    t *= 1. + step_add;
	t *= length(cam_pos);
    // t *= min(10., r2 * r2);
    //t *= r2 <= 8. ? .2 : 1.;
    //t *= r2 <= 4. ? .2 : 1.;
    //t *= r2 <= 2. ? .2 : 1.;
	pos += v * t;
	v += acc * t;
    //
}


void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec2 uv = fragCoord.xy / iResolution.xy;
    vec2 uv_origin = uv;
    uv.x += (rand(uv + sin(iTime * 1.0)) / iResolution.x) * (iMouse.z > 1.0 ? 0.0 : 1.0);
    uv.y += (rand(uv + 1.0 + sin(iTime * 1.0)) / iResolution.y) * (iMouse.z > 1.0 ? 0.0 : 1.0);

	float aspect = iResolution.x / iResolution.y;
	vec3 dir = vec3((uv * 2. - 1.) * vec2(aspect, 1.) * cam_tan_half_v_angle, 1.);
    dir = normalize(dir);
	vec3 pos = cam_pos;
	vec3 h = cross(pos, dir);
	float h2 = dot(h, h);
	vec3 color = vec3(0, 0, 0);
	float alpha = 1.;
	for (int i = 0; i < steps; i++) {
        vec3 old = pos;
		light_step(h2, pos, dir);
		get_disc(color, alpha, pos, old);
	}
    //color += alpha * texture(iChannel1, dir).xyz;
	
    color *= 0.5;
    
    const float p = 1.0;
    vec3 previous = pow(texture(iChannel0, uv_origin).rgb, vec3(1.0 / p));
    color = pow(color, vec3(1.0 / p));
    float blendWeight = 0.9 * (iMouse.z > 1.0 ? 0.0 : 1.0);
    color = mix(color, previous, blendWeight);
    color = pow(color, vec3(p));
    
    fragColor = vec4(saturate(color), 1.);
}