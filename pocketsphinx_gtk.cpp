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
#define KWSTHRESHOLD 0.97

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
    lbl_s lbl_t;

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
                float score = get_score();

                // set the struct to response

                sprintf(buf_kaldi,"(%f) %s - %s", score, KWS, score > KWSTHRESHOLD ? "(above or equal threshold)" : "(below threshold)");
                lbl_t.type = 'w';
                strcpy(lbl_t.lblvalue, buf_kaldi);
                gdk_threads_add_idle((GSourceFunc)update_labels,&lbl_t);

                if (score >= KWSTHRESHOLD){
                    gdk_threads_add_idle((GSourceFunc)change_btncolor,(gpointer)"green");
                    system("play " MODELDIR "/spot.wav");

                    if (online_on){
                        E_INFO("Go to Kaldi!  %s\n", hyp);
                        active_decoder = 1;
                        total_silence  = 0;
                        skip_bytes = 1;
                        // open the file that will be used by kaldi
                        pFile = fopen (MODELDIR "/audio.raw","w");
                    }
                } else {
                    system("play " MODELDIR "/basso.wav");
                }
                if (ps_start_utt(ps) < 0)
                    E_FATAL("Failed to start utterance\n");
            }
        } else {
            //E_INFO("PROCESSING TO ONLINE!\n");
            if (skip_bytes > 0 && skip_bytes < 4){
                skip_bytes++;
                E_INFO("Skipping bytes..\n");
                continue;
            } else if (skip_bytes == 4) {
                skip_bytes = 0;
            }

            if (pFile!=NULL)
            {

                fwrite(adbuf,sizeof(int16),k,pFile);
                ps_process_raw(ps, adbuf, k, FALSE, FALSE);
                in_speech = ps_get_in_speech(ps);
                if (in_speech){
                    //E_INFO("Has speech...\n");
                    total_silence = 0;
                } else {
                    //E_INFO("Don' Have speech...\n");
                    total_silence += 50;
                }
            }

            //E_INFO("Total Silence  %i\n", total_silence);
            if (total_silence >= 1000){
                E_INFO("ENOUGH SILENCE. DECODING ON KALDI..\n");
                active_decoder = 0;
                fclose (pFile);
                FILE *php = popen("php " MODELDIR "/kaldi.php " MODELDIR "/audio.raw", "r");
                fgets(buf_kaldi, sizeof(buf_kaldi), php);
                pclose(php);
                E_INFO("Result: %s\n", buf_kaldi);
                lbl_t.type = 'k';
                strcpy(lbl_t.lblvalue, buf_kaldi);
                gdk_threads_add_idle((GSourceFunc)update_labels,&lbl_t);
                gdk_threads_add_idle((GSourceFunc)change_btncolor,(gpointer)"yellow");
                system("play " MODELDIR "/end_spot.wav");
            }
        }
        sleep_msec(50);
    }
    ad_close(ad);
    return NULL;
}

float get_score(){
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

void pocketsphinxstart(){
    config = cmd_ln_init(NULL, ps_args(), TRUE,
                         "-hmm", MODELDIR "models/std-en-us/",
                         "-keyphrase", KWS,
                         "-dict", MODELDIR "models/cmudict-en-us.dict",
                         "-kws_threshold", "1e-20",
                         NULL);
    if (config == NULL) {
        fprintf(stderr, "Failed to create config object, see log for details\n");
    }
    ps = ps_init(config);
    if (ps == NULL) {
        cmd_ln_free_r(config);
    }

    E_INFO("COMPILED ON: %s, AT: %s\n\n", __DATE__, __TIME__);
}

void destroy_ps(){
    ps_free(ps);
    cmd_ln_free_r(config);
}