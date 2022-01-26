package com.teamclouday.marker

import android.graphics.SurfaceTexture
import android.opengl.GLSurfaceView
import android.os.SystemClock
import android.util.Log
import android.view.TextureView
import java.lang.Exception
import java.lang.IllegalArgumentException
import javax.microedition.khronos.egl.*
import javax.microedition.khronos.opengles.GL10


// refer to: https://gist.github.com/ilya-t/c54bd715edd495c07677
// render OpenGL context to a TextureView
class ContextManager(val mActivity : MainActivity) : TextureView(mActivity), TextureView.SurfaceTextureListener{
    private val mLogTag : String = "Marker@ContextManager"

    private lateinit var mThread : GLThread

    inner class GLThread(val surface : SurfaceTexture, var width: Int, var height: Int) : Thread(){
        private val mLogTag : String = "Marker@ContextThread"

        private lateinit var mMotionCamera : MotionCamera
        private lateinit var mRenderer : GLSurfaceView.Renderer

        private lateinit var mDisplay : EGLDisplay
        private lateinit var mConfig : EGLConfig
        private lateinit var mContext : EGLContext
        private var mSurface : EGLSurface? = null
        private val egl = EGLContext.getEGL() as EGL10
        private lateinit var gl : GL10
        // refer to: https://www.khronos.org/registry/EGL/api/EGL/egl.h
        private val EGL_OPENGL_ES2_BIT : Int = "00000004".toInt(16)
        private val EGL_CONTEXT_CLIENT_VERSION : Int = "00003098".toInt(16)

        private val FPS : Float = 30.0f
        private val SPF : Float = 1000.0f / FPS // in milliseconds

        @Volatile private var stopRender = false
        @Volatile private var windowSizeChanged = false

        private fun init() {
            // get display
            mDisplay = egl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY)
            require(mDisplay != EGL10.EGL_NO_DISPLAY){
                "No default display for OpenGL context"
            }
            Log.d(mLogTag, "[init] OpenGL display selected")
            // initialize version
            val version = IntArray(2)
            require(egl.eglInitialize(mDisplay, version)){
                "Cannot initialize OpenGL 2.0"
            }
            Log.d(mLogTag, "[init] OpenGL version: (${version[0]}, ${version[1]})")
            // choose config
            val configSpec = intArrayOf(
                EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL10.EGL_RED_SIZE, 8,
                EGL10.EGL_GREEN_SIZE, 8,
                EGL10.EGL_BLUE_SIZE, 8,
                EGL10.EGL_ALPHA_SIZE, 8,
                EGL10.EGL_DEPTH_SIZE, 16,
                EGL10.EGL_NONE
            )
            val configs = arrayOfNulls<EGLConfig>(1)
            val configsNum = IntArray(1)
            require(egl.eglChooseConfig(mDisplay, configSpec, configs, 1, configsNum)){
                "Cannot choose OpenGL config (0x${egl.eglGetError().toString(16)})"
            }
            require(configsNum[0] > 0 && configs[0] != null){
                "Cannot choose OpenGL config (0x${egl.eglGetError().toString(16)})"
            }
            mConfig = configs[0]!!
            Log.d(mLogTag, "[init] OpenGL config selected")
            // create context
            val contextAttrib = intArrayOf(
                EGL_CONTEXT_CLIENT_VERSION, 2,
                EGL10.EGL_NONE
            )
            mContext = egl.eglCreateContext(mDisplay, mConfig, EGL10.EGL_NO_CONTEXT, contextAttrib)
            Log.d(mLogTag, "[init] OpenGL context created")
            // create surface
            createSurface()
            // set gl
            gl = mContext.gl as GL10
            // init renderer and virtual camera
            mMotionCamera = MotionCamera(mActivity)
            mMotionCamera.start()
            mRenderer = ModelRenderer(mMotionCamera)
        }

        override fun run() {
            init()
            mRenderer.onSurfaceCreated(gl, mConfig)
            mRenderer.onSurfaceChanged(gl, width, height)
            var timePrev: Long = SystemClock.elapsedRealtime()
            var timeCurr: Long
            while(!stopRender){
                // check if context is valid
                if(mContext != egl.eglGetCurrentContext() ||
                        mSurface != egl.eglGetCurrentSurface(EGL10.EGL_DRAW)){
                    if(!egl.eglMakeCurrent(mDisplay, mSurface, mSurface, mContext))
                        Log.d(mLogTag, "[run] eglMakeCurrent failed (0x${egl.eglGetError().toString(16)})")
                }
                // check for window size event
                if(windowSizeChanged){
                    createSurface()
                    mRenderer.onSurfaceChanged(gl, width, height)
                    windowSizeChanged = false
                }
                // render frame
                mRenderer.onDrawFrame(gl)
                // swap buffers
                if(!egl.eglSwapBuffers(mDisplay, mSurface))
                    Log.d(mLogTag, "[run] eglSwapBuffers failed (0x${egl.eglGetError().toString(16)})")
                // FPS control
                timeCurr = SystemClock.elapsedRealtime()
                if(timeCurr - timePrev < SPF){
                    sleep((SPF - (timeCurr - timePrev)).toLong())
                }
                timePrev = timeCurr
            }
            // clean and destroy OpenGL context
            clean()
        }

        private fun createSurface(){
            destroySurface()
            try{
                mSurface = egl.eglCreateWindowSurface(mDisplay, mConfig, surface, null)
            }catch(e : IllegalArgumentException){
                Log.d(mLogTag, "[createSurface] failed to create window surface")
            }
            if(!egl.eglMakeCurrent(mDisplay, mSurface, mSurface, mContext))
                Log.d(mLogTag, "[createSurface] eglMakeCurrent failed (0x${egl.eglGetError().toString(16)})")
            Log.d(mLogTag, "[createSurface] surface created ${mSurface != null}")
        }

        private fun destroySurface(){
            if(mSurface != null){
                egl.eglMakeCurrent(mDisplay, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT)
                egl.eglDestroySurface(mDisplay, mSurface)
                mSurface = null
                Log.d(mLogTag, "[destroySurface] surface destroyed")
            }
        }

        private fun clean(){
            destroySurface()
            egl.eglDestroyContext(mDisplay, mContext)
            egl.eglTerminate(mDisplay)
            mMotionCamera.stop()
        }

        @Synchronized fun stopThread(){
            stopRender = true
        }

        @Synchronized fun onWindowResize(w : Int, h : Int){
            width = w
            height = h
            windowSizeChanged = true
        }
    }

    override fun onSurfaceTextureAvailable(surface: SurfaceTexture, width: Int, height: Int) {
        try{
            mThread = GLThread(surface, getWidth(), getHeight())
            mThread.start()
        }catch (e : Exception){
            Log.d(mLogTag, "[onSurfaceTextureAvailable] ${e.message}")
        }
        Log.d(mLogTag, "[onSurfaceTextureAvailable] ($width, $height)")
    }

    override fun onSurfaceTextureDestroyed(surface: SurfaceTexture): Boolean {
        mThread.stopThread()
        mThread.join()
        Log.d(mLogTag, "[onSurfaceTextureDestroyed]")
        return true
    }

    override fun onSurfaceTextureSizeChanged(surface: SurfaceTexture, width: Int, height: Int) {
        mThread.onWindowResize(width, height)
        Log.d(mLogTag, "[onSurfaceTextureSizeChanged] ($width, $height)")
    }

    override fun onSurfaceTextureUpdated(surface: SurfaceTexture) {
//        Log.d(mLogTag, "[onSurfaceTextureUpdated]")
    }
}