#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_MATH_TRANSFORM_H
#define FAY_MATH_TRANSFORM_H

#include "fay/math/matrix.h"

namespace fay
{

/*

   (1). OpenGL                                            GLSL(in math same as OpenGL)

   math(M * V):            memory(M * V):        |        mul(M, V)
          | x                       | x          |                 | x
          | y                       | y          |                 | y
          | z                       | z          |                 | z 
          V w                       V w          |                 V w 
   - - - >                 | - - - -             |        | - - - - 
   - - - x                 | - - - -             |        | - - - - 
   - - - y                 | - - - -             |        | - - - - 
   - - - z                 V x y z w             |        V x y z w 
   - - - w                                       |

   -----------------------------------------------------------------------------

   (2). Direct3D                                          HLSL(in math same as Direct3D, you need to hold memory and order by yourself)

   math(V * M):            memory(V * M):        |        mul(V, M):                mul(M, V):
                                                 |
          | - - - -               | - - - -      |               | - - - -                 | x 
          | - - - -               | - - - -      |               | - - - -                 | y 
          | - - - -               | - - - -      |               | - - - -                 | z 
          V x y z w               V x y z w      |               V x y z w                 V w 
   - - - >                 - - - >               |        - - - >                   - - - > 
   x y z w                 x y z w               |        x y z w                   - - - x
                                                 |                                  - - - y
                                                 |                                  - - - z
                                                 |                                  - - - w

   -----------------------------------------------------------------------------

   (3). PBRT

   math(M * V):            memory(M * V):        |
          | x                     | x            |
          | y                     | y            |
          | z                     | z            |
          V w                     V w            |
   - - - >                 - - - >               |
   - - - x                 - - - x               |
   - - - y                 - - - y               |
   - - - z                 - - - z               |
   - - - w                 - - - w               |

*/

// now fay only support OpenGL



} // namespace fay

#endif // FAY_MATH_TRANSFORM_H

