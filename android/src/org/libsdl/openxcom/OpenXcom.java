package org.libsdl.openxcom;

import org.libsdl.app.SDLActivity;
//import org.libsdl.openxcom.DirsConfigActivity;




import android.os.Build;
import android.util.Log;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.content.pm.ActivityInfo;

/*
 * Just a wrapper, I guess
 *
 */


public class OpenXcom extends SDLActivity { 
	public static void showDirDialog() {
		android.app.Activity context = (android.app.Activity) SDLActivity.getContext();
		Log.i("OpenXcom", "Starting directory configuration dialog...");
		Intent intent = new Intent(context, DirsConfigActivity.class);
		context.startActivityForResult(intent, 0);
	}
	
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		super.onActivityResult(requestCode, resultCode, data);
		SharedPreferences preferences = getSharedPreferences(DirsConfigActivity.PREFS_NAME, 0);
		String dataPath = preferences.getString(DirsConfigActivity.DATA_PATH_KEY, "");
		String savePath = preferences.getString(DirsConfigActivity.SAVE_PATH_KEY, "");
		String confPath = preferences.getString(DirsConfigActivity.CONF_PATH_KEY, "");
		nativeSetPaths(dataPath, savePath, confPath);
	}
	
	public static native void nativeSetPaths(String dataPath, String savePath, String confPath);

	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		if (newConfig.orientation == Configuration.ORIENTATION_PORTRAIT) {
			newConfig.orientation = Configuration.ORIENTATION_LANDSCAPE;
		}

		super.onConfigurationChanged(newConfig);
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE);
	}

};
