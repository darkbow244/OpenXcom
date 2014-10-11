package org.libsdl.openxcom;

import org.libsdl.app.SDLActivity;
import android.util.Log;
import android.content.res.Configuration;
import android.content.pm.ActivityInfo;

/*
 * Just a wrapper, I guess
 *
 */


public class OpenXcom extends SDLActivity { 
	public static void showDirDialog() {
		Log.i("OpenXcom", "Hello from showDirDialog()!");

	}

	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		if (newConfig.orientation == Configuration.ORIENTATION_PORTRAIT) {
			newConfig.orientation = Configuration.ORIENTATION_LANDSCAPE;
		}

		super.onConfigurationChanged(newConfig);
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE);
	}

};
