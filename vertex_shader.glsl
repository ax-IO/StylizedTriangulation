attribute highp vec4 vertex;
attribute highp vec4 qt_MultiTexCoord0;

uniform highp mat4 ModelViewProjectionMatrix;
varying highp vec4 qt_TexCoord0;

void main(void)
{
    gl_Position = ModelViewProjectionMatrix * vertex;
    qt_TexCoord0 = qt_MultiTexCoord0;
}
