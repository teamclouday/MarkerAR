package com.teamclouday.marker

import android.opengl.GLES20
import android.opengl.GLSurfaceView
import android.opengl.Matrix
import android.util.Log
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class ModelRenderer(private val mMotionCamera : MotionCamera) : GLSurfaceView.Renderer {
    private val mLogTag : String = "Marker@ModelRenderer"

    private var mWidth : Int = 0
    private var mHeight : Int = 0
    private var mRatio : Float = 0.0f

    private val mMatrixProj = FloatArray(16)
    private val mMatrixVP = FloatArray(16)

    private val mCube = ModelCube()

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 1.0f)
        GLES20.glEnable(GLES20.GL_DEPTH_TEST)
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
        GLES20.glViewport(0, 0, width, height)
        mWidth = width
        mHeight = height
        mRatio = mWidth.toFloat() / mHeight.toFloat()
        Matrix.perspectiveM(mMatrixProj, 0, 60.0f, mRatio, 0.1f, 1000.0f)
    }

    override fun onDrawFrame(gl: GL10?) {
        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 1.0f)
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT or GLES20.GL_DEPTH_BUFFER_BIT)
        Matrix.multiplyMM(mMatrixVP, 0, mMatrixProj, 0, mMotionCamera.view(), 0)
        mCube.render(mMatrixVP)
        GLES20.glFlush()
    }
}