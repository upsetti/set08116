#include <glm\glm.hpp>
#include <glm\gtc\constants.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\euler_angles.hpp>
#include <glm\gtx\projection.hpp>
#include <iostream>

using namespace std;
using namespace glm;

int main() 
{
	// V E C T O R S
	vec2 u(1.0f, 0.0f);
	vec2 a(0.0f, 1.0f);

	vec3 v(0.0f, 1.0f, 0.0f);
	vec3 b(1.0f, 0.0f, 0.0f);

	vec4 w(0.0f, 0.0f, 1.0f, 1);
	vec4 c(1.0f, 0.0f, 0.0f, 1);

	float x = u.x;
	u.y = 10.0f;

	//addition/subtraction of vectors
	vec3 y = v + b;
	//scaling vectors
	vec3 e = 5.0f * b;
	//magnitude of a vector
	float l = length(v);
	//normalisation of vector
	vec3 normal = normalize(b);
	//dot product of vectors
	float d = dot(u, a);
	//vector projection
	vec3 projection = proj(w, c);
	//cross product
	vec3 crossproduct = cross(v, b);

	// M A T R I C E S
	//define matrice (identitiy matrix)
	mat4 m(1.0f); mat4 n(2.0f);  mat4 o(3.0f);
	//matrix addition/subtration
	mat4 m = n + o;
	//scalling matrices
	mat4 scaleup = 5.0f * n;
	mat4 scaledown = n / 5.0f;
	
	//matrix mupliplication
	mat4 matrix; //cant multiply a 3D vector by a 4x4 matrix;
	vec3 vector; // convert to 4D vector
	vec4 multiple = matrix * vec4(vector, 1.0f); 
	// We can always get the 3D version if explicit
	vec3 result = vec3(matrix * vec4(vector, 1.0f));

	// T R A N S F O R M A T I O N S
	//translation matrix
	mat4 movematrix = translate(mat4(1.0f), vec3(1.0f, 0.0f, 0.0f)); // We have to transform an 
														   //initial matrix - use identity
	//rotating matrices
	
}