package org.libsdl.openxcom;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Locale;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.content.res.AssetManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.TextView;

public class PreloaderActivity extends Activity {

	protected AssetManager assets = null;
	protected TextView preloaderLog = null;
	protected Context context;
	private ProgressDialog pd;
	
	
	final String dataMarkerName = "openxcom_data_marker";
	final String translationMarkerName = "openxcom_translation_marker";
	final String patchMarkerName = "openxcom_patch_marker";
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_preloader);
		context = this;
		preloaderLog = (TextView) findViewById(R.id.preloaderLog);
	}
	
	@Override
	protected void onDestroy() {
		if (pd != null) {
			pd.dismiss();
		}
		super.onDestroy();
	}
	
	/**
	 * We have to determine if the user has the latest version
	 * of the OpenXcom data, languages and patches. If not, we'll have to install them.
	 */
	
	@Override
	protected void onStart() {
		super.onStart();
		// We only want to do all this if we have the game files.
		if (hasGameFiles()) {
			new AsyncTask<Void, String, Void>() 
				{
			
					@Override
					protected void onPreExecute() {
						pd = new ProgressDialog(context);
						pd.setTitle("Pre-loading OpenXcom 1.0...");
						pd.setMessage("Initializing...");
						pd.setCancelable(false);
						pd.setIndeterminate(true);
						pd.show();
						Log.i("OpenXcom", "AsyncTask started");
					}
					
					public void onProgressUpdate(String... message) {
						pd.setMessage(message[0]);
					}
					
					@Override
					protected Void doInBackground(Void... arg0) {
						try {
							assets = context.getAssets();
							publishProgress("Checking data version...");
							if (needsUpdating(dataMarkerName)) {
								publishProgress("Extracting data...");
								extractFile("data.zip", "/sdcard/OpenXcom/");
								copyMarker(dataMarkerName);
							}
							publishProgress("Checking translations version...");
							if (needsUpdating(translationMarkerName)) {
								publishProgress("Extracting translations...");
								extractFile("latest.zip", "/sdcard/OpenXcom/data/Language/");
								copyMarker(translationMarkerName);
							}
							publishProgress("Checking patch version...");
							if (needsUpdating(patchMarkerName)) {
								publishProgress("Applying patch...");
								extractFile("universal-patch.zip", "/sdcard/OpenXcom/data/");
								copyMarker(patchMarkerName);
							}
						}
						catch (Exception e) {
							e.printStackTrace();
						}
						return null;
					}
					
					@Override
					protected void onPostExecute(Void result) {
						if (pd != null) {
							pd.dismiss();
						}
						Log.i("OpenXcom", "Finishing asynctask...");
						PassExecution();	
					}
					
				}.execute((Void[]) null);
		}
	}
	
	/**
	 * This method gets called by AsyncThread to execute the main activity.
	 */
	
	protected void PassExecution() {
		Intent intent = new Intent(this, OpenXcom.class);
		startActivity(intent);
		
	}
	
	
	
	/**
	 * A helper method to extract the contents of assetName to extractPath.
	 * Be extremely careful to put things where they're supposed to be!
	 */
	
	protected void extractFile(String assetName, String extractPath) {
		InputStream is;
		ZipInputStream zis;
		try {
			String filename;
			is = assets.open(assetName);
			zis = new ZipInputStream(new BufferedInputStream(is));
			ZipEntry ze;
			byte[] buffer = new byte[8192];
			int count;
			
			while((ze = zis.getNextEntry()) != null) {
				filename = ze.getName();
				
				if (ze.isDirectory()) {
					File fmd = new File(extractPath + filename);
					fmd.mkdirs();
					continue;
				}
				
				FileOutputStream fout = new FileOutputStream(extractPath + filename);
				
				while ((count = zis.read(buffer)) != -1) {
					fout.write(buffer, 0, count);
				}
				
				fout.close();
				zis.closeEntry();
			}
			zis.close();
			
		}
		catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	/**
	 * Method that (crudely) checks if we have the game files.
	 * @return True if the game files are properly installed.
	 */
	
	protected boolean hasGameFiles() {
		File checkFile = new File("/sdcard/OpenXcom/data/GEODATA/PALETTES.DAT");
		if (checkFile.exists()) {
			return true;
		}
		else {
			return false;
		}
	}
	
	/**
	 * Method that checks we need to update the data.
	 * @return True if we need to update data files.
	 */
	
	protected boolean needsUpdating(String markerName) {
		final String markerPath = "/sdcard/OpenXcom/data/";
		File checkFile = new File(markerPath + markerName);
		// It's our first time here, by the looks of it.
		if (!checkFile.exists()) {
			return true;
		}
		// Check file contents against the one we have in out assets.
		try {
			InputStream assetFileIS = assets.open(markerName);
			InputStream checkFileIS = new FileInputStream (checkFile);
			boolean areSame = sameContents(assetFileIS, checkFileIS);
			assetFileIS.close();
			checkFileIS.close();
			if (areSame) {
				return false;
			}
			return true;
		}
		catch (IOException e) {
			Log.e("OpenXcom", e.getMessage());
			return true;
		}
		
	}
	
	/**
	 * Returns 0 if files are same, and a non-zero value if the files are different.
	 * This function is very slow, so it will probably need a rewrite.
	 * @param fileIS1 Input stream associated with the first file
	 * @param fileIS2 Input stream associated with the second file
	 * @return True if the files are the same, false otherwise.
	 * @throws IOException 
	 */
	protected boolean sameContents(InputStream fileIS1, InputStream fileIS2) throws IOException {
		int b1 = fileIS1.read();
		int b2 = fileIS2.read();
		while((b1 != -1) && (b2 != -1))
		{
			if (b1 != b2) {
				return false;
			}
			b1 = fileIS1.read();
			b2 = fileIS2.read();
		}
		return b1 == b2;
	}
	
	/**
	 * Copies the marker from assets to data folder.
	 * @param markerName Filename of the data marker.
	 */
	
	protected void copyMarker(String markerName) {
		final String markerPath = "/sdcard/OpenXcom/data/";
	    InputStream in = null;
	    OutputStream out = null;
	    try {
	    	in = assets.open(markerName);
	        File outFile = new File(markerPath + markerName);
	        out = new FileOutputStream(outFile);
	        copyFile(in, out);
	        in.close();
	        in = null;
	        out.flush();
	        out.close();
	        out = null;
	    } 
	    catch(IOException e) {
	            Log.e("OpenXcom", "Failed to copy asset file: " + markerName, e);
	        }       
	    }
	/**
	 * Actually copies the file contents.
	 * @param in Stream we're copying from.
	 * @param out Stream we're copying to.
	 * @throws IOException
	 */
	protected void copyFile(InputStream in, OutputStream out) throws IOException {
	    byte[] buffer = new byte[1024];
	    int read;
	    while((read = in.read(buffer)) != -1){
	      out.write(buffer, 0, read);
	    }
	}
	
	
}
