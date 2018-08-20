package top.caffreyfans.www.requestexample;

import android.app.ProgressDialog;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

import org.json.JSONException;
import org.json.JSONObject;

public class mqttConfig extends AppCompatActivity {

    private Button configBtn;
    private EditText mqtt_host;
    private EditText mqtt_port;
    private EditText mqtt_user;
    private EditText mqtt_password;
    private JSONObject mqtt_config;
    private udpUntils udpUntils;
    private ProgressDialog progressDialog;
    private JSONObject json_object;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_mqtt_config);

        configBtn = (Button) findViewById(R.id.confirm_btn);
        mqtt_host = (EditText) findViewById(R.id.mqttHostET);
        mqtt_port = (EditText) findViewById(R.id.mqttPortET);
        mqtt_user = (EditText) findViewById(R.id.mqttUserET);
        mqtt_password = (EditText) findViewById(R.id.mqttPwdET);
        progressDialog = new ProgressDialog(this);
        mqtt_config = new JSONObject();
        udpUntils = new udpUntils();
        json_object = new JSONObject();

        configBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                try {
                    mqtt_config.put("host", mqtt_host.getText().toString());
                    mqtt_config.put("port", mqtt_port.getText().toString());
                    mqtt_config.put("user", mqtt_user.getText().toString());
                    mqtt_config.put("password", mqtt_password.getText().toString());
                    json_object.put("mqtt", mqtt_config);

                    progressDialog.setMessage("正在连接到MQTT服务器...");
                    progressDialog.show();
                    progressDialog.setCancelable(false);
                    new Thread(new Runnable() {
                        @Override
                        public void run() {
                            udpUntils.sendMessage(json_object.toString());
                            if(udpUntils.umpReceive().equals("connected")){
                                progressDialog.setMessage("连接成功...");
                                progressDialog.setCancelable(true);
                            }
                            else{
                                progressDialog.setMessage("连接超时...");
                                progressDialog.setCancelable(true);
                            }
                        }
                    }).start();

                } catch (JSONException e) {
                    e.printStackTrace();
                }
            }
        });
    }
}
