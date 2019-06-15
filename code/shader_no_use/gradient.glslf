uniform sampler2D iChannel2;
uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iTime;                 // shader playback time (in seconds)

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.xy;

    // Time varying pixel color
    vec3 col = 0.5 + 0.5*cos(iTime+uv.xyx+vec3(0,2,4));

    col = texture(iChannel2, uv);

    // Output to screen
    fragColor = vec4(col,1.0);
}

void main() {
	mainImage(gl_FragColor,gl_FragCoord.xy);
}