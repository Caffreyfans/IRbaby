package top.caffreyfans.www.requestexample;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import com.espressif.iot.esptouch.demo_activity.EsptouchDemoActivity;

public class MainActivity extends Activity {

    private static String TAG = MainActivity.class.getSimpleName();
    private Button connectWiFiBtn;
    private Button parseDeviceBtn;
    private Button mqttConfigBtn;
    private Button appAboutBtn;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        parseDeviceBtn = (Button) findViewById(R.id.parseDeviceBtn);
        connectWiFiBtn = (Button) findViewById(R.id.connectWiFiBtn);
        mqttConfigBtn = (Button) findViewById(R.id.mqttConfigBtn);
        appAboutBtn = (Button) findViewById(R.id.appAboutBtn);
        Gobal.Irext = new Irext(getApplicationContext());
        Gobal.Irext.appLogin();


        parseDeviceBtn.setOnClickListener(new View.OnClickListener() {
            // 设置选择设备类型响应事件
            @Override
            public void onClick(View view) {

                Intent intent = new Intent(getApplicationContext(), detailActivity.class);
                intent.putExtra("Irext.Type", "applianceTypes");
                startActivity(intent);
            }
        });

        connectWiFiBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent intent = new Intent(getApplicationContext(), EsptouchDemoActivity.class);
                startActivity(intent);
            }
        });

        mqttConfigBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(getApplicationContext(), mqttConfig.class);
                startActivity(intent);
            }
        });

        appAboutBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(getApplicationContext(), appAbout.class);
            }
        });
    }



}

