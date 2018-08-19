package top.caffreyfans.www.requestexample;

import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import com.android.volley.toolbox.Volley;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.List;


public class applianceAdapter extends BaseAdapter {

    private List<String> applianceName;
    LayoutInflater mInflater;

    public applianceAdapter(Context c, List<String> list){
        this.applianceName = list;
        mInflater = (LayoutInflater) c.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    @Override
    public int getCount() {
        return applianceName.size();
    }

    @Override
    public Object getItem(int i) {
        return applianceName.get(i);
    }

    @Override
    public long getItemId(int i) {
        return i;
    }

    @Override
    public View getView(int i, View view, ViewGroup viewGroup) {
        View v = mInflater.inflate(R.layout.appliancetype, null);
        TextView textView = (TextView) v.findViewById(R.id.typeNameTextView);
        String name = applianceName.get(i);
        textView.setText(name);
        return v;
    }
}
