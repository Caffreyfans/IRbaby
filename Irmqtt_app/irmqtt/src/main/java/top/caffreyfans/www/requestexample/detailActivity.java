package top.caffreyfans.www.requestexample;

import android.app.ProgressDialog;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ListView;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

public class detailActivity extends AppCompatActivity {

    private String TAG;
    private ListView mListView;
    private JSONArray entity;
    private Intent in;
    private String type;
    private int categoryId, brandId, id;
    private ArrayList<String> list;
    private ProgressDialog progressDialog;

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
                                    JSONObject messageJson = new JSONObject();
                                    try{
                                        messageJson.put("Irext.categoryId", categoryId);
                                        messageJson.put("Irext.brandId", brandId);
                                    }catch (JSONException e){
                                        e.printStackTrace();
                                    }
                                    startActivity(intent);
                                }
                            });
                        }
                    } catch (JSONException e) {
                        e.printStackTrace();;
                    }
                }
            });

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
                            progressDialog.setMessage("等待ESP下载文件");
                            progressDialog.show();
                            progressDialog.setCancelable(false);

                            final JSONArray jsonArray = new JSONArray(list);
                            final JSONObject jsonObject = new JSONObject();
                            jsonObject.put("list", jsonArray);
                            // 发送 index_id List 到ESP
                            new Thread(new Runnable() {
                                @Override
                                public void run() {
                                    Gobal.udpUtils.sendMessage(jsonObject.toString());
                                    Gobal.udpUtils.umpReceive();
                                    handler.sendEmptyMessage(0);

                                }
                            }).start();

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
        progressDialog = new ProgressDialog(detailActivity.this);
    }

    private void showItems(List<String> list) {

        // 改变ListView
        applianceAdapter applianceAdapter = new applianceAdapter(getApplicationContext(), list);
        mListView.setAdapter(applianceAdapter);
    }


        Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {// handler接收到消息后就会执行此方法
            progressDialog.dismiss();// 关闭ProgressDialog
        }
    };

}
