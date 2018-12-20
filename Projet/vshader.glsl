#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform mat4 m_matrix;
uniform mat4 v_matrix;
uniform mat4 p_matrix;

uniform vec4 a_color;

attribute vec4 a_position;
attribute vec2 a_texcoord;

varying vec2 v_texcoord;
varying vec4 v_color;

//! [0]
void main()
{
    // Calculate vertex position in screen space
    gl_Position = p_matrix * v_matrix * m_matrix * a_position;

    // Pass texture coordinate to fragment shader
    // Value will be automatically interpolated to fragments inside polygon faces
    v_texcoord = a_texcoord;
    v_color = a_color;
}
//! [0]
