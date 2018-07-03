package top.caffreyfans.www.requestexample;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.android.volley.Request;
import com.android.volley.RequestQueue;
import com.android.volley.Response;
import com.android.volley.VolleyError;
import com.android.volley.VolleyLog;
import com.android.volley.toolbox.JsonArrayRequest;
import com.android.volley.toolbox.JsonObjectRequest;
import com.android.volley.toolbox.StringRequest;
import com.android.volley.toolbox.Volley;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

public class MainActivity extends Activity {

    private String urlJsonObj =
            "https://api.androidhive.info/volley/person_object.json";
    private String urlJsonArry =
            "https://api.androidhive.info/volley/person_array.json";
    private static String TAG = MainActivity.class.getSimpleName();
    private Button getApplianceType;
    private static RequestQueue requestQueue;
    private ProgressDialog pDialog;
    private String jsonResponse;
    private RequestQueue mRequestQueue;
    private JsonObjectRequest mJsonObjectRequest;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        getApplianceType = (Button) findViewById(R.id.getApplianceTypeBtn);

        Gobal.Irext = new Irext(getApplicationContext());
        Gobal.Irext.appLogin();


        pDialog = new ProgressDialog(this);
        pDialog.setMessage("Please wait...");
        pDialog.setCancelable(false);

        getApplianceType.setOnClickListener(new View.OnClickListener() {
            // 设置选择设备类型响应事件
            @Override
            public void onClick(View view) {
                Intent intent = new Intent(getApplicationContext(), detailActivity.class);
                intent.putExtra("Irext.Type", "applianceTypes");
                startActivity(intent);
            }
        });

    }


    private void showpDialog() {

        if (!pDialog.isShowing())
            pDialog.show();
    }

    private void hidepDialog() {
        if (pDialog.isShowing())
            pDialog.dismiss();
    }


}