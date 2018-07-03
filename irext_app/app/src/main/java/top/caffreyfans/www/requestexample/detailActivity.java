package top.caffreyfans.www.requestexample;

import android.content.Context;
import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ListView;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class detailActivity extends AppCompatActivity {

    private String TAG;
    private ListView mListView;
    private JSONArray entity;
    private Intent in;
    private String type;
    private int categoryId, brandId, id;
    private ArrayList<String> list;

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_detail);

        varInit();

        if (type.equals("applianceTypes")) {

            // 请求打开电器类型列表界面
            Gobal.Irext.setVolleyCallBack(new volleyCallBack() {
                @Override
                public void afterGetData(JSONObject object) {
                    try {
                        if (object != null) {
                            entity = object.getJSONArray("entity");
                            for (int i=0; i<entity.length(); i++) {
                                list.add(entity.getJSONObject(i).getString("name"));
                            }
                            showItems(list);
                            mListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
                                @Override
                                public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
                                    Intent intent = (Intent) new Intent(getApplicationContext(), detailActivity.class);
                                    intent.putExtra("Irext.Type", "applianceBrands");
                                    try {
                                        categoryId = entity.getJSONObject(i).getInt("id");
                                    } catch (JSONException e) {
                                        e.printStackTrace();
                                    }
                                    intent.putExtra("Irext.categoryId", categoryId);
                                    startActivity(intent);
                                }
                            });
                        }
                    } catch (JSONException e) {
                        e.printStackTrace();;
                    }
                }
            });
            Gobal.Irext.getApplianceTypes();
        }

        if (type.equals("applianceBrands")) {

            // 请求打开电器品牌列表界面
            Gobal.Irext.setVolleyCallBack(new volleyCallBack() {
                @Override
                public void afterGetData(JSONObject object) {
                    try {
                        if (object != null) {
                            entity = object.getJSONArray("entity");
                            for (int i=0; i<entity.length(); i++) {
                                list.add(entity.getJSONObject(i).getString("name"));
                            }
                            showItems(list);
                            mListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
                                @Override
                                public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
                                    Intent intent = (Intent) new Intent(getApplicationContext(), detailActivity.class);
                                    intent.putExtra("Irext.Type", "codeList");
                                    try {
                                        categoryId = entity.getJSONObject(i).getInt("categoryId");
                                        brandId = entity.getJSONObject(i).getInt("id");
                                    } catch (JSONException e) {
                                        e.printStackTrace();
                                    }
                                    intent.putExtra("Irext.categoryId", categoryId);
                                    intent.putExtra("Irext.brandId", brandId);
                                    startActivity(intent);
                                }
                            });
                        }
                    } catch (JSONException e) {
                        e.printStackTrace();;
                    }
                }
            });

            Log.i(TAG, "the categoryId is" + Integer.toString(categoryId));
            Gobal.Irext.getApplianceBrands(categoryId);
        }

        if (type.equals("codeList")) {

            // 请求遥控码索引列表界面
            Gobal.Irext.setVolleyCallBack(new volleyCallBack() {
                @Override
                public void afterGetData(JSONObject object) {
                    try {
                        if (object != null) {
                            entity = object.getJSONArray("entity");
                            for (int i=0; i<entity.length(); i++) {
                                list.add(entity.getJSONObject(i).getString("id"));
                            }
                            showItems(list);
                            mListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
                                @Override
                                public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
                                    Intent intent = (Intent) new Intent(getApplicationContext(), OnoffActivity.class);
                                    intent.putExtra("Irext.Type", "codeList");
                                    try {
                                        id = entity.getJSONObject(i).getInt("id");
                                    } catch (JSONException e) {
                                        e.printStackTrace();
                                    }
                                    intent.putExtra("Irext.id", id);
                                    startActivity(intent);
                                }
                            });
                        }
                    } catch (JSONException e) {
                        e.printStackTrace();;
                    }
                }
            });

            Log.i(TAG, "the categoryId is" + Integer.toString(categoryId));
            Gobal.Irext.getCodeList(categoryId, brandId);
        }
    }

    private void varInit(){
        TAG  = (String) "The Detail";
        mListView = (ListView) findViewById(R.id.mListView);
        entity = (JSONArray) new JSONArray();
        in = (Intent) getIntent();
        type = (String) in.getStringExtra("Irext.Type");
        categoryId = in.getIntExtra("Irext.categoryId", -1);
        brandId = in.getIntExtra("Irext.brandId", -1);
        id = in.getIntExtra("Irext.Id",-1);
        list = (ArrayList<String>) new ArrayList<String>();
    }

    private void showItems(List<String> list) {

        // 改变ListView
        applianceAdapter applianceAdapter = new applianceAdapter(getApplicationContext(), list);
        mListView.setAdapter(applianceAdapter);
    }

}
