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

// call flite
//$comando = '/Users/anatal/projects/mozilla/vaani-iot/mimic/bin/mimic "' . $synthval . '" -voice /Users/anatal/projects/mozilla/vaani-iot/mimic/voices/cmu_us_slt.flitevox';
//exec($comando);

// call marytts
// ESCAPE THE single quotes

$url = "http://mary.dfki.de:59125/process?INPUT_TYPE=TEXT&OUTPUT_TYPE=AUDIO&INPUT_TEXT=" .$synthval. "&OUTPUT_TEXT=&effect_Volume_selected=&effect_Volume_parameters=amount%3A2.0%3B&effect_Volume_default=Default&effect_Volume_help=Help&effect_TractScaler_selected=&effect_TractScaler_parameters=amount%3A1.5%3B&effect_TractScaler_default=Default&effect_TractScaler_help=Help&effect_F0Scale_selected=&effect_F0Scale_parameters=f0Scale%3A2.0%3B&effect_F0Scale_default=Default&effect_F0Scale_help=Help&effect_F0Add_selected=&effect_F0Add_parameters=f0Add%3A50.0%3B&effect_F0Add_default=Default&effect_F0Add_help=Help&effect_Rate_selected=&effect_Rate_parameters=durScale%3A1.5%3B&effect_Rate_default=Default&effect_Rate_help=Help&effect_Robot_selected=&effect_Robot_parameters=amount%3A100.0%3B&effect_Robot_default=Default&effect_Robot_help=Help&effect_Whisper_selected=&effect_Whisper_parameters=amount%3A100.0%3B&effect_Whisper_default=Default&effect_Whisper_help=Help&effect_Stadium_selected=&effect_Stadium_parameters=amount%3A100.0&effect_Stadium_default=Default&effect_Stadium_help=Help&effect_Chorus_selected=&effect_Chorus_parameters=delay1%3A466%3Bamp1%3A0.54%3Bdelay2%3A600%3Bamp2%3A-0.10%3Bdelay3%3A250%3Bamp3%3A0.30&effect_Chorus_default=Default&effect_Chorus_help=Help&effect_FIRFilter_selected=&effect_FIRFilter_parameters=type%3A3%3Bfc1%3A500.0%3Bfc2%3A2000.0&effect_FIRFilter_default=Default&effect_FIRFilter_help=Help&effect_JetPilot_selected=&effect_JetPilot_parameters=&effect_JetPilot_default=Default&effect_JetPilot_help=Help&HELP_TEXT=&exampleTexts=&VOICE_SELECTIONS=cmu-slt%20en_US%20female%20unitselection%20general&AUDIO_OUT=WAVE_FILE&LOCALE=en_US&VOICE=cmu-slt&AUDIO=WAVE_FILE";
echo $url;
exec("wget '" . $url . "' -O tts.wav");
exec("play tts.wav");

// close the session


?>