package com.example.fan3d;

import androidx.appcompat.app.AppCompatActivity;

import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * MainActivity - Assignment 2 "3D Fan", Android host.
 *
 * Hosts a GLSurfaceView backed by the shared Scene/ C++ engine (Renderer +
 * Fan) via JNI (see NativeTemplate.cpp). Touch events are forwarded to the
 * native renderer through glSurfaceView.queueEvent(...) so every GL call
 * happens on the GLSurfaceView render thread, matching Chapter 2 tutorial 4.
 *
 * Gestures:
 *   - Tap        -> toggles the fan ON/OFF (see Scene/Fan.cpp)
 *   - Drag/swipe -> spins the fan faster proportionally to swipe speed
 */
public class MainActivity extends AppCompatActivity {

    private static final String TAG = "Fan3D";

    static {
        System.loadLibrary("fan3d");
    }

    private GLSurfaceView glSurfaceView;
    private boolean rendererInitialized = false;

    // ---------------------------------------------------------------------------
    // JNI declarations (implemented in NativeTemplate.cpp)
    // ---------------------------------------------------------------------------
    private native boolean nativeInit(AssetManager assetManager);
    private native void    nativeResize(int width, int height);
    private native void    nativeRender();
    private native void    nativeTouchDown(float x, float y);
    private native void    nativeTouchMove(float x, float y);
    private native void    nativeTouchRelease(float x, float y);

    // ---------------------------------------------------------------------------
    // Activity lifecycle
    // ---------------------------------------------------------------------------
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Keep screen on while the demo is running
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        setContentView(R.layout.activity_main);

        glSurfaceView = findViewById(R.id.gl_surface_view);

        // Request OpenGL ES 3.0 context
        glSurfaceView.setEGLContextClientVersion(3);
        glSurfaceView.setRenderer(new GLSurfaceView.Renderer() {

            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                Log.d(TAG, "onSurfaceCreated");
                boolean ok = nativeInit(getAssets());
                if (ok) {
                    rendererInitialized = true;
                    Log.d(TAG, "Native renderer initialized successfully");
                } else {
                    Log.e(TAG, "Native renderer initialization failed");
                }
            }

            @Override
            public void onSurfaceChanged(GL10 gl, int width, int height) {
                Log.d(TAG, "onSurfaceChanged: " + width + "x" + height);
                if (rendererInitialized) {
                    nativeResize(width, height);
                }
            }

            @Override
            public void onDrawFrame(GL10 gl) {
                if (rendererInitialized) {
                    nativeRender();
                }
            }
        });

        // Render continuously so the blades keep spinning
        glSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);

        // Handle touch events - must be set on the view itself
        glSurfaceView.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (!rendererInitialized) return true;

                final float x = event.getX();
                final float y = event.getY();

                switch (event.getActionMasked()) {
                    case MotionEvent.ACTION_DOWN:
                        // Queue on GL thread to avoid race conditions
                        glSurfaceView.queueEvent(() -> nativeTouchDown(x, y));
                        break;
                    case MotionEvent.ACTION_MOVE:
                        glSurfaceView.queueEvent(() -> nativeTouchMove(x, y));
                        break;
                    case MotionEvent.ACTION_UP:
                    case MotionEvent.ACTION_CANCEL:
                        glSurfaceView.queueEvent(() -> nativeTouchRelease(x, y));
                        break;
                }
                return true;
            }
        });
    }

    @Override
    protected void onPause() {
        super.onPause();
        glSurfaceView.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        glSurfaceView.onResume();
    }
}
