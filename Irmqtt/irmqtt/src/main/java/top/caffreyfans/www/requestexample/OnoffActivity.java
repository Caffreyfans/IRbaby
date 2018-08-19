package top.caffreyfans.www.requestexample;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

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
        udpUntils = new udpUntils();

        switchOnBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        udpUntils.sendMessage(String.valueOf(index_id));
                    }
                }).start();
            }
        });


        irSendBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

            }
        });
    }
}
