/*
 * Copyright (c) 2019 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.lineageos.simdetect;

import android.app.AlertDialog;
import android.app.Service;
import android.content.ActivityNotFoundException;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Resources;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.PowerManager;
import android.os.UEventObserver;
import android.util.Log;
import android.view.WindowManager;
import com.android.internal.R;

public class SimDetectService extends Service {
    private static final String TAG = "SimDetectService";

    public static final String EXTRA_ICC_CARD_ADDED =
            "com.android.internal.telephony.uicc.ICC_CARD_ADDED";

    // From drivers/misc/sim_detect.c
    private static final String NOTHING_HAPPENED = "0";
    private static final String SIM_REMOVED = "1";
    private static final String SIM_INSERTED = "2";

    private final Object mLock = new Object();

    private final UEventObserver mSimDetectEventObserver = new UEventObserver() {
        @Override
        public void onUEvent(UEvent event) {
            synchronized (mLock) {
                String switchState = event.get("SWITCH_STATE");

                if (SIM_REMOVED.equals(switchState)) {
                    promptForRestart(false);
                } else if (SIM_INSERTED.equals(switchState)) {
                    promptForRestart(true);
                }
            }
        }
    };

    @Override
    public void onCreate() {
        mSimDetectEventObserver.startObserving("DEVPATH=/devices/virtual/switch/sim_detect");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private void promptForRestart(boolean isAdded) {
        synchronized (mLock) {
            final Resources res = getResources();
            final String dialogComponent = res.getString(
                    R.string.config_iccHotswapPromptForRestartDialogComponent);

            if (dialogComponent != null) {
                Intent intent = new Intent().setComponent(ComponentName.unflattenFromString(
                        dialogComponent)).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                        .putExtra(EXTRA_ICC_CARD_ADDED, isAdded);

                try {
                    startActivity(intent);
                    return;
                } catch (ActivityNotFoundException e) {
                    Log.e(TAG, "Unable to find ICC hotswap prompt for restart activity: " + e);
                }
            }

            // TODO: Here we assume the device can't handle SIM hot-swap
            //      and has to reboot. We may want to add a property,
            //      e.g. REBOOT_ON_SIM_SWAP, to indicate if modem support
            //      hot-swap.
            final DialogInterface.OnClickListener listener =
                    new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    synchronized (mLock) {
                        if (which == DialogInterface.BUTTON_POSITIVE) {
                            PowerManager pm = (PowerManager)
                                    getSystemService(Context.POWER_SERVICE);
                            pm.reboot("SIM is added.");
                        }
                    }
                }
            };

            Resources r = Resources.getSystem();
            String title = r.getString(
                    isAdded ? R.string.sim_added_title : R.string.sim_removed_title);
            String message = r.getString(
                    isAdded ? R.string.sim_added_message : R.string.sim_removed_message);
            String buttonText = r.getString(R.string.sim_restart_button);

            new Handler(Looper.getMainLooper()).post(new Runnable() {
                @Override
                public void run() {
                    AlertDialog dialog = new AlertDialog.Builder(SimDetectService.this)
                            .setTitle(title)
                            .setMessage(message)
                            .setPositiveButton(buttonText, listener)
                            .create();
                    dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
                    dialog.show();
                }
            });
        }
    }
}
