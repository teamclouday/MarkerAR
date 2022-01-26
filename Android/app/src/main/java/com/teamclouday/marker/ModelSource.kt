package com.teamclouday.marker

import android.opengl.GLES20
import android.opengl.Matrix
import android.util.Log
import java.nio.ByteBuffer
import java.nio.ByteOrder

// refer to: https://developer.android.com/training/graphics/opengl/shapes
class ModelCube {
    private val mLogTag : String = "Marker@ModelCube"

    // raw cube data
    private val mVertexData = floatArrayOf(
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.583f, 0.771f, 0.014f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.609f, 0.115f, 0.436f,
        0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.327f, 0.483f, 0.844f,
        0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.822f, 0.569f, 0.201f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.435f, 0.602f, 0.223f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.310f, 0.747f, 0.185f,

        -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.597f, 0.770f, 0.761f,
        0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.559f, 0.436f, 0.730f,
        0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.359f, 0.583f, 0.152f,
        0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.483f, 0.596f, 0.789f,
        -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.559f, 0.861f, 0.639f,
        -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.195f, 0.548f, 0.859f,

        -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f, 0.014f, 0.184f, 0.576f,
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 0.771f, 0.328f, 0.970f,
        -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 0.406f, 0.615f, 0.116f,
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 0.676f, 0.977f, 0.133f,
        -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f, 0.971f, 0.572f, 0.833f,
        -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f, 0.140f, 0.616f, 0.489f,

        0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.997f, 0.513f, 0.064f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.945f, 0.719f, 0.592f,
        0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.543f, 0.021f, 0.978f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.279f, 0.317f, 0.505f,
        0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.167f, 0.620f, 0.077f,
        0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.347f, 0.857f, 0.137f,

        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.055f, 0.953f, 0.042f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.714f, 0.505f, 0.345f,
        0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 0.783f, 0.290f, 0.734f,
        0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 0.722f, 0.645f, 0.174f,
        -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 0.302f, 0.455f, 0.848f,
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.225f, 0.587f, 0.040f,

        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.517f, 0.713f, 0.338f,
        0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.053f, 0.959f, 0.120f,
        0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.393f, 0.621f, 0.362f,
        0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.673f, 0.211f, 0.457f,
        -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.820f, 0.883f, 0.371f,
        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.982f, 0.099f, 0.879f
    )
    // allocated buffer
    private val mVertexBuffer = ByteBuffer.allocateDirect(mVertexData.size * Float.SIZE_BYTES).run {
        order(ByteOrder.nativeOrder())
        asFloatBuffer().apply {
            put(mVertexData)
            position(0)
        }
    }
    // shader program
    private val mProgram : Int
    // VBO
    private val mVBO = intArrayOf(0)
    // vertex input locations
    private val mVertexPos : Int
    private val mVertexNorm : Int
    private val mVertexColor : Int
    private val mHandleMVP : Int
    // vertex counts
    private val mCount : Int
    // model transformation matrix
    private val mMatrix = FloatArray(16)
    private val mMVP = FloatArray(16)

    init {
        // generate buffer
        GLES20.glGenBuffers(1, mVBO, 0)
        GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER, mVBO[0])
        GLES20.glBufferData(GLES20.GL_ARRAY_BUFFER, mVertexData.size * Float.SIZE_BYTES, mVertexBuffer, GLES20.GL_STATIC_DRAW)
        GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER, 0)
        // generate shader and compile program
        val vertexShader = ShaderCode.loadShader(GLES20.GL_VERTEX_SHADER, ShaderCode.modelCubeVertexShaderCode)
        val fragmentShader = ShaderCode.loadShader(GLES20.GL_FRAGMENT_SHADER, ShaderCode.modelCubeFragmentShaderCode)
        mProgram = GLES20.glCreateProgram().also {
            GLES20.glAttachShader(it, vertexShader)
            GLES20.glAttachShader(it, fragmentShader)
            GLES20.glLinkProgram(it)
            val linkStatus = IntArray(1)
            GLES20.glGetProgramiv(it, GLES20.GL_LINK_STATUS, linkStatus, 0)
            if(linkStatus[0] != GLES20.GL_TRUE){
                Log.d(mLogTag, "[init] Failed to link program (${GLES20.glGetProgramInfoLog(it)})")
            }
        }
        // get vertex positions
        mVertexPos = GLES20.glGetAttribLocation(mProgram, "inPosition")
        mVertexNorm = GLES20.glGetAttribLocation(mProgram, "inNorm")
        mVertexColor = GLES20.glGetAttribLocation(mProgram, "inColor")
        mHandleMVP = GLES20.glGetUniformLocation(mProgram, "matMVP")
        // set vertex count
        mCount = mVertexData.size / 9
        // set model matrix
        Matrix.setIdentityM(mMatrix, 0)
        Matrix.scaleM(mMatrix, 0, 0.5f, 0.5f, 0.5f)
    }

    fun render(matrixVP : FloatArray){
        GLES20.glUseProgram(mProgram)
        // set VBO positions
        GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER, mVBO[0])
        GLES20.glEnableVertexAttribArray(mVertexPos)
        GLES20.glVertexAttribPointer(mVertexPos, 3, GLES20.GL_FLOAT, false, 9*Float.SIZE_BYTES, 0)
        GLES20.glEnableVertexAttribArray(mVertexNorm)
        GLES20.glVertexAttribPointer(mVertexNorm, 3, GLES20.GL_FLOAT, false, 9*Float.SIZE_BYTES, 3*Float.SIZE_BYTES)
        GLES20.glEnableVertexAttribArray(mVertexColor)
        GLES20.glVertexAttribPointer(mVertexColor, 3, GLES20.GL_FLOAT, false, 9*Float.SIZE_BYTES, 6*Float.SIZE_BYTES)
        GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER, 0)
        // set transformation
        Matrix.multiplyMM(mMVP, 0, matrixVP, 0, mMatrix, 0)
        GLES20.glUniformMatrix4fv(mHandleMVP, 1, false, mMVP, 0)
        // render
        GLES20.glDrawArrays(GLES20.GL_TRIANGLES, 0, mCount)
        GLES20.glUseProgram(0)
    }
}