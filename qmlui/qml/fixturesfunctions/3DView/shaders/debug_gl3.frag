#version 150

uniform sampler2D color;
uniform sampler2D position;
uniform sampler2D normal;
uniform sampler2D depth;
uniform vec2 winSize;

out vec4 fragColor;

void main()
{
    vec2 texCoord = (gl_FragCoord.xy + vec2(-winSize.x, 0)) / winSize;

    // Draw 4 quadrants
    if (texCoord.x > 0.5) { // Right
        if (texCoord.y > 0.5) { // Top
            fragColor = vec4(texture(normal, vec2(texCoord.x - 0.5, texCoord.y - 0.5) * 2.0).xyz, 1.0);
        } else { // Bottom
            fragColor = vec4(texture(color, vec2(texCoord.x - 0.5, texCoord.y) * 2.0).xyz, 1.0);
        }
    } else { // Left
        if (texCoord.y > 0.5) { // Top
            fragColor = texture(position, vec2(texCoord.x, texCoord.y - 0.5) * 2.0);
        } else { // Bottom
            fragColor = vec4(texture(depth, texCoord * 2.0).rrr, 1.0);
        }
    }
    fragColor.a = 0.5;
}
