#version 330 core

in vec3 vertPos;
in vec2 vertTexCoord;
in vec3 vertNormal;
in vec4 vertCol;

out vec4 fragCol;
out vec2 fragTexCoord;

uniform mat4 mvp;
uniform vec2 winSize;

void main() {
    gl_Position.xy = vertPos.xy / winSize * 2. - 1.;
    gl_Position.y = -gl_Position.y;
    gl_Position.zw = vec2(1.);
    fragCol = vertCol;
    fragTexCoord = vertTexCoord;
}
