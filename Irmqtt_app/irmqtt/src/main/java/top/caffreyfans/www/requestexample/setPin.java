package top.caffreyfans.www.requestexample;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

import org.json.JSONException;
import org.json.JSONObject;

public class setPin extends AppCompatActivity {
    private Button setPinBtn;
    private udpUntils udpUntils;
    private EditText set_pin;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_set_pin);
        setPinBtn = (Button) findViewById(R.id.setPinBtn);
        set_pin = (EditText) findViewById(R.id.setPinET);
        udpUntils = new udpUntils();

        setPinBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        final JSONObject jsonObject = new JSONObject();
                        try {
                            jsonObject.put("data_pin", set_pin.getText().toString());
                        } catch (JSONException e) {
                            e.printStackTrace();
                        }
                        udpUntils.sendMessage(jsonObject.toString());
                    }
                }).start();
            }
        });
    }
}
