import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Date;

import org.apache.http.Header;
import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.DefaultHttpClient;

public class Http {
	public Http(){

	}
	public static void main(String[] args) {
		try {
			UrlAndCookie uac=new UrlAndCookie();
			String cookie=uac.getCookie();
			ArrayList<String> urls=uac.getDownloadUrl();
			Thread[] ts=new Thread[urls.size()];
			int i=-1;
			for(String url : urls){
				i++;
				ts[i]=new Thread(new XunleiDownload(url,cookie));
				ts[i].start();
			}
			for(;i>=0;i--){
				ts[i].join();
			}
		} catch (IOException e) {
			e.printStackTrace();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
}
class UrlAndCookie {
	public String getCookie() throws IOException{
		BufferedReader br=new BufferedReader(new FileReader(cookieFile));
		String cookie=br.readLine();
		br.close();
		return cookie;
	}
	public ArrayList<String> getDownloadUrl() throws IOException{
		BufferedReader br=new BufferedReader(new FileReader(urlFile));
		ArrayList<String> urls=new ArrayList<String>();
		String url=null;
		while((url = br.readLine()) != null) {
			urls.add(url);
		}
		br.close();
		return urls;
	}
	private String cookieFile="c:/cookie.txt";
	private String urlFile="c:/url.txt";
}

class XunleiDownload implements Runnable {
	public XunleiDownload(String url,String cookie){
		downloadUrl=url;
		cookieStr=cookie;
	}
	public  void run() {
		try {
			Date start=null;
			Date end=null;
			HttpClient hc=new DefaultHttpClient();
			HttpGet hg=new HttpGet(downloadUrl);
			hg.addHeader("Cookie", cookieStr);
			HttpResponse hp=hc.execute(hg);
			HttpEntity et=null;
			String outfile=null;
			if(hp != null){
				if (hp.getStatusLine().getStatusCode() == 200 ){
					et=hp.getEntity();
					Header disp=hp.getFirstHeader("Content-Disposition");
					String dispv=disp.getValue();
					outfile=dispv.split("\"")[1];
					start=new Date();
					System.out.println(outfile + " : Download started at : " + start.toString());
				}
			}
			if(et != null){
				File of=new File(baseOutputDir + outfile);
				of.createNewFile();
				FileOutputStream ofs=new FileOutputStream(of);
				et.writeTo(ofs);
				ofs.close();
				end=new Date();
				System.out.println(outfile + " : Download ended at : " + end.toString());
				System.out.println(outfile + " : Download File Size : " + of.length());
				System.out.println(outfile + " : Average download speed : " + of.length() / (end.getTime() - start.getTime()) + "KBps");
			}
		} catch (ClientProtocolException e) {	
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	private String cookieStr=null;
	private String baseOutputDir="c:/x/";
	private String downloadUrl=null;
}
