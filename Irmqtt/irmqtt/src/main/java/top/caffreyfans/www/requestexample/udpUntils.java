package top.caffreyfans.www.requestexample;

import android.util.Log;

import java.io.IOException;
import java.net.BindException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;


public class udpUntils {

    private int server_port = 8000;
    private byte[] buffer = new byte[256];

    public void sendMessage(String msg){
        try {

            DatagramSocket s = new DatagramSocket();
            InetAddress local = InetAddress.getByName("192.168.0.100");
            int msg_length=msg.length();
            byte[] message = msg.getBytes();
            DatagramPacket p = new DatagramPacket(message, msg_length, local, server_port);
            s.send(p);
            s.close();

        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void umpReceive(){

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
                    String str = new String(dp.getData(), 0, dp.getLength());
                    Log.i("udp:", str);
                }
            } catch (IOException e){
                e.printStackTrace();
            }
        }
    }

}
