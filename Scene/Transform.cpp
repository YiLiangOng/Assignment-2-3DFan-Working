// Transform.cpp -- implementation of the Transform matrix stack.
//
// Original source: OpenGL ES 3.0 Programming Guide framework (GLPIFramework)
// Modified by   : Parminder Singh
// Modifications : Single include -- Transform.h is now self-contained (no
//                 dependency on glutils.h or constant.h).
//
// MIT License -- see Transform.h for full license text.

#include "Transform.h"

Transform::Transform(void)
{
    TransformMemData.modelMatrixIndex      = 0;
    TransformMemData.viewMatrixIndex       = 0;
    TransformMemData.projectionMatrixIndex = 0;
    TransformMemData.textureMatrixIndex    = 0;
}

Transform::~Transform(void)
{
}

void Transform::TransformInit( void )
{
    memset( &TransformMemData, 0, sizeof( TransformData ) );

    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE  );
    glDisable( GL_DITHER );
    glDepthMask( GL_TRUE );
    glDepthFunc( GL_LESS );
    glDepthRangef( 0.0f, 1.0f );
    glClearDepthf( 1.0f );
    glCullFace ( GL_BACK );
    glFrontFace( GL_CCW  );
    glClearStencil( 0 );
    glStencilMask( 0xFFFFFFFF );

    glClear( GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

    TransformSetMatrixMode( TEXTURE_MATRIX );    TransformLoadIdentity();
    TransformSetMatrixMode( PROJECTION_MATRIX ); TransformLoadIdentity();
    TransformSetMatrixMode( MODEL_MATRIX );      TransformLoadIdentity();
    TransformSetMatrixMode( VIEW_MATRIX );       TransformLoadIdentity();

    TransformError();
}

void Transform::TransformError( void )
{
    unsigned int error;
    while ( ( error = glGetError() ) != GL_NO_ERROR )
    {
        char str[ MAX_CHAR ] = {""};
        switch ( error )
        {
            case GL_INVALID_ENUM:      strcpy( str, "GL_INVALID_ENUM" );      break;
            case GL_INVALID_VALUE:     strcpy( str, "GL_INVALID_VALUE" );     break;
            case GL_INVALID_OPERATION: strcpy( str, "GL_INVALID_OPERATION" ); break;
            case GL_OUT_OF_MEMORY:     strcpy( str, "GL_OUT_OF_MEMORY" );     break;
        }
        LOGI( "[ GL_ERROR ] %s\n", str );
    }
}

void Transform::TransformSetMatrixMode( unsigned int mode )
{
    TransformMemData.matrix_mode = mode;
}

void Transform::TransformLoadIdentity( void )
{
    switch ( TransformMemData.matrix_mode )
    {
        case MODEL_MATRIX:
        {
            memset( TransformGetModelMatrix(), 0, sizeof(glm::mat4) );
            glm::mat4 *mv = TransformGetModelMatrix();
            (*mv)[0][0] = (*mv)[1][1] = (*mv)[2][2] = (*mv)[3][3] = 1.0f;
            break;
        }
        case VIEW_MATRIX:
        {
            memset( TransformGetViewMatrix(), 0, sizeof(glm::mat4) );
            glm::mat4 *mv = TransformGetViewMatrix();
            (*mv)[0][0] = (*mv)[1][1] = (*mv)[2][2] = (*mv)[3][3] = 1.0f;
            break;
        }
        case PROJECTION_MATRIX:
        {
            memset( TransformGetProjectionMatrix(), 0, sizeof(glm::mat4) );
            glm::mat4 *pm = TransformGetProjectionMatrix();
            (*pm)[0][0] = (*pm)[1][1] = (*pm)[2][2] = (*pm)[3][3] = 1.0f;
            break;
        }
        case TEXTURE_MATRIX:
        {
            memset( TransformGetTextureMatrix(), 0, sizeof(glm::mat4) );
            glm::mat4 *tm = TransformGetTextureMatrix();
            (*tm)[0][0] = (*tm)[1][1] = (*tm)[2][2] = (*tm)[3][3] = 1.0f;
            break;
        }
    }
}

glm::mat4 *Transform::TransformGetModelMatrix( void )
{
    return &TransformMemData.model_matrix[ TransformMemData.modelMatrixIndex ];
}

glm::mat4 *Transform::TransformGetViewMatrix( void )
{
    return &TransformMemData.view_matrix[ TransformMemData.viewMatrixIndex ];
}

glm::mat4 *Transform::TransformGetProjectionMatrix( void )
{
    return &TransformMemData.projection_matrix[ TransformMemData.projectionMatrixIndex ];
}

glm::mat4 *Transform::TransformGetTextureMatrix( void )
{
    return &TransformMemData.texture_matrix[ TransformMemData.textureMatrixIndex ];
}

glm::mat4 *Transform::TransformGetModelViewProjectionMatrix( void )
{
    TransformMemData.modelview_projection_matrix =
        *TransformGetProjectionMatrix() *
        *TransformGetViewMatrix()       *
        *TransformGetModelMatrix();
    return &TransformMemData.modelview_projection_matrix;
}

glm::mat4 *Transform::TransformGetModelViewMatrix( void )
{
    TransformMemData.modelview_matrix =
        *TransformGetViewMatrix() * *TransformGetModelMatrix();
    return &TransformMemData.modelview_matrix;
}

void Transform::TransformPushMatrix( void )
{
    switch ( TransformMemData.matrix_mode )
    {
        case MODEL_MATRIX:
            if ( TransformMemData.modelMatrixIndex >= MAX_MODEL_MATRIX ) return;
            memcpy( &TransformMemData.model_matrix[ TransformMemData.modelMatrixIndex + 1 ],
                    &TransformMemData.model_matrix[ TransformMemData.modelMatrixIndex     ],
                    sizeof(glm::mat4) );
            ++TransformMemData.modelMatrixIndex;
            break;

        case VIEW_MATRIX:
            if ( TransformMemData.viewMatrixIndex >= MAX_VIEW_MATRIX ) return;
            memcpy( &TransformMemData.view_matrix[ TransformMemData.viewMatrixIndex + 1 ],
                    &TransformMemData.view_matrix[ TransformMemData.viewMatrixIndex     ],
                    sizeof(glm::mat4) );
            ++TransformMemData.viewMatrixIndex;
            break;

        case PROJECTION_MATRIX:
            if ( TransformMemData.projectionMatrixIndex >= MAX_PROJECTION_MATRIX ) return;
            memcpy( &TransformMemData.projection_matrix[ TransformMemData.projectionMatrixIndex + 1 ],
                    &TransformMemData.projection_matrix[ TransformMemData.projectionMatrixIndex     ],
                    sizeof(glm::mat4) );
            ++TransformMemData.projectionMatrixIndex;
            break;

        case TEXTURE_MATRIX:
            if ( TransformMemData.textureMatrixIndex >= MAX_TEXTURE_MATRIX ) return;
            memcpy( &TransformMemData.texture_matrix[ TransformMemData.textureMatrixIndex + 1 ],
                    &TransformMemData.texture_matrix[ TransformMemData.textureMatrixIndex     ],
                    sizeof(glm::mat4) );
            ++TransformMemData.textureMatrixIndex;
            break;
    }
}

void Transform::TransformPopMatrix( void )
{
    switch ( TransformMemData.matrix_mode )
    {
        case MODEL_MATRIX:
            if ( TransformMemData.modelMatrixIndex == 0 ) return;
            --TransformMemData.modelMatrixIndex;
            break;
        case VIEW_MATRIX:
            if ( TransformMemData.viewMatrixIndex == 0 ) return;
            --TransformMemData.viewMatrixIndex;
            break;
        case PROJECTION_MATRIX:
            if ( TransformMemData.projectionMatrixIndex == 0 ) return;
            --TransformMemData.projectionMatrixIndex;
            break;
        case TEXTURE_MATRIX:
            if ( TransformMemData.textureMatrixIndex == 0 ) return;
            --TransformMemData.textureMatrixIndex;
            break;
    }
}

void Transform::TransformLoadMatrix( glm::mat4 *m )
{
    switch ( TransformMemData.matrix_mode )
    {
        case MODEL_MATRIX:      memcpy( TransformGetModelMatrix(),      m, sizeof(glm::mat4) ); break;
        case VIEW_MATRIX:       memcpy( TransformGetViewMatrix(),       m, sizeof(glm::mat4) ); break;
        case PROJECTION_MATRIX: memcpy( TransformGetProjectionMatrix(), m, sizeof(glm::mat4) ); break;
        case TEXTURE_MATRIX:    memcpy( TransformGetTextureMatrix(),    m, sizeof(glm::mat4) ); break;
    }
}

void Transform::TransformMultiplyMatrix( glm::mat4 *m )
{
    switch ( TransformMemData.matrix_mode )
    {
        case MODEL_MATRIX:      *TransformGetModelMatrix()      *= (*m); break;
        case VIEW_MATRIX:       *TransformGetViewMatrix()       *= (*m); break;
        case PROJECTION_MATRIX: *TransformGetProjectionMatrix() *= (*m); break;
        case TEXTURE_MATRIX:    *TransformGetTextureMatrix()    *= (*m); break;
    }
}

void Transform::TransformTranslate( float x, float y, float z )
{
    glm::vec3 v( x, y, z );
    switch ( TransformMemData.matrix_mode )
    {
        case MODEL_MATRIX:
            *TransformGetModelMatrix() = glm::translate( *TransformGetModelMatrix(), v ); break;
        case VIEW_MATRIX:
            *TransformGetViewMatrix() = glm::translate( *TransformGetViewMatrix(), v ); break;
        case PROJECTION_MATRIX:
            *TransformGetProjectionMatrix() = glm::translate( *TransformGetProjectionMatrix(), v ); break;
        case TEXTURE_MATRIX:
            *TransformGetTextureMatrix() = glm::translate( *TransformGetTextureMatrix(), v ); break;
    }
}

void Transform::TransformRotate( float angle, float x, float y, float z )
{
    if ( !angle ) return;
    glm::vec3 v( x, y, z );
    switch ( TransformMemData.matrix_mode )
    {
        case MODEL_MATRIX:
            *TransformGetModelMatrix() = glm::rotate( *TransformGetModelMatrix(), angle, v ); break;
        case VIEW_MATRIX:
            *TransformGetViewMatrix() = glm::rotate( *TransformGetViewMatrix(), angle, v ); break;
        case PROJECTION_MATRIX:
            *TransformGetProjectionMatrix() = glm::rotate( *TransformGetProjectionMatrix(), angle, v ); break;
        case TEXTURE_MATRIX:
            *TransformGetTextureMatrix() = glm::rotate( *TransformGetTextureMatrix(), angle, v ); break;
    }
}

void Transform::TransformScale( float x, float y, float z )
{
    static glm::vec3 identity( 1.0f, 1.0f, 1.0f );
    glm::vec3 v( x, y, z );
    if ( !memcmp( &v, &identity, sizeof(glm::vec3) ) ) return;
    switch ( TransformMemData.matrix_mode )
    {
        case MODEL_MATRIX:
            *TransformGetModelMatrix() = glm::scale( *TransformGetModelMatrix(), v ); break;
        case VIEW_MATRIX:
            *TransformGetViewMatrix() = glm::scale( *TransformGetViewMatrix(), v ); break;
        case PROJECTION_MATRIX:
            *TransformGetProjectionMatrix() = glm::scale( *TransformGetProjectionMatrix(), v ); break;
        case TEXTURE_MATRIX:
            *TransformGetTextureMatrix() = glm::scale( *TransformGetTextureMatrix(), v ); break;
    }
}

void Transform::TransformGetNormalMatrix( glm::mat3 *mat3Obj )
{
    glm::mat4 mat4Obj = glm::transpose( glm::inverse( *TransformGetModelViewMatrix() ) );
    *mat3Obj = glm::mat3( mat4Obj );
}

void Transform::TransformOrtho( float left, float right, float bottom, float top,
                                 float clip_start, float clip_end )
{
    switch ( TransformMemData.matrix_mode )
    {
        case MODEL_MATRIX:
            *TransformGetModelMatrix() = glm::ortho(left,right,bottom,top,clip_start,clip_end); break;
        case VIEW_MATRIX:
            *TransformGetViewMatrix()  = glm::ortho(left,right,bottom,top,clip_start,clip_end); break;
        case PROJECTION_MATRIX:
            *TransformGetProjectionMatrix() = glm::ortho(left,right,bottom,top,clip_start,clip_end); break;
        case TEXTURE_MATRIX:
            *TransformGetTextureMatrix() = glm::ortho(left,right,bottom,top,clip_start,clip_end); break;
    }
}

void Transform::TransformOrthoGrahpic( float screen_ratio, float scale,
                                        float aspect_ratio, float clip_start,
                                        float clip_end, float orientation )
{
    scale = ( scale * 0.5f ) * aspect_ratio;
    TransformOrtho( -1.0f, 1.0f, -screen_ratio, screen_ratio, clip_start, clip_end );
    TransformScale( 1.0f / scale, 1.0f / scale, 1.0f );
    if ( orientation )
        TransformRotate( orientation, 0.0f, 0.0f, 1.0f );
}

void Transform::TransformSetPerspective( float fovy, float aspect_ratio,
                                          float clip_start, float clip_end,
                                          float screen_orientation )
{
    switch ( TransformMemData.matrix_mode )
    {
        case MODEL_MATRIX:
            *TransformGetModelMatrix() = glm::perspective(fovy,aspect_ratio,clip_start,clip_end); break;
        case VIEW_MATRIX:
            *TransformGetViewMatrix()  = glm::perspective(fovy,aspect_ratio,clip_start,clip_end); break;
        case PROJECTION_MATRIX:
            *TransformGetProjectionMatrix() = glm::perspective(fovy,aspect_ratio,clip_start,clip_end); break;
        case TEXTURE_MATRIX:
            *TransformGetTextureMatrix() = glm::perspective(fovy,aspect_ratio,clip_start,clip_end); break;
    }
}

void Transform::TransformSetView( glm::mat4 mat )
{
    // Reserved for future use.
}

void Transform::TransformLookAt( glm::vec3 *eye, glm::vec3 *center, glm::vec3 *up )
{
    glm::mat4 mat = glm::lookAt( *eye, *center, *up );
    TransformMultiplyMatrix( &mat );
}

void Transform::Vec4MultiplyMat4( glm::vec4 *dst, glm::vec4 *v, glm::mat4 *m )
{
    dst->x = v->x*(*m)[0].x + v->y*(*m)[1].x + v->z*(*m)[2].x + v->w*(*m)[3].x;
    dst->y = v->x*(*m)[0].y + v->y*(*m)[1].y + v->z*(*m)[2].y + v->w*(*m)[3].y;
    dst->z = v->x*(*m)[0].z + v->y*(*m)[1].z + v->z*(*m)[2].z + v->w*(*m)[3].z;
    dst->w = v->x*(*m)[0].w + v->y*(*m)[1].w + v->z*(*m)[2].w + v->w*(*m)[3].w;
}

int Transform::TransformProject( float objx, float objy, float objz,
                                  glm::mat4 *mv, glm::mat4 *proj,
                                  int *vp,
                                  float *winx, float *winy, float *winz )
{
    glm::vec4 vin( objx, objy, objz, 1.0f ), vout;
    Vec4MultiplyMat4( &vout, &vin,  mv   );
    Vec4MultiplyMat4( &vin,  &vout, proj );
    if ( !vin.w ) return 0;
    vin /= vin.w;
    *winx = (vin.x * 0.5f + 0.5f) * vp[2] + vp[0];
    *winy = (vin.y * 0.5f + 0.5f) * vp[3] + vp[1];
    *winz =  vin.z * 0.5f + 0.5f;
    return 1;
}

int Transform::TransformUnproject( float winx, float winy, float winz,
                                    glm::mat4 *mv, glm::mat4 *proj,
                                    int *vp,
                                    float *objx, float *objy, float *objz )
{
    glm::mat4 final  = glm::inverse( *proj * *mv );
    glm::vec4 vin, vout;
    vin.x = (winx - vp[0]) / vp[2] * 2.0f - 1.0f;
    vin.y = (winy - vp[1]) / vp[3] * 2.0f - 1.0f;
    vin.z =  winz * 2.0f - 1.0f;
    vin.w = 1.0f;
    Vec4MultiplyMat4( &vout, &vin, &final );
    if ( !vout.w ) return 0;
    vout /= vout.w;
    *objx = vout.x;  *objy = vout.y;  *objz = vout.z;
    return 1;
}
