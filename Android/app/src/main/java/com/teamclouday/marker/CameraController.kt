package com.teamclouday.marker

import android.Manifest
import android.content.Context
import android.content.pm.PackageManager
import android.hardware.camera2.CameraAccessException
import android.hardware.camera2.CameraCharacteristics
import android.hardware.camera2.CameraDevice
import android.hardware.camera2.CameraManager
import android.os.Handler
import android.os.HandlerThread
import android.os.Process
import android.util.Log
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat

// refer to: https://developer.android.com/guide/topics/media/camera
// refer to: https://developer.android.com/training/camera/cameradirect
// refer to: https://developer.android.com/training/camera2
class CameraController(val mActivity: MainActivity) {
    private val mLogTag : String = "Marker@CameraController"

    private val CAMERA_REQUEST_CODE = 0
    private val mName : String
    private val mManager : CameraManager
    private var mDevice : CameraDevice? = null

    // camera state callback
    private val mStateCallback = object : CameraDevice.StateCallback(){
        override fun onOpened(camera: CameraDevice) {
            Log.d(mLogTag, "[mStateCallback] Camera opened")
            mDevice = camera
            displayNotification(true)
        }

        override fun onError(camera: CameraDevice, error: Int) {
            Log.d(mLogTag, "[mStateCallback] Camera error (code = $error)")
            closeCamera()
        }

        override fun onDisconnected(camera: CameraDevice) {
            Log.d(mLogTag, "[mStateCallback] Camera disconnected")
            closeCamera()

        }
    }
    private val mStateCallbackHandler = HandlerThread("CameraControllerCallback", Process.THREAD_PRIORITY_FOREGROUND)

    init {
        // check if camera hardware exists
        require(mActivity.packageManager.hasSystemFeature(PackageManager.FEATURE_CAMERA_ANY)){
            "Camera is not detected on this device"
        }
        // get camera manager and target camera id
        mManager = mActivity.getSystemService(Context.CAMERA_SERVICE) as CameraManager
        mName = getBackCamera()
        require(mName.isNotBlank()){
            "No valid camera found (back or front)"
        }
        // start thread
        mStateCallbackHandler.start()
    }

    // open camera
    fun openCamera(){
        require(mStateCallbackHandler.isAlive){
            "Camera is destroyed already"
        }
        // request camera access permission
        if(ContextCompat.checkSelfPermission(mActivity, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED){
            ActivityCompat.requestPermissions(mActivity, arrayOf(Manifest.permission.CAMERA), CAMERA_REQUEST_CODE)
        }
        require(ContextCompat.checkSelfPermission(mActivity, Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED){
            "Camera access not granted"
        }

        try {
            mManager.openCamera(mName, mStateCallback, Handler(mStateCallbackHandler.looper))
        } catch (e : CameraAccessException){
            Log.d(mLogTag, "[openCamera] Failed to open camera: ${e.reason}")
            closeCamera()
        }

    }

    // close camera
    fun closeCamera(){
        require(mStateCallbackHandler.isAlive){
            "Camera is destroyed already"
        }
        // close device
        mDevice?.close()
        mDevice = null
        displayNotification(false)
    }

    // clean all
    fun clean(){
        closeCamera()
        mStateCallbackHandler.quitSafely()
    }

    // display or close a notification for camera usage
    private fun displayNotification(showUp : Boolean){
        if(showUp){
            mActivity.runOnUiThread{
                mActivity.showNotification()
            }
        }
        else{
            mActivity.runOnUiThread{
                mActivity.removeNotification()
            }
        }
    }

    // retrieve back camera, if not found, try front camera
    private fun getBackCamera() : String{
        // get available camera names
        val cameraIds = try{
            mManager.cameraIdList
        }catch(e : CameraAccessException){
            Log.d(mLogTag, "[getBackCamera] Camera list access violation")
            null
        } ?: return ""
        if(cameraIds.isEmpty()) return ""
        // set default target
        var target = ""
        for (id in cameraIds){
            Log.d(mLogTag, "[getBackCamera] Checking $id")
            val characteristic = try{
                mManager.getCameraCharacteristics(id)
            }catch(e : CameraAccessException){
                Log.d(mLogTag, "[getBackCamera] $id access violation")
                null
            } ?: continue
            if(characteristic.get(CameraCharacteristics.LENS_FACING) == CameraCharacteristics.LENS_FACING_BACK){
                target = id
                break
            }
            else if((characteristic.get(CameraCharacteristics.LENS_FACING) == CameraCharacteristics.LENS_FACING_FRONT) &&
                    target.isBlank()){
                target = id
            }
        }
        Log.d(mLogTag, "[getBackCamera] Selected $target")
        return target
    }
}