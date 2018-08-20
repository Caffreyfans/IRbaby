package top.caffreyfans.www.requestexample;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import org.json.JSONException;
import org.json.JSONObject;

public class OnoffActivity extends AppCompatActivity {
    private Button switchOnBtn;
    private Button irSendBtn;
    private udpUntils udpUntils;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_onoff);
        switchOnBtn = (Button) findViewById(R.id.switchOnBtn);
        irSendBtn = (Button) findViewById(R.id.irSendBtn);
        Intent in = (Intent) getIntent();
        final int index_id = in.getIntExtra("Irext.id", -1);
        final JSONObject jsonObject = new JSONObject();
        try {
            jsonObject.put("use_file", index_id);
        } catch (JSONException e) {
            e.printStackTrace();
        }
        udpUntils = new udpUntils();

        switchOnBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        udpUntils.sendMessage(jsonObject.toString());
                    }
                }).start();
            }
        });

        final JSONObject jsonObject1 = new JSONObject();
        try {
            jsonObject1.put("cmd", "ir");
        } catch (JSONException e) {
            e.printStackTrace();
        }
        irSendBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        udpUntils.sendMessage(jsonObject1.toString());
                    }
                }).start();
            }
        });
    }
}
