#version 440

//layout(location = 0)
//out vec3 color_out;

uniform float time_total;
uniform vec3 resolution;
uniform sampler2D tex_disc;
uniform sampler2D tex_previous;

const vec3 cam_pos = vec3(0,0.5, -8);
const vec3 cam_dir = vec3(0, 0, 1);
const float cam_tan_half_v_angle = 1.;
const float disc_inner = 3.;
const float disc_mid = 3.5;
const float disc_outer = 6.;
const float disc_thickness = 0.2;
const float step_time_total = 0.01;
const int steps = 200;
const float disc_rot_inner = 0.15;
const float disc_rot_outer = 0.02;
const float r_cutoff = 6.5;

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

float rand(vec3 coord) {
    return saturate(fract(sin(dot(coord, vec3(12.9898, 78.223, 21.132))) * 43758.5453));
}

void get_disc(inout vec3 color, inout float alpha_remain, vec3 pos, vec3 old_pos) {

    bool use_mid = abs(pos.y - old_pos.y) > .01;
    vec2 mid = (pos.y * old_pos.xz - old_pos.y * pos.xz) / (pos.y - old_pos.y);
    vec2 xz0 = use_mid ? mid : pos.xz;
    float r = length(xz0);
    bool crossed = (pos.y * old_pos.y < 0.) || (abs(pos.y) < -.001);
	if (r >= disc_inner && r <= disc_outer && crossed) {
        float alpha = 1.;
		float h_alpha = 0.;
        //float h_alpha = (r >= disc_inner && r <= disc_mid) ? .8 : 0.;
        float ratio_mid = r / disc_mid;
        //h_alpha = (ratio_mid >= 1.) ? max(0., 1. + (r-disc_mid)/(disc_mid-disc_outer)) : h_alpha;
        h_alpha = (r >= disc_inner) ? saturate(pow(abs(r-disc_outer), 3.) / 8. )  : h_alpha;
        //h_alpha /= r * r * r / 30.;

        vec3 cc;
		
        float angle = atan(xz0.y, xz0.x);
        float u_inner = fract(time_total * disc_rot_inner +  angle / 2. /3.14159 + .5);
        float u_outer = fract(time_total * disc_rot_outer +  angle / 2. /3.14159 + .5);
        float v = 1. - (r - disc_inner) / (disc_outer - disc_inner);
        
        vec3 cc_inner = texture(tex_disc, vec2(u_inner, v)).rgb;
		vec3 cc_outer = texture(tex_disc, vec2(u_outer, v)).rgb;
		cc = mix(cc_inner, cc_outer, 1. - pow(v, 2.5));
        
        alpha = h_alpha;
        //alpha = (pos.y * old_pos.y > 0.) ? 0. : alpha;


        color += alpha * alpha_remain * cc;
        alpha_remain *= (1. - alpha);
     }

	bool in_photon_sphere = dot(pos, pos) <= 1.;
	alpha_remain = in_photon_sphere ? 0. : alpha_remain;
     
}

vec3 get_accel(float h2, vec3 pos) {
    float r2 = dot(pos, pos);
	vec3 acc = -1.5 * h2 * pos / pow(r2, 2.5) * 1. ;
    return acc;
}

void RK4f(float h2, out vec3 fp, out vec3 fv, vec3 p, vec3 v) {
    fp = v;
    fv = get_accel(h2, p);
}

void light_step(float h2, inout vec3 pos, inout vec3 v) {
	float r2 = dot(pos, pos);
	vec3 acc = get_accel(h2, pos);
    
    float dt = step_time_total;
    dt *= max(10., length(cam_pos));
	    
    float dd = dot(normalize(pos), normalize(v));
    float step_add = 0.;
    step_add += r2 >= 8. ? pow(r2, .35) : 0.;
    
    vec3 d_p, d_v;
    
    vec3 kp1, kp2, kp3, kp4;
    vec3 kv1, kv2, kv3, kv4;
    RK4f(h2, kp1, kv1, pos, v);
    RK4f(h2, kp2, kv2, pos + .5 * dt * kp1, v + .5 * dt * kv1);
    RK4f(h2, kp3, kv3, pos + .5 * dt * kp2, v + .5 * dt * kv2);
    RK4f(h2, kp4, kv4, pos + 1. * dt * kp3, v + 1. * dt * kv3);
    
    d_p = dt * (kp1 + 2. * kp2 + 2. * kp3 + kp4) / 6.;
    d_v = dt * (kv1 + 2. * kv2 + 2. * kv3 + kv4) / 6.;

	pos += d_p;
    v += d_v;
}


void mainImage(out vec4 fragColor)
{
	vec2 uv = gl_FragCoord.xy / resolution.xy;
    vec2 uv_origin = uv;

    //fragColor = texture(tex_disc, uv).rgb;
    //fragColor = vec3(uv, .2);
    //return;

    uv.x += (rand(uv + sin(time_total * 1.0)) / resolution.x);
    uv.y += (rand(uv + 1.0 + sin(time_total * 1.0)) / resolution.y);
    float rand = (rand(uv + 1.0 + sin(time_total * 1.0)) / 50.);

	float aspect = resolution.x / resolution.y;
	vec3 dir = vec3((uv * 2. - 1.) * vec2(aspect, 1.) * cam_tan_half_v_angle, 1.);
    //dir.z += rand;
    // dir = normalize(dir);
    
	vec3 pos = cam_pos;
	vec3 h = cross(pos, dir);
	float h2 = dot(h, h);
	vec3 color = vec3(0, 0, 0);
	float alpha = 1.;
    
    
    // 1. (faster, 120fps)
    /*do {
        if (length(cross(normalize(dir), pos)) > r_cutoff) break;
        for (int i = 0; i < steps; i++) {
            vec3 old = pos;
            light_step(h2, pos, dir);
            get_disc(color, alpha, pos, old);
        }
    
    } while (false);*/
    
    // 2. (slower, 30fps)
	if (length(cross(normalize(dir), pos)) <= r_cutoff)
        for (int i = 0; i < steps; i++) {
                vec3 old = pos;
                light_step(h2, pos, dir);
                get_disc(color, alpha, pos, old);
        }
    

    color *= 3.03;
    //color += texture(tex_previous, uv_origin).rgb;
    const float p = 1.0;
    vec3 previous = pow(texture(tex_previous, uv_origin).rgb, vec3(1.0 / p));
    color = pow(color, vec3(1.0 / p));
    float blendWeight = 0.9;
    color = mix(color, previous, blendWeight);
    color = pow(color, vec3(p));
    //color *= 2.;
    //color = texture(tex_previous, uv_origin).rgb;
    //return;

    fragColor = vec4(saturate(color), 1.);
}

void main() {
	mainImage(gl_FragColor);
}