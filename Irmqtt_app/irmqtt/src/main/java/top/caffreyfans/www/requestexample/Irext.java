package top.caffreyfans.www.requestexample;

import android.content.Context;
import android.util.Log;

import com.android.volley.Request;
import com.android.volley.RequestQueue;
import com.android.volley.Response;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.JsonObjectRequest;
import com.android.volley.toolbox.Volley;

import org.json.JSONException;
import org.json.JSONObject;

/**********************************************
 *
 * 实现所有Irext Web Services索引方法
 *
 **********************************************/
public class Irext {

    private static String TAG;
    private RequestQueue mRequestQueue;
    private JsonObjectRequest mJsonObjectRequest;
    private Context mContext;
    private JSONObject idToken;
    // 实现volley回调2：定义接口成员变量
    private volleyCallBack volleyCallBack;


    public Irext(Context context) {

        TAG = MainActivity.class.getSimpleName();
        mContext = context;
        idToken = new JSONObject();
        mRequestQueue = Volley.newRequestQueue(mContext);
    } // end of Irext object create


    public void setVolleyCallBack(volleyCallBack volleyCallBack) {
        // 实现volley回调3：实例化接口
        this.volleyCallBack = volleyCallBack;
    }

    public void appLogin() {

        // APP登录换取 id 和 token
        String url = "http://irext.net/irext-server/app/app_login";
        JSONObject jsonObject = new JSONObject();
        try{
            jsonObject.put("appKey", "4279187e58326959f1bc047f7900b4ee");
            jsonObject.put("appSecret", "157f29992370f02043aca66893716be9");
            jsonObject.put("appType", "2");
        } catch (JSONException e) {
            e.printStackTrace();
        }
        Gobal.Irext.setVolleyCallBack(new volleyCallBack() {
            @Override
            public void afterGetData(JSONObject object) {
                try{
                    idToken.put("id", object.getJSONObject("entity").getInt("id"));
                    idToken.put("token", object.getJSONObject("entity").getString("token"));
                } catch (JSONException e){
                    e.printStackTrace();
                }
            }
        });
        this.postMessage(url, jsonObject);
    } // End of APP Login


    public void getApplianceTypes() {

        // 获取遥控码(家电)类型列表
        String url = "http://irext.net/irext-server/indexing/list_categories";
        final JSONObject jsonObject = new JSONObject();
        try{
            jsonObject.put("id", idToken.getInt("id"));
            jsonObject.put("token", idToken.getString("token"));
            jsonObject.put("from", "0");
            jsonObject.put("count", "100");
        } catch (JSONException e) {
            e.printStackTrace();
        }
        this.postMessage(url, jsonObject);
        jsonObject.remove("from");
        jsonObject.remove("count");
        final JSONObject json_object = new JSONObject();
        try {
            json_object.put("auth", jsonObject);
        } catch (JSONException e) {
            e.printStackTrace();
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                Gobal.udpUtils.sendMessage(json_object.toString());
            }
        }).start();
    } // End of getApplianceTypes


    public void getApplianceBrands(int categoryId) {

        // 获取遥控码(家电)品牌列表
        String url = "http://irext.net/irext-server/indexing/list_brands";
        JSONObject jsonObject = new JSONObject();
        try{
            jsonObject.put("id", idToken.getInt("id"));
            jsonObject.put("token", idToken.getString("token"));
            jsonObject.put("categoryId", categoryId);
            jsonObject.put("from", "0");
            jsonObject.put("count", "100");
        } catch (JSONException e) {
            e.printStackTrace();
        }
        this.postMessage(url, jsonObject);
    }

    public void getCodeList(int categoryId, int brandId) {

        // 获取遥控码索引列表
        String url = "http://irext.net/irext-server/indexing/list_indexes";
        JSONObject jsonObject = new JSONObject();
        try{
            jsonObject.put("id", idToken.getInt("id"));
            jsonObject.put("token", idToken.getString("token"));
            jsonObject.put("categoryId", categoryId);
            jsonObject.put("brandId", brandId);
            jsonObject.put("from", "0");
            jsonObject.put("count", "100");
        } catch (JSONException e) {
            e.printStackTrace();
        }
        this.postMessage(url, jsonObject);
    }


    public void postMessage(String url, JSONObject msg) {

        // post 请求
        mRequestQueue = Volley.newRequestQueue(mContext);
        final JSONObject object = new JSONObject();
        try {
            mJsonObjectRequest = new JsonObjectRequest(Request.Method.POST, url, msg,
                    new Response.Listener<JSONObject>() {
                        @Override
                        public void onResponse(JSONObject response) {
                            try {
                                // 实现volley回调4：实现接口回调
                                if (volleyCallBack != null) {
                                    volleyCallBack.afterGetData(response);
                                }
                            } catch (Exception e) {
                                e.printStackTrace();
                            }
                        }
                    },
                    new Response.ErrorListener() {
                        @Override
                        public void onErrorResponse(VolleyError error) {
                            Log.i(TAG, error.toString());
                        }
                    });

            mRequestQueue.add(mJsonObjectRequest);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}