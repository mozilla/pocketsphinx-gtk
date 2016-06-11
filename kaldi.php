<?php
// initialize the curl request
$request = curl_init('http://54.171.129.75/asr/?endofspeech=false');

// send a file
curl_setopt($request, CURLOPT_POST, true);
curl_setopt(
    $request,
    CURLOPT_POSTFIELDS,
    array( 
       'endofspeech=false' .
       'file' => '@' . $argv[1]
      . ';type=Content-Type: application/octet-stream'
    ));

// make the call
curl_setopt($request, CURLOPT_RETURNTRANSFER, true);
$result = curl_exec($request);
$obj = json_decode($result);

// return the result or the error to the shell
if ($obj->{'status'} == 'ok'){
	echo "(" . $obj->{'data'}[0]->{'confidence'} . ") " . $obj->{'data'}[0]->{'text'};
} else {
    echo "ERROR:" . $result;
}

// close the session
curl_close($request);
?>
