package constantin.video.core.VideoNative;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.AssetManager;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.view.Surface;

import java.io.File;

import constantin.video.core.R;

public class VideoNative {
    static {
        System.loadLibrary("VideoNative");
    }
    public static native  <T extends NativeInterfaceVideoParamsChanged> long initialize(T t, Context context, String groundRecordingDirectory);
    public static native void finalize(long nativeVideoPlayer);
    //Consumers are currently
    //1) The LowLag decoder (if Surface!=null)
    //2) The GroundRecorder (if enableGroundRecording=true)
    public static native void nativeAddConsumers(long nativeInstance, Surface surface);
    public static native void nativeRemoveConsumers(long videoPlayerN);

    public static native void nativePassNALUData(long nativeInstance,byte[] b,int offset,int size);
    public static native void nativeStartReceiver(long nativeInstance, AssetManager assetManager);
    public static native void nativeStopReceiver(long nativeInstance);

    /**
     * Debugging/ Testing only
     */
    public static native String getVideoInfoString(long nativeInstance);
    public static native boolean anyVideoDataReceived(long nativeInstance);
    public static native boolean anyVideoBytesParsedSinceLastCall(long nativeInstance);
    public static native boolean receivingVideoButCannotParse(long nativeInstance);

    public interface NativeInterfaceVideoParamsChanged {
        @SuppressWarnings("unused")
        void onVideoRatioChanged(int videoW, int videoH);
        @SuppressWarnings("unused")
        void onDecodingInfoChanged(float currentFPS, float currentKiloBitsPerSecond, float avgParsingTime_ms, float avgWaitForInputBTime_ms, float avgDecodingTime_ms,
                                   int nNALU, int nNALUSFeeded);
    }

    public static final int SOURCE_TYPE_UDP=0;
    public static final int SOURCE_TYPE_FILE=1;
    public static final int SOURCE_TYPE_ASSETS=2;
    public static final int SOURCE_TYPE_EXTERNAL=3;

    public static boolean PLAYBACK_FLE_EXISTS(final Context context){
        SharedPreferences sharedPreferences=context.getSharedPreferences("pref_video",Context.MODE_PRIVATE);
        final String filename=sharedPreferences.getString(context.getString(R.string.VS_PLAYBACK_FILENAME),"");
        File tempFile = new File(filename);
        boolean exists = tempFile.exists();
        return exists;
    }


    private static String getDirectory(){
        return Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM)+"/FPV_VR/";
    }


    @SuppressLint("ApplySharedPref")
    public static void initializePreferences(final Context context){
        PreferenceManager.setDefaultValues(context,R.xml.pref_video,false);
        final SharedPreferences pref_video=context.getSharedPreferences("pref_video",Context.MODE_PRIVATE);
        final String filename=pref_video.getString(context.getString(R.string.VS_PLAYBACK_FILENAME),context.getString(R.string.VS_PLAYBACK_FILENAME_DEFAULT_VALUE));
        if(filename.equals(context.getString(R.string.VS_PLAYBACK_FILENAME_DEFAULT_VALUE))){
            pref_video.edit().putString(context.getString(R.string.VS_PLAYBACK_FILENAME),
                    getDirectory()+"Video/"+"filename.h264").commit();
        }
    }
}
