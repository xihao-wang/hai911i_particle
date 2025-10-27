#version 430

out vec4 fragColor;

// Input du geometry shader
in vec2 UV;

uniform vec4 color = vec4(1.0, 0.0, 0.0, 1.0);
uniform sampler2D texture2D;

void main(void)
{
    vec4 c = texture(texture2D, UV);

    if (c.a < 0.01) discard;
    
    // Utiliser les coordonnées UV pour débugger : un cadre noir autour des particules
    // if (UV.x < 0.1 || UV.x > 0.9 || UV.y < 0.1 || UV.y > 0.9)
    //     fragColor = vec4(0.0, 0.0, 0.0, c.a);
    // else
        fragColor = c;
}