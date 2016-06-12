<?php

$question =  urlencode(trim(substr($argv[1],strpos($argv[1],")"))));

// initialize the curl request
$request = curl_init("https://api.api.ai/v1/query?v=20150910&query=" . $question . "&lang=en&sessionId=63c44ce2-80d2-4c83-8be9-ec6dabc3c428&timezone=2016-06-12T03:27:40+0200");
curl_setopt($request, CURLOPT_HTTPHEADER, array(
    'Authorization:Bearer 787042d394aa46c0ac0f958216557cb6'
    ));

// make the call
curl_setopt($request, CURLOPT_RETURNTRANSFER, true);
$result = curl_exec($request);
$obj = json_decode($result);
//return the result or the error to the shell
if (($obj->{'status'}->{'code'} == 200) && ($obj->{'result'}->{'fulfillment'}->{'speech'} != "")){
	$synthval = $obj->{'result'}->{'fulfillment'}->{'speech'};
} else {
  $synthval = "Sorry, I don't know anything about that.";
}

curl_close($request);

echo $synthval;

$file = '/Users/anatal/ClionProjects/pocketsphinx_gtk/tts.txt';
file_put_contents($file, $synthval, LOCK_EX);

// call flite
$comando = '/Users/anatal/projects/mozilla/vaani-iot/mimic/bin/mimic -f /Users/anatal/ClionProjects/pocketsphinx_gtk/tts.txt -voice /Users/anatal/projects/mozilla/vaani-iot/mimic/voices/cmu_us_slt.flitevox tts.wav';
exec($comando);

?>