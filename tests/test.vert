#version 330 core

in vec3 vertPos;
in vec2 vertTexCoord;
in vec3 vertNormal;
in vec4 vertCol;

out vec4 fragCol;
out vec2 fragTexCoord;

uniform mat4 mvp;
uniform vec2 winSize;
uniform float time;

mat4 rotate(vec3 angles) {
	mat4 pitch = mat4(
		vec4(1.,0.,0.,0.),
		vec4(0.,cos(angles.x),sin(angles.x),0.),
		vec4(0.,-sin(angles.x),cos(angles.x),0.),
		vec4(vec3(0.),1.)
	);
	mat4 yaw = mat4(
		vec4(cos(angles.y),0.,sin(angles.y),0.),
		vec4(0.,1.,0.,0.),
		vec4(-sin(angles.y),0.,cos(angles.y),0.),
		vec4(vec3(0.),1.)
	);
	mat4 roll = mat4(
		vec4(cos(angles.z),sin(angles.z),0.,0.),
		vec4(-sin(angles.z),cos(angles.z),0.,0.),
		vec4(0.,0.,1.,0.),
		vec4(vec3(0.),1.)
	);
	return roll*pitch*yaw;
}

void main() {
	//gl_Position = vec4(vertPos.xy/winSize*2. - 1., 1., 1.);
	//gl_Position = mvp * vec4(vertPos,1.);
	gl_Position.xy = vertPos.xy/winSize*2. - 1.;
	gl_Position.y = -gl_Position.y;
	gl_Position.zw = vec2(1.);
	fragCol = vertCol;
	fragTexCoord = vertTexCoord;
}
