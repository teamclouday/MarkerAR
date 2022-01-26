package com.teamclouday.marker

import android.opengl.GLES20
import android.util.Log

class ShaderCode{

    companion object {
        val mLogTag : String = "Marker@Shader"
        val modelCubeVertexShaderCode =
            """
            attribute vec3 inPosition;
            attribute vec3 inNorm;
            attribute vec3 inColor;
            varying vec3 vertColor;
            uniform mat4 matMVP;
            void main() {
//                vertColor = inColor;
                vertColor = inNorm * 0.5 + 0.5;
                gl_Position = matMVP * vec4(inPosition, 1.0);
            }
            """.trimIndent()
        val modelCubeFragmentShaderCode =
            """
            precision mediump float;
            varying vec3 vertColor;
            void main() {
                gl_FragColor = vec4(vertColor, 1.0);
            }
            """.trimIndent()

        // refer to: https://developer.android.com/training/graphics/opengl/draw
        fun loadShader(type : Int, shaderCode : String) : Int{
            return GLES20.glCreateShader(type).also {
                GLES20.glShaderSource(it, shaderCode)
                GLES20.glCompileShader(it)
                val compiledStatus = IntArray(1)
                GLES20.glGetShaderiv(it, GLES20.GL_COMPILE_STATUS, compiledStatus, 0)
                if(compiledStatus[0] != GLES20.GL_TRUE){
                    Log.d(mLogTag, "[loadShader] Failed to compile shader (${GLES20.glGetShaderInfoLog(it)})\n$shaderCode")
                }
            }
        }
    }
}