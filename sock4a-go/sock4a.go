package main

import "fmt"
import "net"

func SimpleEncode(buf []byte) {
    for i:=0;i<len(buf);i++ {
        buf[i]+=3
    }
}
func SimpleDecode(buf []byte) {
    for i:=0;i<len(buf);i++ {
        buf[i]-=3
    }
}

func handleSocks(browser net.Conn,socks net.Conn) {
    buf := make([]byte,1024)
    for {
        n,err := socks.Read(buf)
        if err != nil {
            browser.Close()
            socks.Close()
            return
        }
        SimpleDecode(buf)
        n,err = browser.Write(buf[0:n])
        if err != nil {
            browser.Close()
            socks.Close()
            return
        }
    }
}

func handleBrowser(browser net.Conn,socks net.Conn) {
    buf := make([]byte,1024)
    for {
        n,err := browser.Read(buf)
        if err != nil {
            browser.Close()
            socks.Close()
            return
        }
        SimpleEncode(buf)
        n,err = socks.Write(buf[0:n])
        if err != nil {
            browser.Close()
            socks.Close()
            return
        }
    }
}

func handleConnection(browser net.Conn) {
    socks,err := net.Dial("tcp","106.187.41.101:21080")
    if err != nil {
        fmt.Println("error on Dail.")
        browser.Close()
        return
    }
    go handleBrowser(browser,socks)
    handleSocks(browser,socks)
}

func main() {
    ln,err := net.Listen("tcp","10.1.9.140:21080")
    if err != nil {
        fmt.Println("error on listen.")
        return
    }
    for {
        conn,err := ln.Accept()
        if err != nil {
            fmt.Println("error on accept.")
            continue
        }
        go handleConnection(conn)
    }
}
