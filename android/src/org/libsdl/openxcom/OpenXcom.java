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

	public static int systemUIStyle;

	protected final static string SYSTEM_UI_NAME = "SystemUIStyle";
	protected final int SYSTEM_UI_ALWAYS_SHOWN = 0;
	protected final int SYSTEM_UI_LOW_PROFILE = 1;
	protected final int SYSTEM_UI_IMMERSIVE = 2;

	@Override
	protected void onCreate(Bundle savedInstance) {
		super.onCreate(savedInstance);
		SharedPreferences preferences = getSharedPreferences(DirsConfigActivity.PREFS_NAME, 0);
		systemUIStyle = preferences.getInt(SYSTEM_UI_NAME, 0);
		setSystemUI;
	}

	public void setSystemUI() {
		final View rootView = getWindow().getDecorView();
		int systemUIFlags;

		// Only set these flags if the device supports them
		if (Build.VERSION.SDK_INT >= 11) {
			switch(systemUIStyle) {
				case SYSTEM_UI_ALWAYS_SHOWN:
					// Set flags to "visible"
					if (Build.VERSION.SDK_INT < 14) {
						systemUIFlags = View.STATUS_BAR_VISIBLE;
					} else {
						systemUIFlags = View.SYSTEM_UI_FLAG_VISIBLE;
					}
					break;
				case SYSTEM_UI_LOW_PROFILE:
					// Set flags to "low profile"
					if (Build.VERSION.SDK_INT < 14) {
						systemUIFlags = View.STATUS_BAR_HIDDEN;
					} else {
						systemUIFlags = View.SYSTEM_UI_FLAG_LOW_PROFILE
					}
					break;
				case SYSTEM_UI_IMMERSIVE:
					// Set flags to "Immersive", use "Low profile" otherwise
					if (Build.VERSION.SDK_INT < 19) {
						systemUIFlags = View.SYSTEM_UI_FLAG_FULLSCREEN
										| View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
										| View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;
					} else {
						if (Build.VERSION.SDK_INT < 14) {
							systemUIFlags = View.STATUS_BAR_HIDDEN;
						} else {
							systemUIFlags = View.SYSTEM_UI_FLAG_LOW_PROFILE;
						}
					}
			}
			rootView.setSystemUiVisibility(systemUIFlags);
			if (systemUIFlags & (View.STATUS_BAR_HIDDEN | SYSTEM_UI_FLAG_LOW_PROFILE)) {
				// Set a listener to dim back navigation buttons
				rootView.setOnSystemUiVisibilityChangeListener(
					new View.OnSystemUiVisibilityChangeListener(){
						@Override
						public void onSystemUiVisibilityChange(int visibility) {
							Timer timer = new Timer();
							timer.schedule(new TimerTask() {
								@Override
								public void run() {
									OpenXcom.this.runOnUiThread(new Runnable() {
										rootView.setSystemUiVisibility(systemUiFlags);
									}); // Runnable ends here
								}
							}, 1000); // TimerTask ends here
						}

					}); // listener ends here
			}
        	rootView.setOnSystemUiVisibilityChangeListener(new View.OnSystemUiVisibilityChangeListener() {
        		@Override
        		public void onSystemUiVisibilityChange(int visibility) {
        			Timer timer = new Timer();
        			timer.schedule(new TimerTask() {
        				@Override
        				public void run() {
        					SDLActivity.this.runOnUiThread(new Runnable() {
        						@Override
        						public void run() {
        							int systemUiFlags;
        							if (Build.VERSION.SDK_INT < 14) {
        								systemUiFlags = View.STATUS_BAR_HIDDEN;
        							} else {
        								systemUiFlags = View.SYSTEM_UI_FLAG_LOW_PROFILE;
        							}
        							rootView.setSystemUiVisibility(systemUiFlags);
        						}
        					});
        				}
        			}, 1000);
        		}
	    
        	});
        }
	}

	public static void showDirDialog() {
		android.app.Activity context = (android.app.Activity) SDLActivity.getContext();
		Log.i("OpenXcom", "Starting directory configuration dialog...");
		Intent intent = new Intent(context, DirsConfigActivity.class);
		context.startActivityForResult(intent, 0);
	}

	public void changeSystemUI(int newSystemUIStyle) {
		Log.i("OpenXcom", "Changing system UI");
		systemUIStyle = newSystemUIStyle;
		SharedPreferences preferences = getSharedPreferences(DirsConfigActivity.PREFS_NAME, 0);
		SharedPreferences.Editor preferencesEditor = preferences.edit();
		preferencesEditor.putInt(SYSTEM_UI_NAME, systemUIStyle);
		preferencesEditor.apply();
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
