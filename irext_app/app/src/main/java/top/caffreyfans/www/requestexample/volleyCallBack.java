package top.caffreyfans.www.requestexample;

import org.json.JSONObject;

import java.util.List;

/**
 * 实现volley回调1：定义监听接口
 */
public interface volleyCallBack {
    public void afterGetData(JSONObject object);
}
