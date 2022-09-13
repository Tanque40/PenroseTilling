#shader vertex
#version 410 core

layout( location = 0 ) in vec3 aPos;   // the position variable has attribute position 0
layout( location = 1 ) in vec3 o_Color; // the color variable has attribute position 1
layout( location = 2 ) in vec2 texCoord; // the texture variable has attribute position 2
layout( location = 3 ) in float o_TexIndex; // the texture index variable has attribute psition 3
layout( location = 4 ) in vec3 aNormal; // the normal light variable has attribute psition 4

out  VS_OUT{
    vec2 ourTexture; // output a texture to the fragment shader
    float v_TexIndex;
    vec3 v_Color;
    vec3 v_Normal;
    vec3 v_FragPos;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
    vs_out.v_FragPos = vec3( model * vec4( aPos, 1.0 ) );
    gl_Position = projection * view * vec4( vs_out.v_FragPos, 1.0 );
    vs_out.ourTexture = texCoord; // set ourColor to the input color we got from the vertex data
    vs_out.v_TexIndex = o_TexIndex;
    vs_out.v_Color = o_Color;
    vs_out.v_Normal = aNormal;
}


#shader geometry
#version 410 core
layout( triangles ) in;
layout( triangle_strip, max_vertices = 3 ) out;

in VS_OUT{
    vec2 ourTexture;
    float v_TexIndex;
    vec3 v_Color;
    vec3 v_Normal;
    vec3 v_FragPos;
} gs_in[];

out vec2 OurTexture;
out vec3 g_Color;
out vec3 g_Normal;
out vec3 g_FragPos;
out float g_TexIndex;

uniform float time;
uniform float magnitude;
vec4 explode( vec4 position, vec3 normal ){

    vec3 direction = normal * ( ( sin( time ) + 1.0 ) / 2.0 ) * magnitude;
    return position + vec4( direction, 0.0 );
}

vec3 GetNormal(){
    vec3 a = vec3( gl_in[ 0 ].gl_Position ) - vec3( gl_in[ 1 ].gl_Position );
    vec3 b = vec3( gl_in[ 2 ].gl_Position ) - vec3( gl_in[ 1 ].gl_Position );
    vec3 c = a + b;
    return normalize( c );
}

void main(){
    vec3 normal;

    if( time == 0 ){
        normal = vec3( 0.0, 0.0, 0.0 );
    } else{

        normal = GetNormal();
    }

    gl_Position = explode( gl_in[ 0 ].gl_Position, normal );
    OurTexture = gs_in[ 0 ].ourTexture;
    g_TexIndex = gs_in[ 0 ].v_TexIndex;
    g_Color = gs_in[ 0 ].v_Color;
    g_Normal = gs_in[ 0 ].v_Normal;
    g_FragPos = gs_in[ 0 ].v_FragPos;
    EmitVertex();
    gl_Position = explode( gl_in[ 1 ].gl_Position, normal );
    OurTexture = gs_in[ 1 ].ourTexture;
    g_TexIndex = gs_in[ 1 ].v_TexIndex;
    g_Color = gs_in[ 1 ].v_Color;
    g_Normal = gs_in[ 1 ].v_Normal;
    g_FragPos = gs_in[ 1 ].v_FragPos;
    EmitVertex();
    gl_Position = explode( gl_in[ 2 ].gl_Position, normal );
    OurTexture = gs_in[ 2 ].ourTexture;
    g_TexIndex = gs_in[ 2 ].v_TexIndex;
    g_Color = gs_in[ 2 ].v_Color;
    g_Normal = gs_in[ 2 ].v_Normal;
    g_FragPos = gs_in[ 2 ].v_FragPos;
    EmitVertex();
    EndPrimitive();

}


#shader fragment
#version 410 core

out vec4 FragColor;

struct Material{
    sampler2D diffuse[ 3 ];
    vec3 specular;
    float shininess;
};

struct DirLight{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight{
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define NR_POINT_LIGHTS 4

in vec2 OurTexture;

in float g_TexIndex;
in vec3 g_Color;
in vec3 g_Normal;
in vec3 g_FragPos;

int index = int( g_TexIndex );

//uniform sampler2D u_Textures[3];
uniform vec3 viewPos;
uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[ NR_POINT_LIGHTS ];
uniform SpotLight spotLight;

// function prototypes
vec3 CalcDirLight( DirLight light, vec3 normal, vec3 viewDir );
vec3 CalcPointLight( PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir );
vec3 CalcSpotLight( SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir );

void main(){

    // properties
    vec3 norm = normalize( g_Normal );
    vec3 viewDir = normalize( viewPos - g_FragPos );

    // == =====================================================
    // Our lighting is set up in 3 phases: directional, point lights and an optional flashlight
    // For each phase, a calculate function is defined that calculates the corresponding color
    // per lamp. In the main() function we take all the calculated colors and sum them up for
    // this fragment's final color.
    // == =====================================================
    // phase 1: directional lighting
    vec3 result = CalcDirLight( dirLight, norm, viewDir );
    // phase 2: point lights
    for( int i = 0; i < NR_POINT_LIGHTS; i++ )
        result += CalcPointLight( pointLights[ i ], norm, g_FragPos, viewDir );
    // phase 3: spot light
    result += CalcSpotLight( spotLight, norm, g_FragPos, viewDir );

    FragColor = vec4( result, 1.0 );

    //vec4 texColor = texture( u_Textures[ index ], OurTexture );
    //color = texColor * vec4( g_Color, 1.0 );

}

// calculates the color when using a directional light.
vec3 CalcDirLight( DirLight light, vec3 normal, vec3 viewDir ){
    vec3 lightDir = normalize( -light.direction );
    // diffuse shading
    float diff = max( dot( normal, lightDir ), 0.0 );
    // specular shading
    vec3 reflectDir = reflect( -lightDir, normal );
    float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), material.shininess );
    // combine results
    vec3 ambient = light.ambient * vec3( texture( material.diffuse[ index ], OurTexture ) );
    vec3 diffuse = light.diffuse * diff * vec3( texture( material.diffuse[ index ], OurTexture ) );
    vec3 specular = light.specular * spec * material.specular;
    return ( ambient + diffuse + specular );
}

// calculates the color when using a point light.
vec3 CalcPointLight( PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir ){
    vec3 lightDir = normalize( light.position - fragPos );
    // diffuse shading
    float diff = max( dot( normal, lightDir ), 0.0 );
    // specular shading
    vec3 reflectDir = reflect( -lightDir, normal );
    float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), material.shininess );
    // attenuation
    float distance = length( light.position - fragPos );
    float attenuation = 1.0 / ( light.constant + light.linear * distance + light.quadratic * ( distance * distance ) );
    // combine results
    vec3 ambient = light.ambient * vec3( texture( material.diffuse[ index ], OurTexture ) );
    vec3 diffuse = light.diffuse * diff * vec3( texture( material.diffuse[ index ], OurTexture ) );
    vec3 specular = light.specular * spec * material.specular;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return ( ambient + diffuse + specular );
}

// calculates the color when using a spot light.
vec3 CalcSpotLight( SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir ){
    vec3 lightDir = normalize( light.position - fragPos );
    // diffuse shading
    float diff = max( dot( normal, lightDir ), 0.0 );
    // specular shading
    vec3 reflectDir = reflect( -lightDir, normal );
    float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), material.shininess );
    // attenuation
    float distance = length( light.position - fragPos );
    float attenuation = 1.0 / ( light.constant + light.linear * distance + light.quadratic * ( distance * distance ) );
    // spotlight intensity
    float theta = dot( lightDir, normalize( -light.direction ) );
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp( ( theta - light.outerCutOff ) / epsilon, 0.0, 1.0 );
    // combine results
    vec3 ambient = light.ambient * vec3( texture( material.diffuse[ index ], OurTexture ) );
    vec3 diffuse = light.diffuse * diff * vec3( texture( material.diffuse[ index ], OurTexture ) );
    vec3 specular = light.specular * spec * material.specular;
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return ( ambient + diffuse + specular );
}