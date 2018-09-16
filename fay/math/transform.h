#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_MATH_TRANSFORM_H
#define FAY_MATH_TRANSFORM_H

#include "fay/math/matrix.h"

namespace fay
{

/*

   (1). OpenGL                                                GLSL(in math same as OpenGL)

   math(M * V):                memory(M * V):        |        mul(M, V)
          | x                           | x          |                 | x
          | y                           | y          |                 | y
          | z                           | z          |                 | z 
          V w                           V w          |                 V w 
   - - - >                     | - - - -             |        | - - - - 
   - - - x                     | - - - -             |        | - - - - 
   - - - y                     | - - - -             |        | - - - - 
   - - - z                     V x y z w             |        V x y z w 
   - - - w                                           |

   -----------------------------------------------------------------------------

   (2). Direct3D                                              HLSL(in math same as Direct3D, you need to hold memory and order by yourself)

   math(V * M):                memory(V * M):        |        mul(V, M):                mul(M, V):
                                                     |
          | - - - -                   | - - - -      |               | - - - -                 | x 
          | - - - -                   | - - - -      |               | - - - -                 | y 
          | - - - -                   | - - - -      |               | - - - -                 | z 
          V x y z w                   V x y z w      |               V x y z w                 V w 
   - - - >                     - - - >               |        - - - >                   - - - > 
   x y z w                     x y z w               |        x y z w                   - - - x
                                                     |                                  - - - y
                                                     |                                  - - - z
                                                     |                                  - - - w

   -----------------------------------------------------------------------------

   (3). PBRT

   math(M * V):                memory(M * V):        |
          | x                         | x            |
          | y                         | y            |
          | z                         | z            |
          V w                         V w            |
   - - - >                     - - - >               |
   - - - x                     - - - x               |
   - - - y                     - - - y               |
   - - - z                     - - - z               |
   - - - w                     - - - w               |

*/

// now fay only support OpenGL


/*
   math:                memory: 

   - - - >
   1 0 0 x              | 1 0 0 0
   0 1 0 y              | 0 1 0 0
   0 0 1 z              | 0 0 1 0
   0 0 0 1              V x y z 1
*/
template<typename T>
inline mat<4, 4, T> translate(const vec<3, T>& v)
{
	T x = v.x, y = v.y, z = v.z;

	mat<4, 4, T> r{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		x, y, z, 1
	};
	return r;
}
/*
   math:                                memory:
           | 1 0 0 x                                - - - >
           | 0 1 0 y                                1 0 0 0 
           | 0 0 1 z                                0 1 0 0
           V 0 0 0 1                                0 0 1 0
   - - - >                                          x y z 1
   - - - -   - - - ?                    | - - - -   - - - - 
   - - - -   - - - ?                    | - - - -   - - - -
   - - - -   - - - ?                    | - - - -   - - - -
   - - - -   - - - ?                    V - - - -   ? ? ? ?

   it is clear that only r[3] were change

   r[0] =         r[1] =         r[2] =         r[3] =     
   m[0][0] * x    m[0][1] * x    m[0][2] * x    m[0][3] * x
        +              +              +              +
   m[1][0] * y    m[1][1] * y    m[1][2] * y    m[1][3] * y
        +              +              +              +
   m[2][0] * z    m[2][1] * z    m[2][2] * z    m[2][3] * z
        +              +              +              +
   m[3][0] * 1    m[3][1] * 1    m[3][2] * z    m[3][3] * 1

   it equal to r[3] = m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3] * 1
*/
template<typename T>
inline mat<4, 4, T> translate(const mat<4, 4, T>& m, const vec<3, T>& v)
{
	// mat<4, 4, T> r(m);
	// r[3] = m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3] * 1;
	// return r;
	mat<4, 4, T> t{1}; t[3] = vec<4, T>{ v.x, v.y, v.z, 1 };
	return m * t;
}

/*
Transform scale(Float x, Float y, Float z);
Transform rotateX(Float theta);
Transform rotateY(Float theta);
Transform rotateZ(Float theta);
Transform rotate(Float theta, const Vector3f &axis);
Transform look_at(const Point3f &pos, const Point3f &look, const Vector3f &up);
Transform orthographic(Float znear, Float zfar);
Transform perspective(Float fov, Float znear, Float zfar);
*/


} // namespace fay

#endif // FAY_MATH_TRANSFORM_H

