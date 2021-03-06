
package com.yarlungsoft.ams;

import java.io.File;
import java.io.IOException;
import java.net.ota.OTAConfig;
import java.net.ota.OTADownload;
import java.net.ota.OTAListener;

import com.yarlungsoft.util.Log;

public class TCKRunner extends Thread implements OTAListener {

    private static final String TAG = "TCKRunner";

    /** The number of tries for installing test Applets */
    private static final int TRIES_FOR_TEST_FINISHED = 6;

    /** The number of milliseconds to wait between retries */
    private static final int RETRY_DELAY = 1000;

    /** The number of milliseconds to wait before removing */
    private static final int REMOVE_DELAY = 1000;

    /** The interval in milliseconds of checking OTA status */
    private static final long CHECK_INTERVAL = 5000;

    private byte[] lock = new byte[0];
    private int otaResult = OTAConfig.OTA_UNKNOWN;
    private String otaStorageFilename = null;

    private String jadUrl = null;
    private boolean restartRunner = false;

    public TCKRunner() {
        super();
        setPriority(MAX_PRIORITY);
    }

    public TCKRunner(String jadUrl) {
        this();
        this.jadUrl = jadUrl;
    }

    public static void launchFromNative(String[] urls) {
        if (urls == null || urls.length == 0) {
            Log.amsLog(TAG, "nothing to do");
            return;
        }
        Log.amsLog(TAG, "url " + urls[0]);
        TCKRunner tckrunner = new TCKRunner(urls[0]);
        tckrunner.start();
    }

    @Override
    public void onResult(int result) {
        synchronized (lock) {
            otaResult = result;
            Log.amsLog(TAG, "ota result " + result + "\n");
            lock.notifyAll();
        }
    }

    private void sleep0(long time) {
        try {
            sleep(time);
        } catch (InterruptedException e) {
            // ignore
        }
    }

    private boolean downloadUrl(String url) {
        int tries = 0;
        boolean isDownloaded = false;
        OTADownload ota = null;
        otaStorageFilename = null;

        /*
         * We need to distinguish between failures that are expected during the testing, and
         * failures that should cause us to stop testing and print an error message. We print
         * error messages for everything, to make sure we have the information in the log.
         * <p>
         * We ignore all failures - just reset the VM and try again - with the following
         * exceptions. This can lead to the VM restarting in a furious tempo. If that happened
         * to you, consider adding to this list of exceptions:
         * OTA_IO_ERROR, OTA_TASK_FAIL, OTA_NET_ERROR: received when the TCK is finished
         * <p>
         * For these exceptions we re-try 3 times (TRIES_FOR_TEST_FINISHED) to avoid exiting for
         * something that was part of a normal test.
         */
        while (!isDownloaded && tries < TRIES_FOR_TEST_FINISHED) {
            Log.amsLog(TAG, "installing from " + url + " (try " + (tries + 1) + "/"
                            + TRIES_FOR_TEST_FINISHED + ")");

            ota = new OTADownload(url);
            ota.setListener(this);

            do {
                otaResult = OTAConfig.OTA_UNKNOWN;
                ota.OTAStart();

                while (otaResult == OTAConfig.OTA_UNKNOWN) {
                    try {
                    	Log.amsLog(TAG, "sleep 100 ms to wait ota result \n");
                    	sleep(100);
                        //lock.wait(CHECK_INTERVAL);
                    } catch (InterruptedException ie) {
                        // ignore
                    }
                }

                isDownloaded = (otaResult == OTAConfig.OTA_SUCCESS);

                switch (otaResult) {
                case OTAConfig.OTA_SUCCESS:
                    tries = 0;
                    break;
                case OTAConfig.OTA_FILE_ERROR:
                    Log.amsLog(TAG, "folder not found: " + ota.getAppDir());
                    tries = 0;
                    break;
                case OTAConfig.OTA_INVALID_URL:
                    Log.amsLog(TAG, "invalid TCK url: " + url);
                    tries = 0;
                    break;

                case OTAConfig.OTA_IO_ERROR:
                case OTAConfig.OTA_TASK_FAIL:
                    Log.amsLog(TAG, "failed to download: " + url);
                    tries++;
                    break;
                case OTAConfig.OTA_NET_ERROR:
                    Log.amsLog(TAG, "cannot connect to: " + url);
                    tries++;
                    break;
                }
            } while(false);

            if (!isDownloaded) {
                Log.amsLog(TAG, "waiting for " + (RETRY_DELAY / 1000) + " seconds");
                sleep0(RETRY_DELAY);
                if (tries == 0) {
                    break;
                }
            }
        }

        if (isDownloaded) {
            otaStorageFilename = ota.getStorageFilename();
            Log.amsLog(TAG, "downloaded " + otaStorageFilename);
        } else {
            if (tries >= TRIES_FOR_TEST_FINISHED) {
                Log.amsLog(TAG, "install() failing " + TRIES_FOR_TEST_FINISHED
                                + " times, we suppose tests are finished");
                restartRunner = false;
            } else {
                Log.amsLog(TAG, "install() failing, we restart the vm and retry");
                restartRunner = true;
            }
        }

        return isDownloaded;
    }

    private void removeCurDownloadedFile() {
        if (otaStorageFilename == null) {
            return;
        }
        File f = new File(otaStorageFilename);
        if (f.exists()) {
            f.delete();
        }
    }

    public void run() {
        Log.amsLog(TAG, "started!");
        boolean downloaded = false;
        boolean bRestart = true;

        do {
            downloaded = downloadUrl(jadUrl);
            Log.amsLog(TAG, "downloaded=" + downloaded);
            if (!downloaded) {
                if (restartRunner) {
                    continue;
                } else {
                	bRestart = false;
                    break;
                }
            }

            JadInfo jadInfo = null;
            try {
                Log.amsLog(TAG, "parseJad:" + otaStorageFilename);
                jadInfo = new JadParser().parseJad(otaStorageFilename);
                Log.amsLog(TAG, "jadInfo=" + jadInfo);
                if (jadInfo != null) {
                    jadInfo.jarUrl = JadParser.buildJarUrl(jadUrl, jadInfo.jarUrl);
                    Log.amsLog(TAG, "jadInfo.jarUrl=" + jadInfo.jarUrl);
                    Log.amsLog(TAG, "jadInfo.appClass=" + jadInfo.appClass);
                }
            } catch (IOException e) {
                Log.amsLog(TAG, "parseJad exception:" + e);
            }
            
            Log.amsLog(TAG, "restartRunner :" + restartRunner);
            if (jadInfo == null || jadInfo.jarUrl == null || jadInfo.jarUrl.length() == 0
                    /*|| jadInfo.appClass == null || jadInfo.appClass.length() == 0*/) {
                if (restartRunner) {
                	Log.amsLog(TAG, "restartRunner : continue");
                    continue;
                } else {
                	Log.amsLog(TAG, "restartRunner : break");
                	bRestart = false;
                    break;
                }
            }

            // delete downloaded jad file
            removeCurDownloadedFile();

            Log.amsLog(TAG, "ready to doanload :" + jadInfo.jarUrl);
            downloaded = downloadUrl(jadInfo.jarUrl);
            if (!downloaded) {
                if (restartRunner) {
                    continue;
                } else {
                	bRestart = false;
                    break;
                }
            }
            Log.amsLog(TAG, "file to doanload : " + otaStorageFilename);
            Log.amsLog(TAG, "final to doanload : " + jadInfo.jarUrl);

            Main.launchApp(jadInfo.appClass, otaStorageFilename);

            Log.amsLog(TAG, "launch " + jadInfo.appClass + "over!");
            /* waiting for the applet finishing */
            while (Scheduler.getCurrentRunningApp() != null) {
            	Log.amsLog(TAG, "wait " + jadInfo.appClass + "over!");
                sleep0(CHECK_INTERVAL);
            }
            Log.amsLog(TAG, "run " + jadInfo.appClass + "over!");
            /*
             * The sleep is to allow tracing to come out on slow machines, and to prevent spinning
             * if something goes wrong.
             */
            sleep0(REMOVE_DELAY);

            // delete downloaded jar file
            removeCurDownloadedFile();
            Log.amsLog(TAG, "ready to run next tck !");
        } while(false);
        if(!bRestart){
        	Main.setNextSchedulerState(Main.NATIVE_NEXT_SCDH_STATE_NULL);
        } else {
        	Main.setNextSchedulerState(Main.NATIVE_NEXT_SCDH_STATE_TCK);
        }
        Log.amsLog(TAG, "ended!");
    }
}
