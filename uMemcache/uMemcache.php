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

define("UDP_MAX_PAYLOAD_SIZE",1400);

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

/*
VALUE abc 0 9\r\n
abcabcabc\r\n
VALUE def 0 9\r\n
defdefdef\r\n
VALUE mg 0 6\r\n
mgmgmg\r\n
VALUE mk 0 6\r\n
mkmkmk\r\n
VALUE mh 0 6\r\n
mhmhmh\r\n
END\r\n
*/

function response_decode($reps,&$err)
{
    $repl=strlen($reps);
    if(substr($reps,0,5) != "VALUE" || substr($reps,-5) != "END\r\n")
    {
        $err="invalid response protocol.";
        return false;
    }
    $start=0;
    $res=array();
    while($start < strlen($reps))
    {
        if(substr($reps,$start,5) == "END\r\n")
        {
            break;
        }
        $end=strpos($reps,"\r\n",$start);
        if($end === FALSE)
        {
            $err="invalid response protocol 2.";
            return false;
        }
        list($non,$key,$flags,$bytes)=sscanf(substr($reps,$start,$end-$start),"%s %s %s %s");
        $start=$end+2;
        $val=substr($reps,$start,$bytes);
        if($flags & 16)
        {
            $val=gzuncompress($val);
        }
        $res[$key]=$val;
        $start+=$bytes;
        if(substr($reps,$start,2) != "\r\n")
        {
            $err="invalid response protocol 3.";
            return false;
        }
        $start+=2;
    }
    return $res;
}


/*
 * $host: string, Redis Server IP address or hostname. 
 * $port: int, Redis Server port.
 * $keys: array of strings, keys to get from Redis Server.
 * $err: string, error details.
 * $timeout: int, the timeout(us) of recvfrom call.
 * return: assoc array of key => value. return false on error, check $err for details.
 */
 
function uMemcache_mget($host,$port,$keys,&$err,$timeout=100000)
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
    $cmd="get ";
    $cmd.=implode(" ",$keys);
    $cmd.="\r\n";
    $cmdl=strlen($cmd);
    if($cmdl > UDP_MAX_PAYLOAD_SIZE - $headerl)
    {
        $err="request is too big.";
        return false;
    }
    $sfd=socket_create(AF_INET,SOCK_DGRAM,SOL_UDP);
    socket_set_option($sfd,SOL_SOCKET,SO_RCVTIMEO,array("sec" => 0,"usec" => $timeout));
    $ns=socket_sendto($sfd,$header.$cmd,$cmdl+$headerl,0,$host,$port);

    $rs=array();
    while(1)
    {
        $nr=socket_recvfrom($sfd,$rbuf,UDP_MAX_PAYLOAD_SIZE,0,$host,$port);
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
    if(substr($rstr,-5) != "END\r\n")
    {
        $err="incompletely response.";
        return false;
    }
    if(substr($rstr,0,7)  == "ERROR\r\n" || substr($rstr,0,12) == "CLIENT_ERROR" || substr($rstr,0,12) == "SERVER_ERROR")
    {
        $err=trim($rstr);
        return false;
    }
    else if(substr($rstr,0,5) == "VALUE")
    {
        return response_decode($rstr,$err);
    }
    else
    {
        $err="unknown response string.";
        return false;
    }
}
/*
$host="localhost";
$port=11211;
$r=new Memcached();
$r->addServer($host,$port);

$val="0123456789";
$val.=$val;//20
$val.=$val;//40
$val.=$val;//80
$val.=$val;//160
$val.=$val;//320
//$val.=$val;//640
//$val.=$val;//1280
//$val.=$val;//2560

$j=10000;
while($j > 0){
for($nkey=0; $nkey < 102;$nkey++)
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
    $ret=uMemcache_mget($host,$port,$keys,$err);
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
//$j--;
//break;
}
*/
?>
