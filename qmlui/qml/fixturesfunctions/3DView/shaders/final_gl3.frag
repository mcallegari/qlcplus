#version 140

uniform sampler2D color;
uniform sampler2D position;
uniform sampler2D normal;
uniform vec2 winSize;

out vec4 fragColor;

const int MAX_LIGHTS = 102;
const int TYPE_POINT = 0;
const int TYPE_DIRECTIONAL = 1;
const int TYPE_SPOT = 2;

struct Light {
    int   type;
    vec3  position;
    vec3  color;
    float intensity;
    vec3  direction;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
    float cutOffAngle;
};

uniform Light lightsArray[MAX_LIGHTS];
uniform int lightsNumber;

void main()
{
    vec2 texCoord = gl_FragCoord.xy / winSize;
    vec4 col = texture(color, texCoord);
    vec3 pos = texture(position, texCoord).xyz;
    vec3 norm = texture(normal, texCoord).xyz;

    vec3 lightColor = vec3(0.0);
    vec3 s;
    float att;

    for (int i = 0; i < lightsNumber; ++i) {
        att = 1.0;
        if ( lightsArray[i].type != TYPE_DIRECTIONAL ) {
            s = lightsArray[i].position - pos;
            if (lightsArray[i].constantAttenuation != 0.0
             || lightsArray[i].linearAttenuation != 0.0
             || lightsArray[i].quadraticAttenuation != 0.0) {
                float dist = length(s);
                att = 1.0 / (lightsArray[i].constantAttenuation + lightsArray[i].linearAttenuation * dist + lightsArray[i].quadraticAttenuation * dist * dist);
            }
            s = normalize( s );
            if ( lightsArray[i].type == TYPE_SPOT ) {
                if ( degrees(acos(dot(-s, normalize(lightsArray[i].direction))) ) > lightsArray[i].cutOffAngle)
                    att = 0.0;
            }
        } else {
            s = normalize(-lightsArray[i].direction);
        }

        float diffuse = max( dot( s, norm ), 0.0 );

        lightColor += att * lightsArray[i].intensity * diffuse * lightsArray[i].color;
    }
    fragColor = vec4(col.rgb * lightColor, col.a);
}
