<?php

/*

UDP protocol
------------

For very large installations where the number of clients is high enough
that the number of TCP connections causes scaling difficulties, there is
also a UDP-based interface. The UDP interface does not provide guaranteed
delivery, so should only be used for operations that aren't required to
succeed; typically it is used for "get" requests where a missing or
incomplete response can simply be treated as a cache miss.

Each UDP datagram contains a simple frame header, followed by data in the
same format as the TCP protocol described above. In the current
implementation, requests must be contained in a single UDP datagram, but
responses may span several datagrams. (The only common requests that would
span multiple datagrams are huge multi-key "get" requests and "set"
requests, both of which are more suitable to TCP transport for reliability
reasons anyway.)

The frame header is 8 bytes long, as follows (all values are 16-bit integers
in network byte order, high byte first):

0-1 Request ID
2-3 Sequence number
4-5 Total number of datagrams in this message
6-7 Reserved for future use; must be 0

The request ID is supplied by the client. Typically it will be a
monotonically increasing value starting from a random seed, but the client
is free to use whatever request IDs it likes. The server's response will
contain the same ID as the incoming request. The client uses the request ID
to differentiate between responses to outstanding requests if there are
several pending from the same server; any datagrams with an unknown request
ID are probably delayed responses to an earlier request and should be
discarded.

The sequence number ranges from 0 to n-1, where n is the total number of
datagrams in the message. The client should concatenate the payloads of the
datagrams for a given response in sequence number order; the resulting byte
stream will contain a complete response in the same format as the TCP
protocol (including terminating \r\n sequences).

*/

function header_encode($req,$starteq,$ngram)
{
    if($req < 0 || $req > 65535 || $starteq < 0 || $starteq > 65535 || $ngram < 0 || $ngram > 65535)
    {
        return false;
    }
    $hdr = chr($req >> 8) . chr(($req << 8) >> 8);
    $hdr.= chr($starteq >> 8) . chr(($starteq << 8) >> 8);
    $hdr.= chr($ngram >> 8) . chr(($ngram << 8) >> 8);
    $hdr.="\x00\x00";
    return $hdr;
}

function header_decode($hdr,&$req,&$starteq,&$ngram)
{
    if(strlen($hdr) < 8)
    {
        return false;
    }
    $req=(ord($hdr[0]) << 8) + ord($hdr[1]);
    $starteq=(ord($hdr[2]) << 8) + ord($hdr[3]);
    $ngram=(ord($hdr[4]) << 8) + ord($hdr[5]);
    return true;
}

function response_decode($reps,&$err)
{
    $repl=strlen($reps);
    if($reps[0] != '*')
    {
        $err="invalid response protocol.";
        return false;
    }
    if(substr_compare($reps,"\r\n",-2,2) != 0)
    {
        $err="incompletely response.";
        return false;
    }
    $start=1;
    $end=strpos($reps,"\r\n",$start);
    $nval=intval(substr($reps,$start,$end-$start));
    if($nval < 1)
    {
        $err="invalid multibulk header.";
        return false;
    }
    $start=$end+2;
    if($start >= $repl)
    {
        $err="response too short 0.";
        return false;
    }
    $ret=array();
    for($i=0 ;$i < $nval; $i++)
    {
        $end=strpos($reps,"\r\n",$start);
        if($end === false)
        {
            $err="response too short 1.";
            return false;
        }                
        $vlen=intval(substr($reps,$start+1,$end-$start));
        if($vlen == -1)
        {
            $ret[$i]=NULL;
            $start=$end+2;
        }
        else
        {
            $start=$end+2;
            $ret[$i]=substr($reps,$start,$vlen);
            $start+=$vlen+2;
        }
    }
    return $ret;
}


/*
 * $host: string, Redis Server IP address or hostname. 
 * $port: int, Redis Server port.
 * $keys: array of strings, keys to get from Redis Server.
 * return: array of strings, values of keys, same chrer with keys.
 */
 
function uRedis_mget($host,$port,$keys,&$err)
{
    if(!is_array($keys) || count($keys) == 0 || count($keys) > 100)
    {
        $err="keys must be an non empty array and contains at most 100 keys.";
        return false;
    }
    if(strlen($host) < 1 || !is_numeric($port))
    {
        $err="host must be a string and port must be a number.";
        return false;
    }
    $req=rand(0,65535);
    $header=header_encode($req,0,1);

    $headerl=8;
    $nkey=count($keys) + 1;
    $cmd="*{$nkey}\r\n\$4\r\nmget\r\n";
    foreach($keys as $key)
    {
        $kl=strlen($key);
        $cmd.="\${$kl}\r\n{$key}\r\n";
    }
    $cmdl=strlen($cmd);
    if($cmdl > 1024 - $headerl)
    {
        $err="request is too big.";
        return false;
    }
    $sfd=socket_create(AF_INET,SOCK_DGRAM,SOL_UDP);
    socket_set_option($sfd,SOL_SOCKET,SO_RCVTIMEO,array('sec' => 0,'usec' => 100000));
    $ns=socket_sendto($sfd,$header.$cmd,$cmdl+$headerl,0,$host,$port);

    $rs=array();
    while(1)
    {
        $nr=socket_recvfrom($sfd,$rbuf,1024,0,$host,$port);
        if($nr === false)
        {
            $err="recvfrom failed.";
            return false;
        }
        if(header_decode($rbuf,$rreq,$starteq,$ngram) === false)
        {
            $err="bad UDP header.";
            return false;
        }
        if($req != $rreq)
        {
            continue;
        }
        else
        {
            $rs[$starteq]=substr($rbuf,8);
            if(count($rs) == $ngram)
            {
                break;
            }
        }
    }
    $rstr=implode("",$rs);
    if($rstr[0] == '-')
    {
        $err=substr($rstr,6);
        return false;
    }
    else if($rstr[0] == '*')
    {
        return response_decode($rstr,$err);
    }
    else
    {
        $err="unknown response string.";
        return false;
    }

}

$host="10.210.210.67";
$port=9002;
$r=new Redis();
$r->connect($host,$port);

$val="xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
$val.=$val;
$val.=$val;
$val.=$val;
$val.=$val;
while(1){
for($nkey=0; $nkey < 110;$nkey++)
{
    $keys=array();
    for($i=0;$i < $nkey;$i++)
    {
        $keys[]=$i."000.keys";
    }
    foreach($keys as $key)
    {
        //$r->set($key,$val);
    }
    $ret=uRedis_mget($host,$port,$keys,$err);
    if($ret == false)
    {
        echo $err;
        echo "\n";
    }
    else
    {
        //var_dump(array_combine($keys,$ret));
    }
}
}

?>
