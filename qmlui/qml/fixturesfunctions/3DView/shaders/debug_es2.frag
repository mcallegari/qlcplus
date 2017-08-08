#version 110

uniform sampler2D color;
uniform sampler2D position;
uniform sampler2D normal;
uniform sampler2D depth;
uniform vec2 winSize;

void main()
{
    vec2 texCoord = (gl_FragCoord.xy + vec2(-winSize.x, 0)) / winSize;

    // Draw 4 quadrants
    if (texCoord.x > 0.5) { // Right
        if (texCoord.y > 0.5) { // Top
            gl_FragColor = vec4(texture2D(normal, vec2(texCoord.x - 0.5, texCoord.y - 0.5) * 2.0).xyz, 1.0);
        } else { // Bottom
            gl_FragColor = vec4(texture2D(color, vec2(texCoord.x - 0.5, texCoord.y) * 2.0).xyz, 1.0);
        }
    } else { // Left
        if (texCoord.y > 0.5) { // Top
            gl_FragColor = texture2D(position, vec2(texCoord.x, texCoord.y - 0.5) * 2.0);
        } else { // Bottom
            gl_FragColor = vec4(texture2D(depth, texCoord * 2.0).rrr, 1.0);
        }
    }
    gl_FragColor.a = 0.5;
}
