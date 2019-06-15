
uniform sampler2D tex_disc;
uniform sampler2D tex_previous;
const vec3 cam_pos = vec3(0,0.5, -28);
const vec3 cam_dir = vec3(0, 0, 1);
const float cam_tan_half_v_angle = 1.;
const float disc_inner = 3.;
const float disc_mid = 3.5;
const float disc_outer = 6.;
const float disc_thickness = 0.2;
const float step_time = 0.01;
const int steps = 1600;
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

float noise( in vec3 x ) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);
    vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
    vec2 rg = textureLod( iChannel2, (uv+ 0.5)/256.0, 0.0 ).yx;
    return -1.0+2.0*mix( rg.x, rg.y, f.z );
    //return rand(x);
    return 0.1;
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
        float u_inner = fract(iTime * disc_rot_inner +  angle / 2. /3.14159 + .5);
        float u_outer = fract(iTime * disc_rot_outer +  angle / 2. /3.14159 + .5);
        float v = 1. - (r - disc_inner) / (disc_outer - disc_inner);
        
        vec3 cc_inner = texture(iChannel2, vec2(u_inner, v)).rgb;
		vec3 cc_outer = texture(iChannel3, vec2(u_outer, v)).rgb;
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
    
    float dt = step_time;
    dt *= max(10., length(cam_pos));
	    
    float dd = dot(normalize(pos), normalize(v));
    float step_add = 0.;
    step_add += r2 >= 8. ? pow(r2, .35) : 0.;
    //step_add *= saturate(abs(pos.y));
    //t *= 1. - dd * 0.5;
    //dt *= 1. + step_add;
	
	//t *= 1. + dd * .5;
    
    //t *= .11 + saturate(pos.y);

    //d_v = acc * dt;
    //d_p = v * dt;
    
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


void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec2 uv = fragCoord.xy / iResolution.xy;
    vec2 uv_origin = uv;
    uv.x += (rand(uv + sin(iTime * 1.0)) / iResolution.x) * (iMouse.z > 1.0 ? 0.0 : 1.0);
    uv.y += (rand(uv + 1.0 + sin(iTime * 1.0)) / iResolution.y) * (iMouse.z > 1.0 ? 0.0 : 1.0);
    float rand = (rand(uv + 1.0 + sin(iTime * 1.0)) / 50.);

	float aspect = iResolution.x / iResolution.y;
	vec3 dir = vec3((uv * 2. - 1.) * vec2(aspect, 1.) * cam_tan_half_v_angle, 1.);
    //dir.z += rand;
    // dir = normalize(dir);
    
	vec3 pos = cam_pos;
	vec3 h = cross(pos, dir);
	float h2 = dot(h, h);
	vec3 color = vec3(0, 0, 0);
	float alpha = 1.;
    
    
    // 1. (faster, 120fps)
    do {
        if (length(cross(normalize(dir), pos)) > r_cutoff) break;
        for (int i = 0; i < steps; i++) {
            vec3 old = pos;
            light_step(h2, pos, dir);
            get_disc(color, alpha, pos, old);
        }
    
    } while (false);
    
    // 2. (slower, 30fps)
    /*bool cond = length(cross(normalize(dir), pos)) <= r_cutoff;
	if (cond)
        for (int i = 0; i < steps; i++) {

                vec3 old = pos;
                light_step(h2, pos, dir);
                get_disc(color, alpha, pos, old);
        }*/
    

    color *= 0.03;
    
    const float p = 1.0;
    vec3 previous = pow(texture(iChannel0, uv_origin).rgb, vec3(1.0 / p));
    color = pow(color, vec3(1.0 / p));
    float blendWeight = 0.9 * (iMouse.z > 1.0 ? 0.0 : 1.0);
    color = mix(color, previous, blendWeight);
    color = pow(color, vec3(p));
    
    fragColor = vec4(saturate(color), 1.);
}