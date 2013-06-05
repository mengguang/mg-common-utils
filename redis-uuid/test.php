<?php

for($i=0;$i<500000000;$i++) {
	$mc=new Memcached();
	$mc->addServer("localhost",3001);
	$uuid=$mc->get("uuid");
	if($uuid === false) {
		echo "false !!! $i\n";
		break;
	}
	unset($mc);
	if($i % 1000 == 0) echo "$i\n";
}
var_dump($uuid);
?>

