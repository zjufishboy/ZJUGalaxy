uniform sampler2D tex_disc;
uniform sampler2D tex_previous;
const vec3 cam_pos = vec3(0,0.9, -10);
const vec3 cam_dir = vec3(0, 0, 1);
const float cam_tan_half_v_angle = 1.;
const float disc_inner = 4.;
const float disc_mid = 6.;
const float disc_outer = 8.;
const float disc_thickness = 0.3;
const float step_time = .05;
const int steps = 400;
const float disc_rot = 0.5;

// uniform float direct_escape_dist = 3;

float v_alpha_itg(float x) {
    x = clamp(x, -disc_thickness, disc_thickness);
    //return 10. * x;
    return x - x * x * sign(x) / 2. / disc_thickness;
}

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
    //return color;
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
	float v_dist = pos.y;
	float v_alpha = 0.; 
  
    v_alpha = max(-abs(v_dist)  / disc_thickness + 1.0, v_alpha);
   	v_alpha = min(v_alpha, 1.);
    
    vec2 xz0 = pos.xz;
    
    float r = length(xz0);
	float h_alpha = (r >= disc_inner && r <= disc_mid) ? 1. : 0.;
    float ratio_mid = r / disc_mid;
    h_alpha = (ratio_mid >= 1.) ? max(0., 1. + (r-disc_mid)/(disc_mid-disc_outer)) : h_alpha;
    
    float alpha = h_alpha * v_alpha;
    
    vec3 cc = vec3(1.2, 1.2, 1.2);

    vec3 radialCoords;
    radialCoords.x = r ;
    radialCoords.y = atan(xz0.y, xz0.x);
    radialCoords.z = v_dist;
        
    float speed = 0.06;
    
    float noise1 = cascade_noise(radialCoords + 2.0, 0.16);
    float noise2 = cascade_noise(radialCoords + 4.0, 0.04);
    float t = (r - disc_inner) / (disc_outer - disc_inner);
    float noise = mix(noise1, noise2, t);
    cc = mix(vec3(1.2), vec3(1.2, 1, 1), t);
    
    alpha *=  noise * 0.998 +  0.002;
    alpha *= min(1., mix(1.1, .3, t));
    
	color += alpha * alpha_remain * cc;
	alpha_remain *= (1. - alpha);
	bool in_photon_sphere = dot(pos, pos) <= 2.25;
	alpha_remain = in_photon_sphere ? 0. : alpha_remain;
}

void light_step(float h2, inout vec3 pos, inout vec3 v) {
	float r2 = dot(pos, pos);
	vec3 acc = -3. * h2 * pos / pow(r2, 2.5) * 1. ;
	pos += v * step_time;
	v += acc * step_time;
    //
}


void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec2 uv = fragCoord.xy / iResolution.xy;
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
    //color += alpha * texture(iChannel0, dir).xyz;

    
    fragColor = vec4(color, 1.);
}