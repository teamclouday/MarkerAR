package com.teamclouday.marker

import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.content.pm.ActivityInfo
import android.opengl.GLSurfaceView
import android.os.Build
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.WindowManager
import androidx.core.app.NotificationCompat
import androidx.core.app.NotificationManagerCompat

// helper function to ignore some exceptions
inline fun ignore(body: () -> Unit) {
    try {
        body()
    }catch (e : Exception){}
}

class MainActivity : AppCompatActivity() {
    private val mLogTag : String = "Marker@MainActivity"

    private lateinit var mTextureView : ContextManager

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        mTextureView = ContextManager(this)
        mTextureView.surfaceTextureListener = mTextureView
        setContentView(mTextureView)
        // set up notification
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
        {
            val name = "Marker"
            val descriptionText = "Marker app is using camera"
            val importance = NotificationManager.IMPORTANCE_DEFAULT
            val channel = NotificationChannel("Marker", name, importance).apply {
                description = descriptionText
            }
            // Register the channel with the system
            val notificationManager: NotificationManager =
                getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
            notificationManager.createNotificationChannel(channel)
        }
        // keep screen on for this app
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        // lock orientation to portrait only
        requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_PORTRAIT
    }

    // display notification
    fun showNotification(){
        val onTap = Intent(this, MainActivity::class.java).apply {
            addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP or Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_BROUGHT_TO_FRONT)
        }
        val pendingIntent = PendingIntent.getActivity(this, 0, onTap, 0)
        val builder = NotificationCompat.Builder(this, "Marker")
            .setSmallIcon(R.mipmap.ic_launcher)
            .setContentTitle("Marker")
            .setContentText("Camera is in use")
            .setPriority(NotificationCompat.PRIORITY_DEFAULT)
            .setContentIntent(pendingIntent)
            .setOngoing(true)
        with(NotificationManagerCompat.from(this)) {
            notify(0, builder.build())
        }
    }

    // remove notification
    fun removeNotification(){
        with(NotificationManagerCompat.from(this)) {
            cancel(0)
        }
    }
}