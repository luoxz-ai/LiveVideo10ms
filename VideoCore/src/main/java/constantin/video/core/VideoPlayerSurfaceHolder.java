package constantin.video.core;

import android.content.Context;
import android.view.Surface;
import android.view.SurfaceHolder;

import constantin.video.core.IVideoParamsChanged;
import constantin.video.core.VideoPlayer;

/**
 * In contrast to VideoPlayerSurfaceTexture, here the lifecycle is tied to the SurfaceHolder backing a android SurfaceView
 * Since the surface is created / destroyed when pausing / resuming the app there is no need
 * for a Lifecycle Observer
 */
public class VideoPlayerSurfaceHolder implements SurfaceHolder.Callback{

    private final VideoPlayer videoPlayer;

    public VideoPlayerSurfaceHolder(final Context context, final IVideoParamsChanged iVideoParamsChanged){
        videoPlayer=new VideoPlayer(context,iVideoParamsChanged);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        final Surface surface=holder.getSurface();
        videoPlayer.prepare(surface);
        videoPlayer.addAndStartReceiver();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        //Log.d(TAG, "Video surfaceChanged fmt=" + format + " size=" + width + "x" + height);
        //format 4= rgb565
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        videoPlayer.stopAndRemovePlayerReceiver();
    }

}
