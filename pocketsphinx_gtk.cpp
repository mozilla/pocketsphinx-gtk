//
// Created by Andre Natal on 5/23/16.
//

#include <pocketsphinx/pocketsphinx.h>
#include <sphinxbase/ad.h>
#include <sphinxbase/err.h>
#include <sys/select.h>
#include "pocketsphinx_gtk.h"
#include "main.h"
#include <glib/gprintf.h>
#include <glib/gmain.h>


#define MIC_BUF_SIZE 4096
#define KWS "foxy"
#define KWSTHRESHOLD 0.84

#ifdef RPI
#define MODELDIR  "/home/pi/projects/pocketsphinx-gtk/"
#else
#define MODELDIR  "/Users/anatal/ClionProjects/pocketsphinx_gtk/"
#endif

ps_decoder_t *ps;
cmd_ln_t *config;
bool decoder_paused = TRUE;
bool online_on = true;

/* Sleep for specified msec */
static void
sleep_msec(int32 ms)
{
#if (defined(_WIN32) && !defined(GNUWINCE)) || defined(_WIN32_WCE)
    Sleep(ms);
#else
    struct timeval tmo;
    tmo.tv_sec = 0;
    tmo.tv_usec = ms * 1000;
    select(0, NULL, NULL, NULL, &tmo);
#endif
}

void *
recognize_from_microphone(void *args)
{
    ad_rec_t *ad;
    int16 adbuf[MIC_BUF_SIZE];
    uint8 in_speech;
    int32 k;
    char const *hyp;
    FILE * pFile;
    int total_silence = 0;
    int skip_bytes = 0;
    char buf_kaldi[512];
    char tts_cmd[2048];
    float score;
    lbl_s lbl_w,lbl_o,lbl_k;

    int active_decoder = 0; // 0 = pocketsphinx ; 1 = kaldi
    if ((ad = ad_open_dev(cmd_ln_str_r(config, "-adcdev"),
                          (int) cmd_ln_float32_r(config,
                                                 "-samprate"))) == NULL)
        E_FATAL("Failed to open audio device\n");
    if (ad_start_rec(ad) < 0)
        E_FATAL("Failed to start recording\n");

    if (ps_start_utt(ps) < 0)
        E_FATAL("Failed to start utterance\n");

    E_INFO("READY....\n");

    for (;;) {

        if ((k = ad_read(ad, adbuf, MIC_BUF_SIZE)) < 0)
            E_FATAL("Failed to read audio\n");

        if (decoder_paused){
            continue;
        }

        if (active_decoder == 0){
            // process pocketsphinx
            ps_process_raw(ps, adbuf, k, FALSE, FALSE);

            hyp = ps_get_hyp(ps, NULL );

            if (hyp != NULL) {
                ps_end_utt(ps);
                E_INFO("FOUND!!  %s\n", hyp);
                // get kws score
                score = get_score();

                // update the main window labels
                lbl_k.type = 'k';
                strcpy(lbl_k.lblvalue, "");
                gdk_threads_add_idle((GSourceFunc)update_labels,&lbl_k);

                // update the main window labels
                lbl_o.type = 'o';
                strcpy(lbl_o.lblvalue, "");
                gdk_threads_add_idle((GSourceFunc)update_labels,&lbl_o);

                // set the struct to response
                sprintf(buf_kaldi,"(%f) %s - %s", score, KWS, score > KWSTHRESHOLD ? "(above or equal threshold)" : "(below threshold)");
                lbl_w.type = 'w';
                strcpy(lbl_w.lblvalue, buf_kaldi);
                gdk_threads_add_idle((GSourceFunc)update_labels,&lbl_w);

                if (score >= KWSTHRESHOLD){
                    gdk_threads_add_idle((GSourceFunc)change_btncolor,(gpointer)"green");
                    system("aplay " MODELDIR "/spot.wav");

                    if (online_on){
                        E_INFO("KWS spot. Go to decoding!  %s\n", hyp);
                        active_decoder = 1;
                        skip_bytes = 1;

                        // change the search to offline kws
                        ps_set_search(ps,"offline_kws");

                        // open the file that will be used by kaldi
                        pFile = fopen (MODELDIR "/audio.raw","w");
                    }
                } else {
                    system("aplay " MODELDIR "/basso.wav");
                }

                // start the utterance
                if (ps_start_utt(ps) < 0)
                    E_FATAL("Failed to start utterance\n");
            }
        } else {
            //E_INFO("PROCESSING TO ONLINE!\n");

            // skip some bytes to remove the beep
            if (skip_bytes > 0 && skip_bytes < 2){
                skip_bytes++;
                E_INFO("Skipping bytes..\n");
                continue;
            } else if (skip_bytes == 2) {
                skip_bytes = 0;
            }

            if (pFile!=NULL)
            {
                // write the bytes on the disk for kaldi
                fwrite(adbuf,sizeof(int16),k,pFile);

                // process on pocketsphinx
                ps_process_raw(ps, adbuf, k, FALSE, FALSE);

                // search for silence
                in_speech = ps_get_in_speech(ps);
                if (in_speech){
                    //E_INFO("Has speech...\n");
                    //total_silence = 0;
                } else {
                    //E_INFO("Don' Have speech...\n");
                    total_silence += 50;
                }
            }

            //Ok, we have enough silence;
            if (total_silence >= 1500){
                E_INFO("ENOUGH SILENCE.\n");

                // reset the amount of silence
                total_silence = 0;

                E_INFO("DECODING ON PS\n");

                // end the utterance
                ps_end_utt(ps);
                // search for the hypothesis
                hyp = ps_get_hyp(ps, NULL );

                if (hyp != NULL) {
                    // ok. we found offline command.
                    E_INFO("FOUND HYP OFFLINE!!  %s\n", hyp);

                    // we get the score to check if is reliable
                    score = get_score();

                    // TODO : check if is reliable

                    // - we send the command to OH
                    char cmd[1024];
                    sprintf(cmd,"curl --header 'Content-Type: text/plain' --request POST --data '%s' http://192.168.1.200:8080/rest/items/wemo_socket_Socket_1_0_221517K11005FE_state",strstr (hyp,"off") ? "OFF" : "ON");
                    system(cmd);

                    // update the main window labels
                    sprintf(buf_kaldi,"(%f) %s - %s", score, hyp, score > KWSTHRESHOLD ? "(above or equal threshold)" : "(below threshold)");
                    lbl_o.type = 'o';
                    strcpy(lbl_o.lblvalue, buf_kaldi);
                    gdk_threads_add_idle((GSourceFunc)update_labels,&lbl_o);
                } else {
                    lbl_o.type = 'o';
                    strcpy(lbl_o.lblvalue, "no hypo found offline - going online");
                    gdk_threads_add_idle((GSourceFunc)update_labels,&lbl_o);

                    // if not found offline, we go to kaldi
                    E_INFO("DECODING ON KALDI..\n");
                    system("aplay " MODELDIR "/end_spot.wav");
                    fclose (pFile);
                    FILE *php = popen("php " MODELDIR "/kaldi.php " MODELDIR "/audio.raw", "r");
                    fgets(buf_kaldi, sizeof(buf_kaldi), php);
                    pclose(php);
                    E_INFO("Result: %s\n", buf_kaldi);

                    // - we send the command to OH
                    char cmd[1024];
                    sprintf(cmd,"curl --header 'Content-Type: text/plain' --request POST --data '%s' http://192.168.1.200:8080/rest/items/wemo_socket_Socket_1_0_221517K11005FE_state",strstr (buf_kaldi,"OFF") ? "OFF" : "ON");
                    system(cmd);

                    // update the main window labels
                    lbl_k.type = 'k';
                    strcpy(lbl_k.lblvalue, buf_kaldi);
                    gdk_threads_add_idle((GSourceFunc)update_labels,&lbl_k);

                }

                // aplay the TTS with the result
                sprintf(tts_cmd,"php " MODELDIR "/tts.php \"%s\"", buf_kaldi);
                system(tts_cmd);
                system("aplay tts.wav");

                // update the main window color
                gdk_threads_add_idle((GSourceFunc)change_btncolor,(gpointer)"yellow");

                // then we send the decoder back to kws
                active_decoder = 0;

                // change the search to offline kws
                ps_set_search(ps,"kws");

                // start the utterance
                if (ps_start_utt(ps) < 0)
                    E_FATAL("Failed to start utterance\n");

            }
        }
        sleep_msec(50);
    }
    ad_close(ad);
    return NULL;
}

float
get_score(){
    ps_seg_t *iter = ps_seg_iter(ps);
    float conf;

    while (iter != NULL)
    {
        int32 sf, ef, pprob;

        ps_seg_frames (iter, &sf, &ef);
        pprob = ps_seg_prob (iter, NULL, NULL, NULL);
        conf = logmath_exp(ps_get_logmath(ps), pprob);
        printf ("%s - %f\n", ps_seg_word (iter), conf);
        iter = ps_seg_next (iter);
    }

    return conf;
}


bool
change_decoder_state(){
    return (decoder_paused = !decoder_paused);
}

void
pocketsphinxstart(){
    config = cmd_ln_init(NULL, ps_args(), TRUE,
                         "-hmm", MODELDIR "models/std-en-us/",
                         "-dict", MODELDIR "models/cmudict-en-us.dict",
                         NULL);
    if (config == NULL) {
        fprintf(stderr, "Failed to create config object, see log for details\n");
    }
    ps = ps_init(config);
    if (ps == NULL) {
        cmd_ln_free_r(config);
    }

    // here we create the searches. one for kws one word, and another for the offline commands
    ps_set_kws(ps,"kws",MODELDIR "models/kws.txt");
    ps_set_kws(ps,"offline_kws",MODELDIR "models/offline_kws.txt");
    // we'll start with the kws
    ps_set_search(ps,"kws");

    E_INFO("COMPILED ON: %s, AT: %s\n\n", __DATE__, __TIME__);
}

void
destroy_ps(){
    ps_free(ps);
    cmd_ln_free_r(config);
}