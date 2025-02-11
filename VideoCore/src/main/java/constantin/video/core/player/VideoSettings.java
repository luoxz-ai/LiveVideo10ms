package constantin.video.core.player;

import android.annotation.SuppressLint;
import android.content.ContentValues;
import android.content.Context;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Environment;
import android.provider.MediaStore;
import android.util.Log;
import android.widget.Toast;

import androidx.preference.PreferenceManager;

import java.io.File;

import constantin.video.core.R;

import static android.content.Context.MODE_PRIVATE;

//Provides conv
public class VideoSettings {
    private static final String TAG=VideoSettings.class.getSimpleName();
    public enum VS_SOURCE{UDP,FILE,ASSETS,FFMPEG,EXTERNAL}
    public static final int VIDEO_MODE_2D_MONOSCOPIC=0;

    public static boolean PLAYBACK_FLE_EXISTS(final Context context){
        SharedPreferences sharedPreferences=context.getSharedPreferences("pref_video", MODE_PRIVATE);
        final String filename=sharedPreferences.getString(context.getString(R.string.VS_PLAYBACK_FILENAME),"");
        File tempFile = new File(filename);
        return tempFile.exists();
    }

    public static VS_SOURCE getVS_SOURCE(final Context context){
        SharedPreferences sharedPreferences=context.getSharedPreferences("pref_video", MODE_PRIVATE);
        final int val=sharedPreferences.getInt(context.getString(R.string.VS_SOURCE),0);
        VS_SOURCE ret= VS_SOURCE.values()[val];
        sharedPreferences.getInt(context.getString(R.string.VS_SOURCE),0);
        return ret;
    }

    @SuppressLint("ApplySharedPref")
    public static void setVS_SOURCE(final Context context, final VS_SOURCE val){
        SharedPreferences sharedPreferences=context.getSharedPreferences("pref_video", MODE_PRIVATE);
        sharedPreferences.edit().putInt(context.getString(R.string.VS_SOURCE),val.ordinal()).commit();
    }

    @SuppressLint("ApplySharedPref")
    public static void setVS_ASSETS_FILENAME_TEST_ONLY(final Context context, final String filename){
        SharedPreferences sharedPreferences=context.getSharedPreferences("pref_video", MODE_PRIVATE);
        sharedPreferences.edit().putString(context.getString(R.string.VS_ASSETS_FILENAME_TEST_ONLY),filename).commit();
    }
    @SuppressLint("ApplySharedPref")
    public static void setVS_FILE_ONLY_LIMIT_FPS(final Context context, final int limitFPS){
        SharedPreferences sharedPreferences=context.getSharedPreferences("pref_video", MODE_PRIVATE);
        sharedPreferences.edit().putInt(context.getString(R.string.VS_FILE_ONLY_LIMIT_FPS),limitFPS).commit();
    }

    public static String getVS_PLAYBACK_FILENAME(final Context context){
        final String tmp=context.getSharedPreferences("pref_video",Context.MODE_PRIVATE).
                getString(context.getString(R.string.VS_PLAYBACK_FILENAME),context.getString(R.string.VS_PLAYBACK_FILENAME_DEFAULT_VALUE));
        return tmp;
    }

    @SuppressLint("ApplySharedPref")
    public static void setVS_PLAYBACK_FILENAME(final Context context, final String pathAndFilename){
        context.getSharedPreferences("pref_video",Context.MODE_PRIVATE).edit().
                putString(context.getString(R.string.VS_PLAYBACK_FILENAME),pathAndFilename).commit();
    }

    @SuppressLint("ApplySharedPref")
    public static void setVS_GROUND_RECORDING(final Context context,final boolean enable){
        context.getSharedPreferences("pref_video",Context.MODE_PRIVATE).edit().
                putBoolean(context.getString(R.string.VS_GROUND_RECORDING),enable).commit();
    }
    public static boolean getVS_GROUND_RECORDING(final Context context){
        return context.getSharedPreferences("pref_video",Context.MODE_PRIVATE).
                getBoolean(context.getString(R.string.VS_GROUND_RECORDING),false);
    }


    //0=normal
    //1=stereo
    //2=equirectangular 360 sphere
    public static int videoMode(final Context c){
        final SharedPreferences pref_video=c.getSharedPreferences("pref_video",MODE_PRIVATE);
        return pref_video.getInt(c.getString(R.string.VS_VIDEO_VIEW_TYPE),0);
    }

    @SuppressLint("ApplySharedPref")
    public static void initializePreferences(final Context context,final boolean readAgain){
        PreferenceManager.setDefaultValues(context,"pref_video",MODE_PRIVATE,R.xml.pref_video,readAgain);
        final SharedPreferences pref_video=context.getSharedPreferences("pref_video", MODE_PRIVATE);
        final String filename=pref_video.getString(context.getString(R.string.VS_PLAYBACK_FILENAME),context.getString(R.string.VS_PLAYBACK_FILENAME_DEFAULT_VALUE));
        if(filename.equals(context.getString(R.string.VS_PLAYBACK_FILENAME_DEFAULT_VALUE))){
            pref_video.edit().putString(context.getString(R.string.VS_PLAYBACK_FILENAME),
                    Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_MOVIES)+"/FPV_VR/"+"filename.fpv").commit();
        }
    }

    public static String getDirectoryToSaveDataTo(){
        final String ret= Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_MOVIES)+"/FPV_VR/";
        File dir = new File(ret);
        if (!dir.exists()) {
            final boolean mkdirs = dir.mkdirs();
            //System.out.println("mkdirs res"+mkdirs);
        }
        return ret;
    }

    // Adds a .fpv file (created via the ndk file api) to the ContentResolver such that it is
    // indexed by the android os (displayed via the files app)
    // Called via jni from native code
    public static void addFpvFileToContentProvider(final Context c,final String pathToFile){
        Log.d(TAG,"Add file "+pathToFile);
        ContentValues values = new ContentValues();
        //values.put(MediaStore.Video.VideoColumns.DATE_ADDED, System.currentTimeMillis() / 1000);
        //values.put(MediaStore.Video.Media.DISPLAY_NAME, "display_name");
        //values.put(MediaStore.Video.Media.TITLE, "my_title");
        values.put(MediaStore.MediaColumns.MIME_TYPE, "video/fpv");
        //values.put(MediaStore.Video.Media.RELATIVE_PATH, "$Q_VIDEO_PATH/$relativePath")
        values.put(MediaStore.Video.Media.DATA,pathToFile);
        final Uri uri=c.getContentResolver().insert(MediaStore.Video.Media.EXTERNAL_CONTENT_URI, values);
        if(uri!=null){
            Log.d(TAG,"URI "+uri.toString());
            Toast.makeText(c,"Saved Ground Recording File "+pathToFile,Toast.LENGTH_SHORT).show();
        }else{
            //Log.w(TAG,"URI is null - something went wrong");
            throw new RuntimeException("Cannot add file to contentProvider "+pathToFile);
        }
    }

}
