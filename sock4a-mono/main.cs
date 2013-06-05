using System;
using System.Net.Sockets;
using System.Net;
using System.Threading;
using System.Collections;

public class Sock4a
{
    public static void Main(string[] args)
    {
        IPAddress Addr = IPAddress.Parse("0.0.0.0");
        TcpListener server=new TcpListener(Addr,21080);
        server.Start();
        while(true)
        {
            Socket browser=server.AcceptSocket();
            Pipe p=new Pipe(browser);
            Thread t=new Thread(new ThreadStart(p.run));
            t.IsBackground=true;
            t.Start();
        }
    }
}

public class Pipe
{
    private Socket socks;
    private Socket browser;
    private string ip="106.187.41.101";
    private int port=21080;
    public Pipe(Socket ibrowser)
    {
        browser=ibrowser;
    }
    public void SocksClient()
    {
        socks=new Socket(AddressFamily.InterNetwork,SocketType.Stream,ProtocolType.Tcp);
        socks.Connect(ip,port);
    }
    public static void SimpleEncode(Byte[] buf,int size)
    {
        for(int i=0;i<size;i++)
        {
            buf[i] +=3;
        }
    }
    public static void SimpleDecode(Byte[] buf,int size)
    {
        for(int i=0;i<size;i++)
        {
            buf[i] -=3;
        }
    }

    public void run()
    {
        SocksClient();
        ArrayList readList=new ArrayList();
        while(true)
        {
            readList.Add(socks);
            readList.Add(browser);
            try {
                Socket.Select(readList,null,null,60000000);
                if(readList.Contains(socks)) {
                    Byte[] buf=new Byte[8192];
                    int nr=socks.Receive(buf);
                    if(nr <= 0)
                    {
                        socks.Close();
                        browser.Close();
                        return;
                    }
                    SimpleDecode(buf,nr);
                    browser.Send(buf,nr,SocketFlags.None);
                }
                if(readList.Contains(browser)) {
                    Byte[] buf=new Byte[8192];
                    int nr=browser.Receive(buf);
                    if(nr <= 0)
                    {
                        socks.Close();
                        browser.Close();
                        return;
                    }
                    SimpleEncode(buf,nr);
                    socks.Send(buf,nr,SocketFlags.None);
                }
            }
            catch (Exception e) 
            {
                Console.WriteLine("Exception: " + e.Message);
                socks.Close();
                browser.Close();
                return;
            }
        }

    }

}
