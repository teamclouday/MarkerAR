package com.teamclouday.marker

import android.content.Context
import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.opengl.Matrix
import android.util.Log
import kotlin.math.cos
import kotlin.math.sin
import kotlin.math.sqrt

// this is a virtual camera
// that replies on phone motion for view angles
// refer to: https://developer.android.com/guide/topics/sensors/sensors_motion#sensors-motion-gyro
class MotionCamera(val mActivity : MainActivity) : SensorEventListener {
    private val mLogTag : String = "Marker@MotionCamera"

    private val mCameraDist = 2.0f
    @Volatile private var mCameraPos = FloatArray(4)

    private val NS2S = 1.0f / 1000000000.0f
    private var mTimestamp = 0.0f

    private val mMatrixView = FloatArray(16)
    private val mDeltaRotationM = FloatArray(16)
    private val mDeltaRotationV = FloatArray(4)

    private val mSensorManager : SensorManager

    init {
        // prepare camera position
        mCameraPos[0] = 0.0f
        mCameraPos[1] = 0.0f
        mCameraPos[2] = 1.0f
        mCameraPos[3] = 0.0f
        Vector.normalize(mCameraPos, 3)
        // register sensor
        mSensorManager = mActivity.getSystemService(Context.SENSOR_SERVICE) as SensorManager
    }

    fun start(){
        mSensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE)?.also {
            mSensorManager.registerListener(
                this,
                it,
                SensorManager.SENSOR_DELAY_GAME
            )
        }
    }

    fun stop(){
        mSensorManager.unregisterListener(this)
    }

    @Synchronized fun view() : FloatArray
    {
        Matrix.setLookAtM(mMatrixView, 0,
            mCameraPos[0] * mCameraDist, mCameraPos[1] * mCameraDist, mCameraPos[2] * mCameraDist,
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f)
        return mMatrixView
    }

    override fun onAccuracyChanged(sensor: Sensor?, accuracy: Int) {

    }

    @Synchronized override fun onSensorChanged(event: SensorEvent?) {
        if(mTimestamp > 0.0f && event != null){
            val dT = (event.timestamp - mTimestamp) * NS2S
            var axisX: Float = -event.values[0]
            var axisY: Float = -event.values[1]
            var axisZ = 0.0f
            // get angular speed
            val omegaMagnitude: Float = sqrt(axisX * axisX + axisY * axisY + axisZ * axisZ)
            // normalize
            if(omegaMagnitude > 1e-8){
                axisX /= omegaMagnitude
                axisY /= omegaMagnitude
                axisZ /= omegaMagnitude
            }
            val thetaOverTwo: Float = omegaMagnitude * dT / 2.0f
            val sinThetaOverTwo: Float = sin(thetaOverTwo)
            val cosThetaOverTwo: Float = cos(thetaOverTwo)
            mDeltaRotationV[0] = sinThetaOverTwo * axisX
            mDeltaRotationV[1] = sinThetaOverTwo * axisY
            mDeltaRotationV[2] = sinThetaOverTwo * axisZ
            mDeltaRotationV[3] = cosThetaOverTwo

            SensorManager.getRotationMatrixFromVector(mDeltaRotationM, mDeltaRotationV)
            Matrix.multiplyMV(mCameraPos, 0, mDeltaRotationM, 0, mCameraPos, 0)
            Vector.normalize(mCameraPos, 3)
        }
        mTimestamp = event?.timestamp?.toFloat() ?: 0.0f
    }


}