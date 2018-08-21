package top.caffreyfans.www.requestexample;

import android.content.Context;
import android.util.Log;

import java.io.FileInputStream;
import java.io.IOException;
import java.net.BindException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;


public class udpUtils {

    private int server_port = 8000;
    private byte[] buffer = new byte[256];
    private Context context;
    private String filename = "ip_address";
    private String ip_address;

    public udpUtils(Context context){
        this.context = context;
    }

    public void sendMessage(String msg){


        FileInputStream inputStream;

        try {

            //调用方法创建流，参数1：文件名参数2：文件类型为私有
            inputStream = context.openFileInput(filename);

            //调用流的write方法
            int length = inputStream.available();
            byte[] buffer = new byte[256];
            inputStream.read(buffer, 0, length);
            inputStream.close();
            ip_address = new String(buffer, 0, length);
            Log.i("get ip_address:", ip_address);
        }
        catch (Exception e) {
            e.printStackTrace();
        }

        try {
            DatagramSocket s = new DatagramSocket();
            InetAddress local = InetAddress.getByName(ip_address);
            int msg_length=msg.length();
            byte[] message = msg.getBytes();
            DatagramPacket p = new DatagramPacket(message, msg_length, local, server_port);
            s.send(p);
            s.close();

        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public String umpReceive(){
        String str = null;
        DatagramSocket ds = null;
        int data_length = 0;
        try {
            ds = new DatagramSocket(server_port);
        } catch (BindException e){
            e.printStackTrace();
        } catch (SocketException e){
            e.printStackTrace();
        }
        while (ds != null && data_length==0) {
            DatagramPacket dp = new DatagramPacket(buffer, buffer.length);
            try {
                ds.receive(dp);
                data_length = dp.getLength();
                if (data_length != 0){
                    str = new String(dp.getData(), 0, dp.getLength());
                    Log.i("esp8266:", str);
                }
            } catch (IOException e){
                e.printStackTrace();
            }
        }
        ds.close();
        return str;
    }


}
