//
// Created by Andre Natal on 5/23/16.
//

#include <pocketsphinx/pocketsphinx.h>
#include <sphinxbase/ad.h>
#include <sphinxbase/err.h>
#include <sys/select.h>
#include "pocketsphinx_gtk.h"
#include "main.h"

#define MODELDIR  "/Users/anatal/ClionProjects/pocketsphinx_gtk/models"

ps_decoder_t *ps;
cmd_ln_t *config;
bool decoder_paused = TRUE;

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
    int16 adbuf[2048];
    uint8 in_speech;
    int32 k;
    char const *hyp;

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
        if (decoder_paused){
            continue;
        }
        if ((k = ad_read(ad, adbuf, 2048)) < 0)
            E_FATAL("Failed to read audio\n");

        ps_process_raw(ps, adbuf, k, FALSE, FALSE);
        in_speech = ps_get_in_speech(ps);
/*
        if (in_speech){
            E_INFO("Has speech...\n");
        } else {
            E_INFO("Don' Have speech...\n");
        }
*/
        hyp = ps_get_hyp(ps, NULL );
        if (hyp != NULL) {
            E_INFO("FOUND!! Go to Kaldi!  %s\n", hyp);
            ps_end_utt(ps);
            change_btncolor("green");
            system("play /Users/anatal/ClionProjects/pocketsphinx_gtk/spot.wav");
            change_btncolor("yellow");

            if (ps_start_utt(ps) < 0)
                E_FATAL("Failed to start utterance\n");
        }

        sleep_msec(50);
    }
    ad_close(ad);

    return NULL;
}

bool
change_decoder_state(){
    decoder_paused = !decoder_paused;
    return decoder_paused;
}

int pocketsphinxstart(){
    config = cmd_ln_init(NULL, ps_args(), TRUE,
                         "-hmm", MODELDIR "/std-en-us/",
                         "-keyphrase", "alexa",
                         "-dict", MODELDIR "/cmudict-en-us.dict",
                         "-kws_threshold", "1e-20",
                         NULL);
    if (config == NULL) {
        fprintf(stderr, "Failed to create config object, see log for details\n");
        return -1;
    }
    ps = ps_init(config);
    if (ps == NULL) {
        cmd_ln_free_r(config);
        return 1;
    }

    E_INFO("COMPILED ON: %s, AT: %s\n\n", __DATE__, __TIME__);
}

void destroy_ps(){
    ps_free(ps);
    cmd_ln_free_r(config);
}