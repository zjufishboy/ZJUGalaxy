#version 440

//layout(location = 0)
//out vec3 color_out;

uniform float time_total;
uniform vec3 resolution;
uniform sampler2D tex_disc;
uniform sampler2D tex_previous;
uniform sampler2D tex_sun;
uniform sampler2D tex_bg;
uniform mat4 cam_mat;
const vec3 cam_dir = vec3(0, 0, 1);
const float cam_tan_half_v_angle = .5;
const float disc_inner = 3.;
const float disc_mid = 3.5;
const float disc_outer = 6.;
const float disc_thickness = 0.2;
const float step_time_total = 0.01;
const int steps = 200;
const float disc_rot_inner = 0.15;
const float disc_rot_outer = 0.02;
const float r_cutoff = 3600.5;
uniform int samples;

const float star_orbit_r = 12.;
const float star_orbit_rot = 0.8;
const float star_r = 2.;
const float star_rot = 0.1;

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

vec3 cartesian_to_radial( in vec3 xyz ){
	float r = length( xyz );
	xyz *= 1.f/r;
	float theta = acos( xyz.y );
	float phi = atan( xyz.z, xyz.x );
	phi += ( phi < 0. ) ? 2. * 3.14159 : 0.;  // only if you want [0,2pi)
	return vec3(phi, theta, r );
}

vec2 radial_to_uv(vec3 rad) {
    float u = rad.x / 2. / 3.14159;
    float v = rad.y / 3.14159;
    return vec2(u, v);
}

float ray_sphere_intersect(vec3 ro, vec3 rd, vec3 sph, float sr )
{
  vec3 oc = ro - sph;
  float b = dot( oc, rd );
  float c = dot( oc, oc ) - sr * sr;
  float h = b*b - c;
  if( h<0.0 ) return -1.0;
  return -b - sqrt( h );
}

void get_event_horizon(inout vec3 color, inout float alpha_remain, vec3 pos, vec3 old_pos, vec3 cam_pos) {
    vec3 dir = pos - old_pos;
    vec3 norm_dir = normalize(dir);
    float cutoff = min(length(cam_pos), 1.5);
    float trav = ray_sphere_intersect(pos, norm_dir, vec3(0), cutoff);
    bool in_eh = (trav >= 0. && trav < length(dir)) || (length(pos) <= cutoff);
	alpha_remain = in_eh ? 0. : alpha_remain;
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
     
}

void get_star(inout vec3 color, inout float alpha_remain, vec3 pos, vec3 old_pos) {
    float alpha = 0.;
    vec3 cc;
    
    vec3 star_pos = vec3(sin(time_total * star_orbit_rot), 0, cos(time_total * star_orbit_rot));
    star_pos *= star_orbit_r;
    
    
    //vec3 delta = pos - star_pos;
    vec3 dir = pos - old_pos;
    vec3 norm_dir = normalize(dir);
    
    {
        float trav = ray_sphere_intersect(pos, norm_dir, star_pos, star_r);
        if (trav > 0. && trav < length(dir)) {
            vec3 hit_pos = trav * norm_dir + pos;
            vec3 delta = hit_pos - star_pos;

            alpha = 1.;
            vec3 rad = cartesian_to_radial(delta);
            vec2 uv = radial_to_uv(rad);
            uv.x = fract(uv.x + time_total * star_rot);
            cc = texture(tex_sun, uv).rgb * 0.3;
        }
    }
          
    {
        float trav = ray_sphere_intersect(pos, norm_dir, star_pos, star_r * 1.5);
        //vec3 delta = pos - star_pos;
        if (trav > 0. && trav < length(dir)) {
			vec3 hit_pos = trav * norm_dir + pos;
            vec3 delta = hit_pos - star_pos;
            vec3 r_v = cross(normalize(dir), delta / star_r);
            float r = dot(r_v, r_v);
            float f = (1.0-sqrt(abs(1.-r)))/(r);
            f = f*f;
            vec3 orange = vec3( 1.3, 0.65, 0.3 );
            cc += vec3( f * 1.75 * orange ) * 1.;
            alpha += f * 0.4;
        }
    }
    
    alpha = saturate(alpha);
    
    color += alpha * alpha_remain * cc;
    alpha_remain *= (1. - alpha);
}


vec3 get_accel(float h2, vec3 pos) {
    float r2 = dot(pos, pos);
    float r5 = pow(r2, 2.5);
	vec3 acc = -1.5 * h2 * pos / r5 * 1. ;
    return acc;
}

void RK4f(float h2, out vec3 fp, out vec3 fv, vec3 p, vec3 v) {
    fp = v;
    fv = get_accel(h2, p);
}

void light_step(vec3 cam_pos, float h2, inout vec3 pos, inout vec3 v) {
	float r2 = dot(pos, pos);

    float dt = step_time_total;
    dt *= max(10., length(cam_pos));
    
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

    vec3 color_sum;
    float alpha_sum;

    for (int i = 0; i < samples; i++) {
        vec2 uv_rand;
        uv_rand.x = (rand(uv + float(i) + sin(time_total * 1.0)) / resolution.x);
        uv_rand.y = (rand(uv + float(i) + 1.0 + sin(time_total * 1.0)) / resolution.y);

        uv = uv_origin + uv_rand;

        float aspect = resolution.x / resolution.y;
        vec3 dir = vec3((uv * 2. - 1.) * vec2(aspect, 1.) * cam_tan_half_v_angle, 1.);
        dir = normalize(dir);
        dir = (cam_mat * vec4(dir, 0)).xyz;
        
        vec3 cam_pos = (cam_mat * vec4(vec3(0), 1)).xyz;

        vec3 pos = cam_pos;
        vec3 h = cross(pos, dir);
        float h2 = dot(h, h);
        vec3 color = vec3(0, 0, 0);
	    float alpha = 1.;

        for (int i = 0; i < steps; i++) {
            vec3 old = pos;
            light_step(cam_pos, h2, pos, dir); // 184 fps
            get_event_horizon(color, alpha, pos, old, cam_pos); // 500fps
            get_disc(color, alpha, pos, old); // 600fps
            get_star(color, alpha, pos, old); // 600fps
        }
        
        color *= 3.03;

        vec3 c_bg = texture(tex_bg, radial_to_uv(cartesian_to_radial(dir))).rgb;
        color += alpha * c_bg * c_bg * 0.5;

        color_sum += color;
        alpha_sum += alpha;
    }

    vec3 color = color_sum / float(samples);
    float alpha = alpha_sum / float(samples);

    //color += texture(tex_previous, uv_origin).rgb;
    const float p = 1.0;
    vec3 previous = pow(texture(tex_previous, uv_origin).rgb, vec3(1.0 / p));
    color = pow(color, vec3(1.0 / p));
    float blendWeight = 0.5;
    color = mix(color, previous, blendWeight);
    color = pow(color, vec3(p));

    fragColor = vec4(saturate(color), 1.);
}

void main() {
	mainImage(gl_FragColor);
}